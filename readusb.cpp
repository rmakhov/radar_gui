#include "readusb.h"
#include <qsharedmemory.h>
#include <QQueue>

QQueue<target_coord> g_queue;
QReadWriteLock g_lock;

ReadUSB::ReadUSB(QObject *parent)
: QThread(parent)
{
}

ReadUSB::~ReadUSB()
{
}

void ReadUSB::run()
{
    target_coord debugData;
    int i = 0;
    while(true) {
        // TODO: Add read and parse of USB data here

        // Generation of fake data
        debugData.azimuth = (10*i)%90;
        debugData.distance = (10*i)%100;
        debugData.speed = (10*i)%100;
        i++;

        // Place coord data in queue
        g_lock.lockForWrite();
        g_queue.enqueue(debugData);
        g_lock.unlock();

        QThread::msleep ( 1000 );
    }
}
