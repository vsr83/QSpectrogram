#include "mainwindow.h"
#include "spectrogram.h"
#include "qspectrogram.h"
#include "pulsethread.h"
#include <QDebug>
#include <QKeyEvent>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent) {
    spectrogram = new Spectrogram(44100, 44100 * 60, 256, 8192);

    spectrogramWidget = new QSpectrogram(spectrogram, this);
    setCentralWidget(spectrogramWidget);

    resize(1024, 600);
    QString device("alsa_output.pci-0000_00_1f.3.analog-stereo.monitor");
    //device = "alsa_input.pci-0000_00_1f.3.analog-stereo";

    pulseThread = new PulseThread(device, 44100, 1024);
    pulseThread->start();

    connect(pulseThread, SIGNAL(bufferFilled(float*,uint)),
            spectrogramWidget, SLOT(processData(float*,uint)));
}

void
MainWindow::keyPressEvent(QKeyEvent *event) {
    qDebug("keyPress %d", event->key());

    switch (event->key()) {
    case Qt::Key_F1:
        spectrogramWidget->setMinAmpl(0.0001);
        break;
    case Qt::Key_F2:
        spectrogramWidget->setMinAmpl(0.001);
        break;
    case Qt::Key_F3:
        spectrogramWidget->setMinAmpl(0.01);
        break;
    case Qt::Key_F4:
        spectrogramWidget->setMaxAmpl(0.1);
        break;
    case Qt::Key_F5:
        spectrogramWidget->setMaxAmpl(1.0);
        break;
    case Qt::Key_F6:
        spectrogramWidget->setMaxAmpl(10.0);
        break;
    }
}

MainWindow::~MainWindow() {
    delete spectrogram;
    spectrogram = 0;
}
