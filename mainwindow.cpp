#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtCharts/QScatterSeries>
#include <QtCharts/QLineSeries>
#include <QtCharts/QSplineSeries>
#include <QtCharts/QAreaSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QPolarChart>
#include <QtCore/QDebug>
#include <QQueue>
#include <QTimer>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "chartview.h"
#include "readusb.h"

extern QQueue<target_coord> g_queue;
extern QReadWriteLock g_lock;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    chart = new QPolarChart();
    ui->customPlot->initDisplay(chart);
    ui->customPlot->setChart(chart);
    ui->customPlot->setRenderHint(QPainter::Antialiasing);

    stopTimerFlag = false;;
    tmr = new QTimer(this);
    tmr->setInterval(CHART_REFRESH_PERIOD_MS);
    connect(tmr, SIGNAL(timeout()), this, SLOT(updateChart()));
    tmr->start();
}

MainWindow::~MainWindow()
{
    delete ui;
    delete tmr;
}

void MainWindow::closeEvent (QCloseEvent *event)
{
    tmr->stop();
}

void MainWindow::updateChart()
{
    ui->customPlot->update(chart);
}



