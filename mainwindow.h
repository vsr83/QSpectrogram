#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "spectrogram.h"
#include "qspectrogram.h"
#include "pulsethread.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

    PulseThread *pulseThread;
protected:
    void keyPressEvent(QKeyEvent *event);
private:
    Spectrogram *spectrogram;
    QSpectrogram *spectrogramWidget;
};

#endif // MAINWINDOW_H
