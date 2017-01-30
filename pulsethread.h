#ifndef PULSETHREAD_H
#define PULSETHREAD_H

#include <QThread>

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <pulse/simple.h>
#include <pulse/error.h>

class PulseThread : public QThread {
    Q_OBJECT
public:
    PulseThread(const QString &_pulseDevice,
                unsigned int _sampleRate,
                unsigned int _bufferSize);
    ~PulseThread();
    void stop();

    QString pulseDevice;

    unsigned int sampleRate;
    unsigned int bufferSize;
signals:
    void bufferFilled(float *outputBufferLeft,
                      //float *outputBufferRight,
                      unsigned int bufferLength);
protected:
    void run();
private:
    volatile bool stopped;
    float *bufferLeft, *bufferRight;
    float *copyBufferLeft, *copyBufferRight;

    unsigned int bufferIndex;

    pa_sample_spec paSampleSpec;
    pa_simple *paSimple;
};

#endif // PULSETHREAD_H
