#include "spectrogram.h"
#include <assert.h>
#include <iostream>
#include "fftcuda.h"

Spectrogram::Spectrogram(unsigned int _sampleRate,
			 unsigned int _sampleLength,
			 unsigned int _samplesPerLine,
			 unsigned int _numLines) {
    sampleRate     = _sampleRate;
    sampleLength   = _sampleLength;
    samplesPerLine = _samplesPerLine;
    numLines       = _numLines;
    ringBufferSize = (numLines - 1) * samplesPerLine + sampleLength;
    sampleCounter  = 0;

    waveRingBuffer = new float[ringBufferSize];
    std::fill_n(waveRingBuffer, ringBufferSize, 0.0f);

    headTime  = 0.0f;
    deltaTime = 0.0f;
    deltaTime = ((double)samplesPerLine)/((double)sampleRate);
    fftSize = 4096;

    for (unsigned int indFreq = 0; indFreq < fftSize; indFreq++) {
        float freq = ((float)(indFreq)) * ((float)sampleRate) /((float)fftSize);
        std::cout << freq << std::endl;
        frequencyList.push_back(freq);
    }
}

Spectrogram::~Spectrogram() {
    delete [] waveRingBuffer;
    waveRingBuffer = 0;
}

void
Spectrogram::setSampleParameters(unsigned int _sampleRate,
				 unsigned int _sampleLength,
				 unsigned int _samplesPerLine,
				 bool recompute) {

}

unsigned int
Spectrogram::processData(float *buffer,
			 unsigned int bufferLength) {
    unsigned int newLines = 0;

    float waveEnvMin = 0, waveEnvMax = 0;

    for (unsigned int bufferInd = 0; bufferInd < bufferLength; bufferInd++) {
        float value = buffer[bufferInd];

        if (value > waveEnvMax) {
            waveEnvMax = value;
        }
        if (value < waveEnvMin) {
            waveEnvMin = value;
        }

        waveRingBuffer[ringBufferInd] = value;
        ringBufferInd = (ringBufferInd + 1) % ringBufferSize;
        sampleCounter++;

        //std::cout << bufferInd << std::endl;

        if (sampleCounter == fftSize) {
            sampleCounter -= samplesPerLine;

            newLines++;

            //std::cout << "FFT" << std::endl;

            // Fill the fftData array with most recent sample data from the ring buffer:
            float *fftAbs = new float[fftSize];
#ifdef CUDA_FFT
            float *fftData = new float[fftSize];
#else
            std::complex<float> *fftData = new std::complex<float>[fftSize];
#endif
            unsigned int startIndex = (ringBufferInd - fftSize + ringBufferSize) % ringBufferSize;

            for (unsigned int indBuffer = 0; indBuffer < fftSize; indBuffer++) {
                unsigned int sampleIndex = (startIndex + indBuffer) % ringBufferSize;
                fftData[indBuffer] = waveRingBuffer[sampleIndex];
                //std::cout << sampleIndex << " " << indBuffer << " " << fftData[indBuffer] << std::endl;
            }
#ifdef CUDA_FFT
            PerformCUDAFFT(fftData, fftData, fftSize);
#else
            FFTCompute(fftData, fftSize);
#endif
            // Compute the absolute value of each complex Fouerier coefficient and assemble
            // them into a array:
            for (unsigned int indBuffer = 0; indBuffer < fftSize; indBuffer++) {
                fftAbs[indBuffer] = std::abs(fftData[indBuffer]) / ((float)fftSize);
            }
            // Store the new line in the spectrogram:
            addLine(fftAbs, fftSize, waveEnvMin, waveEnvMax);

            delete [] fftData;
            delete [] fftAbs;
        }
    }
    return newLines;
}

void
Spectrogram::removeFoot(unsigned int numLines) {
    for (unsigned int indLine = 0; indLine < numLines; indLine++) {
        assert(!spectrogramData.empty());
        assert(!timeList.empty());
        spectrogramData.pop_front();
        timeList.pop_front();

        waveEnvelopeMin.pop_front();;
        waveEnvelopeMax.pop_front();;

        footTime += deltaTime;
    }
}

void
Spectrogram::addLine(float *fourierData,
                     unsigned int dataLength,
                     float envMin,
                     float envMax) {
    std::vector<float> fourierDataVec;

    if (spectrogramData.size() >= numLines) {
        removeFoot(spectrogramData.size() - numLines + 1);
    }
    fourierDataVec.assign(fourierData, fourierData + dataLength);
    spectrogramData.push_back(fourierDataVec);
    waveEnvelopeMax.push_back(envMax);
    waveEnvelopeMin.push_back(envMin);

    headTime += deltaTime;
    timeList.push_back(headTime);
}

void
Spectrogram::FFTCompute(std::complex<float> *data,
			unsigned int dataLength) {
    for (unsigned int pos= 0; pos < dataLength; pos++) {
        unsigned int mask = dataLength;
        unsigned int mirrormask = 1;
        unsigned int target = 0;

        while (mask != 1) {
            mask >>= 1;
            if (pos & mirrormask)
                target |= mask;
            mirrormask <<= 1;
        }
        if (target > pos) {
            std::complex<float> tmp = data[pos];
            data[pos] = data[target];
            data[target] = tmp;
        }
    }

    for (unsigned int step = 1; step < dataLength; step <<= 1) {
        const unsigned int jump = step << 1;
        const float delta = M_PI / float(step);
        const float sine = sin(delta * 0.5);
        const std::complex<float> mult (-2.*sine*sine, sin(delta));
        std::complex<float> factor(1.0, 0.0);

        for (unsigned int group = 0; group < step; ++group) {
            for (unsigned int pair = group; pair < dataLength; pair += jump) {
                const unsigned int match = pair + step;
                const std::complex<float> prod(factor * data[match]);
                data[match] = data[pair] - prod;
                data[pair] += prod;
            }
            factor = mult * factor + factor;
        }
    }
}

double
Spectrogram::getDeltaTime() {
    return deltaTime;
}

double
Spectrogram::getHeadTime() {
    return headTime;
}

double
Spectrogram::getFootTime() {
    return footTime;
}
