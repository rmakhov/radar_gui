/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Charts module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

// Roman Makhov - added initDisplay() and update() methods

#include "chartview.h"
#include <QtGui/QMouseEvent>
#include <QtCore/QDebug>
#include <QtCharts/QAbstractAxis>
#include <QtCharts/QValueAxis>
#include <QQueue>

// Include for ReadUSB class
#include "readusb.h"

QT_CHARTS_USE_NAMESPACE

// Target coordinators queue
extern QQueue<target_coord> g_queue;
extern QReadWriteLock g_lock;

ChartView::ChartView(QWidget *parent)
    : QChartView(parent)
{
    angularAxis = new QValueAxis();
    radialAxis = new QValueAxis();
}

void ChartView::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Plus:
        chart()->zoomIn();
        break;
    case Qt::Key_Minus:
        chart()->zoomOut();
        break;
    case Qt::Key_Left:
        chart()->scroll(-1.0, 0);
        break;
    case Qt::Key_Right:
        chart()->scroll(1.0, 0);
        break;
    case Qt::Key_Up:
        chart()->scroll(0, 1.0);
        break;
    case Qt::Key_Down:
        chart()->scroll(0, -1.0);
        break;
    case Qt::Key_Space:
        switchChartType();
        break;
    default:
        QGraphicsView::keyPressEvent(event);
        break;
    }
}

void ChartView::switchChartType()
{
    QChart *newChart;
    QChart *oldChart = chart();

    if (oldChart->chartType() == QChart::ChartTypeCartesian)
        newChart = new QPolarChart();
    else
        newChart = new QChart();

    // Move series and axes from old chart to new one
    QList<QAbstractSeries *> seriesList = oldChart->series();
    QList<QAbstractAxis *> axisList = oldChart->axes();
    QList<QPair<qreal, qreal> > axisRanges;

    foreach (QAbstractAxis *axis, axisList) {
        QValueAxis *valueAxis = static_cast<QValueAxis *>(axis);
        axisRanges.append(QPair<qreal, qreal>(valueAxis->min(), valueAxis->max()));
    }

    foreach (QAbstractSeries *series, seriesList)
        oldChart->removeSeries(series);

    foreach (QAbstractAxis *axis, axisList) {
        oldChart->removeAxis(axis);
        newChart->addAxis(axis, axis->alignment());
    }

    foreach (QAbstractSeries *series, seriesList) {
        newChart->addSeries(series);
        foreach (QAbstractAxis *axis, axisList)
            series->attachAxis(axis);
    }

    int count = 0;
    foreach (QAbstractAxis *axis, axisList) {
        axis->setRange(axisRanges[count].first, axisRanges[count].second);
        count++;
    }

    newChart->setTitle(oldChart->title());
    setChart(newChart);
    delete oldChart;
}

// Radar display initialization
void ChartView::initDisplay(QPolarChart *chart)
{
    chart->setTitle("Use arrow keys to scroll, +/- to zoom, and space to switch chart type.");

    angularAxis->setTickCount(9); // First and last ticks are co-located on 0/360 angle.
    angularAxis->setLabelFormat("%.1f");
    angularAxis->setShadesVisible(true);
    angularAxis->setShadesBrush(QBrush(QColor(249, 249, 255)));
    chart->addAxis(angularAxis, QPolarChart::PolarOrientationAngular);

    radialAxis->setTickCount(9);
    radialAxis->setLabelFormat("%d");
    chart->addAxis(radialAxis, QPolarChart::PolarOrientationRadial);

    radialAxis->setRange(radialMin, radialMax);
    angularAxis->setRange(angularMin, angularMax);

    // Initialization the series with maximum number of the targets on the point [0,0]
    for (int i = 0; i < targetMax; i++) {
      QString name = "target" + QString::number(i+1);
      series[i].setName(name);
      series[i].setPointLabelsVisible(true);
      series[i].setPointLabelsFormat("0");

      series[i].append(0,0);
      chart->addSeries(&series[i]);

      series[i].attachAxis(radialAxis);
      series[i].attachAxis(angularAxis);
    }
}


void ChartView::update(QPolarChart *chart)
{
    target_coord coord;
    QString label;

    while(true) {
        g_lock.lockForWrite();
        if (g_queue.empty()) {
            g_lock.unlock();
            break;
        }

        coord=g_queue.dequeue();
        g_lock.unlock();

        if (target_counter == TARGETS_NUM_MAX)
            target_counter = 0;

        series[target_counter].remove(0);
        qApp->processEvents();
        series[target_counter].append(coord.distance, coord.azimuth);
        qApp->processEvents();

        label = QString::number(coord.speed);
        series[target_counter].setPointLabelsFormat(label);

        target_counter++;
        qApp->processEvents();

        qDebug("target: %d, distance: %d azimuth: %d speed %d", target_counter, coord.distance, coord.azimuth, coord.speed);
  }
}


