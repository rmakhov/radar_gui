// The class to read the data from radar through USB

#include <QThread>
#include <QReadWriteLock>

#ifndef READUSB_H
#define READUSB_H

// Detected targets characteristics 
struct target_coord {
    int distance;
    int speed;
    int azimuth;
};

class ReadUSB : public QThread
{
    Q_OBJECT

public:
    ReadUSB(QObject *parent = 0);
    ~ReadUSB();

protected:
    void run();

};

#endif
