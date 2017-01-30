#include "mainwindow.h"
#include "spectrogram.h"
#include "qspectrogram.h"
#include "pulsethread.h"
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent) {
    spectrogram = new Spectrogram(44100, 44100 * 60, 256, 1024);

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

MainWindow::~MainWindow() {
    delete spectrogram;
    spectrogram = 0;
}
