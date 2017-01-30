#include "pulsethread.h"
#include <QCoreApplication>
#include <QDebug>

#define RAW_BUFFERSIZE 8192

PulseThread::PulseThread(const QString &_pulseDevice,
                         unsigned int _sampleRate,
                         unsigned int _bufferSize) {
    pulseDevice = _pulseDevice;
    sampleRate = _sampleRate;
    bufferSize = _bufferSize;

    bufferLeft = new float[bufferSize];
    bufferRight = new float[bufferSize];
    copyBufferLeft = new float[bufferSize];
    copyBufferRight = new float[bufferSize];

    stopped = true;
    paSampleSpec.format = PA_SAMPLE_FLOAT32;
    paSampleSpec.rate = sampleRate;
    paSampleSpec.channels = 2;

    bufferIndex = 0;
}

PulseThread::~PulseThread() {
    delete [] bufferLeft;
    delete [] bufferRight;
    delete [] copyBufferLeft;
    delete [] copyBufferRight;
    bufferLeft = 0;
    bufferRight = 0;
    copyBufferLeft = 0;
    copyBufferRight = 0;
}

void
PulseThread::run() {
    stopped = false;
    int error;
    float rawBuffer[RAW_BUFFERSIZE];

    paSimple = pa_simple_new(NULL,
                             "alsaspecview",
                             PA_STREAM_RECORD,
                             qPrintable(pulseDevice),
                             "record",
                             &paSampleSpec,
                             NULL,
                             NULL,
                             &error);
    if (!paSimple) {
        qErrnoWarning(error, pa_strerror(error));
        QCoreApplication::quit();
    }

    for (;;) {
        int numRead, error;

        numRead = pa_simple_read(paSimple, rawBuffer, RAW_BUFFERSIZE, &error);
        if (numRead < 0) {
            qErrnoWarning(error, pa_strerror(error));
            QCoreApplication::quit();
        }
        if (numRead % 2 != 0) {
            qWarning("Non-even number of samples read!");
        } else {
            for (int indRead = 0; indRead < RAW_BUFFERSIZE / 8; indRead++) {
                bufferLeft[bufferIndex] = rawBuffer[indRead*2];
                bufferRight[bufferIndex] = rawBuffer[indRead*2 + 1];

                bufferIndex++;

                if (bufferIndex == bufferSize) {
                    //qDebug() << "read";

                    for (int ind = 0; ind < bufferSize; ind++) {
                        copyBufferLeft[ind] = bufferLeft[ind];
                        copyBufferRight[ind] = bufferRight[ind];
                    }

                    emit bufferFilled(copyBufferLeft,  bufferSize);
                    bufferIndex = 0;/*
                    for (int ind = bufferSize-513; ind >= 0; ind--) {
                        bufferLeft[ind] = copyBufferLeft[ind + 512];
                        bufferRight[ind] = copyBufferRight[ind + 512];
                    }
                    bufferIndex = bufferSize - 512;*/
                }
            }
        }

    }
}

void
PulseThread::stop() {
    if (paSimple) {
        pa_simple_free(paSimple);
    }
    stopped = true;
}
