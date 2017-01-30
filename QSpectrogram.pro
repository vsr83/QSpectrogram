#-------------------------------------------------
#
# Project created by QtCreator 2016-12-31T02:15:11
#
#-------------------------------------------------

QT       += core gui
LIBS += -lpulse -lpulse-simple -O3

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = QSpectrogram
TEMPLATE = app
QMAKE_CXXFLAGS += -O3

SOURCES += main.cpp\
        mainwindow.cpp \
    spectrogram.cpp \
    qspectrogram.cpp \
    pulsethread.cpp

HEADERS  += mainwindow.h \
    spectrogram.h \
    qspectrogram.h \
    pulsethread.h
