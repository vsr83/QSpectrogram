#ifndef SPECTROGRAM_H
#define SPECTROGRAM_H

#include <list>
#include <vector>
#include <complex>

class Spectrogram {
public:
    Spectrogram(unsigned int _sampleRate,
               unsigned int _sampleLength,
               unsigned int _samplesPerLine,
               unsigned int _numLines);
    ~Spectrogram();

    unsigned int processData(float *buffer,
                             unsigned int bufferLength);

    double getDeltaTime();
    double getFootTime();
    double getHeadTime();

    std::list<std::vector<float> > spectrogramData;
    std::list<float> waveEnvelopeMin;
    std::list<float> waveEnvelopeMax;
    std::list<float> timeList;
    std::vector<float> frequencyList;
private:
    void removeFoot(unsigned int numLines);
    void addLine(float *fourierData,
                 unsigned int dataLength,
                 float envmin,
                 float envmax);
    void FFTCompute(std::complex<float> *data,
                    unsigned int dataLength);

    unsigned int sampleRate;
    unsigned int sampleLength;
    unsigned int samplesPerLine;
    unsigned int fftSize;
    unsigned int numLines;

    float *waveRingBuffer;
    unsigned int ringBufferSize;
    unsigned int ringBufferInd;
    unsigned int sampleCounter;

    double headTime;
    double footTime;
    double deltaTime;

};

#endif // SPECTROGRAM_H
