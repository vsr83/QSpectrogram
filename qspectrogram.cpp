#include "qspectrogram.h"

#include <QStylePainter>

QSpectrogram::QSpectrogram(Spectrogram *_spectrogram,
			   QWidget *parent,
			   double _minFreq,
			   double _maxFreq,
			   double _minAmpl,
			   double _maxAmpl) : QWidget(parent) {
    spectrogram = _spectrogram;
    minFreq = _minFreq;
    maxFreq = _maxFreq;
    minAmpl = _minAmpl;
    maxAmpl = _maxAmpl;

    paddingX = 20;
    paddingY = 20;
    xlabelSpacing = 40;
    ylabelSpacing = 80;

    freqTickBig   = 10000.0;
    freqTickSmall = 1000.0;

    drawMode = DRAWMODE_SCROLL;
    layoutMode = LAYOUT_HORIZONTAL;
    drawTimeGrid = true;
    drawFreqGrid = true;
    drawColorbar = true;
    drawSpectrum = true;
    drawWaveform = true;

    waveformWidth = 100;
    spectrumWidth = 100;
    colorBarWidth = 50;

    backgroundColor = QColor(0, 0, 0);
    gridColor = QColor(128, 128, 128);

    timeScroll = 0.0;
    image = 0;

    logScaleFreq = true;
    logScaleAmpl = true;

    QVector<float> color0, color1, color2, color3, color4;
    color0.push_back(0.0);  color0.push_back(0.0);  color0.push_back(32.0);color0.push_back(0.0);
    color1.push_back(0.0);color1.push_back(0.0);  color1.push_back(255.0);color1.push_back(0.25);
    color2.push_back(0.0);color2.push_back(255.0);color2.push_back(255.0);color2.push_back(0.5);
    color3.push_back(255.0);color3.push_back(255.0);color3.push_back(0.0);color3.push_back(0.75);
    color4.push_back(255.0);color4.push_back(0.0);color4.push_back(0.0);color4.push_back(1.0000);
    colormap.push_back(color0);
    colormap.push_back(color1);
    colormap.push_back(color2);
    colormap.push_back(color3);
    colormap.push_back(color4);
}

void
QSpectrogram::evalColormap(float value, int &r, int &g, int &b) {
    int nRGB = colormap.size();

    QVector<float> RGB;
    QVector<float> RGBnext;

    for (int indRGB = 0; indRGB < nRGB-1; indRGB++) {
        RGB = colormap[indRGB];
        RGBnext = colormap[indRGB+1];

        if (value < RGB[3]) {
            r = (int)RGB[0];
            g = (int)RGB[1];
            b = (int)RGB[2];
            return;
        } else if (value <= RGBnext[3]) {
            float valcoeff = (value - RGB[3]) / (RGBnext[3] - RGB[3]);

            r = (int)(RGB[0] * (1.0 - valcoeff) + valcoeff * RGBnext[0]);
            g = (int)(RGB[1] * (1.0 - valcoeff) + valcoeff * RGBnext[1]);
            b = (int)(RGB[2] * (1.0 - valcoeff) + valcoeff * RGBnext[2]);
            return;
        }
    }
    r = (int)RGBnext[0];
    g = (int)RGBnext[1];
    b = (int)RGBnext[2];
}

void
QSpectrogram::paintEvent(QPaintEvent *event) {
    QStylePainter painter(this);
    painter.drawPixmap(0, 0, pixmap);
    Q_UNUSED(event);
}

void
QSpectrogram::resizeEvent(QResizeEvent *event) {
    plotwidth  = width()  - paddingX * 2 - ylabelSpacing;
    plotheight = height() - paddingY * 2 - xlabelSpacing;
    plotx = paddingX + ylabelSpacing;
    ploty = paddingY;

    if (layoutMode == LAYOUT_HORIZONTAL) {
        if (drawSpectrum) {
            plotwidth -= spectrumWidth;
            plotwidth -= colorBarWidth;
        }
        if (drawWaveform) {
            plotheight -= waveformWidth;
            ploty += waveformWidth;
        }
    }
    if (layoutMode == LAYOUT_VERTICAL) {
        if (drawSpectrum) {
            ploty += spectrumWidth;
            plotheight -= spectrumWidth;
            plotheight -= colorBarWidth;
        }
        if (drawWaveform) {
            plotwidth -= waveformWidth;
        }
    }

    renderImage(0, true);
    refreshPixmap();
    Q_UNUSED(event);
}

int
QSpectrogram::freqToPixel(double freq) {
    double minCoord;
    double imageSize;

    //qDebug("freqToPixel %d", freq );

    if (layoutMode == LAYOUT_HORIZONTAL) {
        minCoord  = (double)ploty;
        imageSize = (double)plotheight;
    } else if (layoutMode == LAYOUT_VERTICAL) {
        minCoord  = (double)plotx;
        imageSize = (double)plotwidth;
    } else {
        qWarning("freqToPixel : Non-supported Layout Mode %d!", layoutMode);
        return 0;
    }

    if (logScaleFreq) {
        // a + b * log10(minFreq) = maxCoord
        // a + b * log10(maxFreq) = minCoord
        // => b * (log10(minFreq) - log10(maxFreq) = maxCoord - minCoord
        // => b = (maxCoord - minCoord) / (log10(minFreq) - log10(maxFreq)
        // => a = maxCoord - b * log10(minFreq)

        double b = imageSize / (log10(minFreq) - log10(maxFreq));
        double a = minCoord - b * log10(maxFreq);

        //c + d * (plotx + plotwidth) = plotx
        //c + d * (plotx) = plotx + plotwidth
        // => d * plotwidth = -plotwidth
        // => d = -1;
        // => c - plotx = plotx + plotwidth
        // c = 2 * plotx + plotwidth;
        // 2 * plotx + plotwidth - x

        if (layoutMode == LAYOUT_HORIZONTAL) {
            return (int)(a + b * log10(freq));
        } else if (layoutMode == LAYOUT_VERTICAL) {
            return (int)(2 * minCoord + imageSize - a - b * log10(freq));
        }
    } else {
        // a + b * minFreq = maxCoord
        // a + b * maxFreq = minCoord
        // => b * (minFreq - maxFreq) = plotsize
        // => b = plotsize  / (minFreq - maxFreq)

        double b = imageSize / (minFreq - maxFreq);
        double a = minCoord - maxFreq * b;
        if (layoutMode == LAYOUT_HORIZONTAL) {
            return (int)(a + b * freq);
        } else if (layoutMode == LAYOUT_VERTICAL) {
            return (int)(2 * minCoord + imageSize - a - b * freq);
        }
    }
    return 0; // dummy
}

int
QSpectrogram::timeToPixel(double time) {
    double deltaTime = spectrogram->getDeltaTime();
    double headTime  = spectrogram->getHeadTime();
    double timeWidth = ((double)plotwidth) * deltaTime;

    // a + b * headTime = plotx + plotwidth
    // a + b * (headTime - timeWidth ) = plotx
    // => plotwidth = b * timeWidth
    // => b = plotWidth / timeWidth
    // => a = plotx + plotwidth - b * headTime

    if (layoutMode == LAYOUT_HORIZONTAL) {
        double b = plotwidth / timeWidth;
        double a = plotx + plotwidth - b * headTime;

        double x_double = a  + b * (time - timeScroll);
        return (int) x_double;
    } else if (layoutMode == LAYOUT_VERTICAL) {
        double b = plotheight / timeWidth;
        double a = ploty + plotheight - b * headTime;

        double y_double = a  + b * (time - timeScroll);
        return (int) y_double;
    }
    return 0; // dummy
}

void
QSpectrogram::drawGrid(QPainter &painter) {
    double deltaTime = spectrogram->getDeltaTime();
    double headTime  = spectrogram->getHeadTime();
    double timeWidth = (double)(plotwidth) * deltaTime;

    qDebug("deltaTime %f, headTime %f, timeWidth %f", deltaTime, headTime, timeWidth);

    QPen thickPen, thinPen, dashPen;
    thinPen.setStyle(Qt::SolidLine);
    thinPen.setWidth(1);
    thinPen.setColor(gridColor);
    dashPen.setStyle(Qt::DotLine);
    dashPen.setWidth(1);
    dashPen.setColor(gridColor);

    painter.setPen(dashPen);
    painter.drawRect(plotx, ploty, plotwidth, plotheight);
    for (unsigned int indSec = 1; indSec <= (unsigned int)(10.0*timeWidth); indSec++) {

        if (layoutMode == LAYOUT_HORIZONTAL) {
            int x = timeToPixel(headTime - 0.1 * (double)indSec);
            if (x > (int)plotx && x < (int)(plotwidth + plotx)) {
                painter.drawLine(x, ploty, x, ploty + plotheight);
            }
        } else if (layoutMode == LAYOUT_VERTICAL) {
            int y = timeToPixel(headTime - 0.1 * (double)indSec);
            if (y > (int)ploty && y < (int)(plotheight + ploty)) {
                painter.drawLine(plotx, y, plotx + plotwidth, y);
            }
        }
    }
    painter.setPen(thinPen);
    for (unsigned int indSec = 0; indSec <= (unsigned int)timeWidth; indSec++) {
        if (layoutMode == LAYOUT_HORIZONTAL) {
            int x = timeToPixel(headTime - (double)indSec);
            if (x >= (int)plotx && x <= (int)(plotwidth + plotx)) {
                painter.drawLine(x, ploty, x, ploty + plotheight + 10);
            }
        } else if (layoutMode == LAYOUT_VERTICAL) {
            int y = timeToPixel(headTime - (double)indSec);
            if (y >= (int)ploty && y <= (int)(plotheight + ploty)) {
                painter.drawLine(plotx-10, y, plotx + plotwidth, y);
            }
        }
    }

    if (logScaleFreq) {
        for (int lfreq = (int) log10(minFreq); lfreq <= (int)log10(maxFreq); lfreq++) {
            double freq = pow(10.0, lfreq);

            painter.setPen(dashPen);
            for (int fmult = 1; fmult < 10; fmult++) {
                if (layoutMode == LAYOUT_HORIZONTAL) {
                    int y = freqToPixel(freq*(double)fmult);
                    if (y >= (int)ploty && y <= (int)(ploty + plotheight)) {
                        painter.drawLine(plotx, y, plotx + plotwidth, y);
                    }
                } else if (layoutMode == LAYOUT_VERTICAL) {
                    int x = freqToPixel(freq*(double)fmult);
                    if (x >= (int)plotx && x <= (int)(plotx + plotwidth)) {
                        painter.drawLine(x, ploty, x, ploty + plotheight);
                    }
                }
            }

            painter.setPen(thinPen);
            if (layoutMode == LAYOUT_HORIZONTAL) {
                int y = freqToPixel(freq);
                painter.drawText(QRect(paddingX-15 , y-10, ylabelSpacing, 20), Qt::AlignRight | Qt::AlignVCenter,
                                  QString::number(freq) + QString(" Hz"));
                if (y >= (int)ploty && y <= (int)(ploty + plotheight)) {
                    painter.drawLine(plotx-10, y, plotx + plotwidth, y);
                }
            } else if (layoutMode == LAYOUT_VERTICAL) {
                int x = freqToPixel(freq);
                painter.drawText(QRect(x-30, ploty + plotheight + 15, 60, 20), Qt::AlignHCenter,
                                  QString::number(freq) + QString(" Hz"));
                if (x >= (int)plotx && x <= (int)(plotx + plotwidth)) {
                    painter.drawLine(x, ploty, x, ploty + plotheight + 10);
                }
            }
        }
    } else {
        for (double freq = floor(minFreq/freqTickSmall); freq <= floor(maxFreq/freqTickSmall); freq += 1) {
            painter.setPen(dashPen);
            if (layoutMode == LAYOUT_HORIZONTAL) {
                int y = freqToPixel(freq * freqTickSmall);
                if (y >= (int)ploty && y <= (int)(ploty + plotheight)) {
                    painter.drawLine(plotx-10, y, plotx + plotwidth, y);
                    painter.drawText(QRect(paddingX-15 , y-10, ylabelSpacing, 20), Qt::AlignRight | Qt::AlignVCenter,
                                      QString::number(freq * freqTickSmall) + QString(" Hz"));
                }
            } else if (layoutMode == LAYOUT_VERTICAL) {
                int x = freqToPixel(freq * freqTickSmall);
                if (x >= (int)plotx && x <= (int)(plotx + plotwidth)) {
                    painter.drawLine(x, ploty, x, ploty + plotheight);
                }

            }
        }
        for (double freq = floor(minFreq/freqTickBig); freq <= floor(maxFreq/freqTickBig); freq += 1) {
            painter.setPen(thinPen);
            if (layoutMode == LAYOUT_HORIZONTAL) {
                int y = freqToPixel(freq * freqTickBig);
                if (y >= (int)ploty && y <= (int)(ploty + plotheight)) {
                    painter.drawLine(plotx-10, y, plotx + plotwidth, y);
                }
            } else if (layoutMode == LAYOUT_VERTICAL) {
                int x = freqToPixel(freq * freqTickBig);
                if (x >= (int)plotx && x <= (int)(plotx + plotwidth)) {
                    painter.drawLine(x, ploty, x, ploty + plotheight + 10);
                    painter.drawText(QRect(x, ploty + plotheight + 10, 80, 20), Qt::AlignHCenter,
                                      QString::number(freq * freqTickBig) + QString(" Hz"));
                }
            }
        }
    }
}

void
QSpectrogram::drawWaveformPlot(QPainter &painter) {
    std::list<float>::iterator itMin = spectrogram->waveEnvelopeMin.end();
    std::list<float>::iterator itMax = spectrogram->waveEnvelopeMax.end();

    int pixel, pixelMin;
    if (layoutMode == LAYOUT_HORIZONTAL) {
        pixel= plotx + plotwidth;
        pixelMin = plotx;
    }
    if (layoutMode == LAYOUT_VERTICAL) {
        pixel= ploty + plotheight;
        pixelMin = ploty;
    }
    for (;;) {
        if (itMin == spectrogram->waveEnvelopeMin.begin()) break;
        if (itMax == spectrogram->waveEnvelopeMax.begin()) break;
        if (pixel < plotx) break;

        int minValue = (int)(0.5 * ((float)waveformWidth)*(*itMin));
        int maxValue = (int)(0.5 * ((float)waveformWidth)*(*itMax));

        if (layoutMode == LAYOUT_HORIZONTAL) {
            painter.drawLine(pixel, ploty - waveformWidth / 2 + minValue,
                             pixel, ploty - waveformWidth / 2 + maxValue);
        }
        if (layoutMode == LAYOUT_VERTICAL) {
            painter.drawLine(plotx + plotwidth + waveformWidth / 2 + minValue, pixel,
                             plotx + plotwidth + waveformWidth / 2 + maxValue, pixel);
        }

        pixel--;
        itMin--;
        itMax--;
    }
}

void
QSpectrogram::drawSpectrumPlot(QPainter &painter) {
    if (spectrogram->spectrogramData.empty()) return;

    std::vector <float> lineData = spectrogram->spectrogramData.back();

    Q_ASSERT(!spectrogram->spectrogramData.empty());
    Q_ASSERT(spectrogram->frequencyList.size() >= lineData.size());
    Q_ASSERT(lineData.size() > 0);

    QPen thinPen;
    thinPen.setStyle(Qt::SolidLine);
    thinPen.setWidth(1);
    thinPen.setColor(gridColor);
    painter.setPen(thinPen);

    float minAmplS = 100.0 * minAmpl;

    qDebug("lineData.size() = %d", lineData.size());
    int prevpixel, prevvalue;
    for (unsigned int ind_freq = 0; ind_freq < lineData.size(); ind_freq++) {
        int pixel = freqToPixel(spectrogram->frequencyList[ind_freq]);

        float value = lineData[ind_freq];
        float lvalue = (((float)spectrumWidth) / (log10(maxAmpl) - log10(minAmplS))) * ( log10(value) - log10(minAmplS) );
        int ivalue = (int) lvalue;

        if (ivalue < 0) ivalue = 0;

        if (layoutMode == LAYOUT_HORIZONTAL) {
            if (ind_freq > 0 && pixel >= ploty && prevpixel >= ploty && pixel <= ploty + plotheight && prevpixel <= ploty + plotheight) {
                //painter.drawLine(plotx + plotwidth + prevvalue + 5, prevpixel,
                //                 plotx + plotwidth + ivalue + 5, pixel);
                painter.drawLine(plotx + plotwidth + 5, pixel,
                                 plotx + plotwidth + ivalue + 5, pixel);
            }
        } else if (layoutMode == LAYOUT_VERTICAL) {
            if (ind_freq > 0 && pixel >= plotx && prevpixel >= plotx && pixel <= plotx + plotwidth && prevpixel <= plotx + plotwidth) {
                painter.drawLine(pixel, ploty - ivalue, prevpixel, ploty - prevvalue);
            }

        }

        prevvalue = ivalue;
        prevpixel = pixel;
    }
}

void
QSpectrogram::renderImage(unsigned int newLines, bool redraw) {

    if (image) {
        QImage *oldimage = image;
        image = new QImage(plotwidth, plotheight, QImage::Format_RGB32);
        image->fill(backgroundColor);

        if (layoutMode == LAYOUT_HORIZONTAL) {
            QPainter painter(image);
            //QImage tmpImage = oldimage->copy(newLines-1, 0, plotwidth-newLines+1, plotheight);
            QImage tmpImage = oldimage->copy(newLines, 0, plotwidth-newLines+1, plotheight);
            painter.drawImage(0, 0, tmpImage);
        } else if (layoutMode == LAYOUT_VERTICAL) {
            QPainter painter(image);
            //QImage tmpImage = oldimage->copy(0, newLines-1, plotwidth, plotheight-newLines+1);
            QImage tmpImage = oldimage->copy(0, newLines, plotwidth, plotheight-newLines+1);
            painter.drawImage(0, 0, tmpImage);
        }
        delete oldimage;
    } else {
        image = new QImage(plotwidth, plotheight, QImage::Format_RGB32);
    }

    unsigned int ind_line = 0;

    if (redraw) {
        newLines = spectrogram->spectrogramData.size();
    }

    std::vector<int> pixelList;
    for (unsigned int ind_freq = 0; ind_freq < spectrogram->frequencyList.size(); ind_freq++)
        pixelList.push_back(freqToPixel(spectrogram->frequencyList[ind_freq]));

    for (std::list<std::vector<float> >::iterator it = spectrogram->spectrogramData.begin();
         it != spectrogram->spectrogramData.end(); it++) {

        if (ind_line > spectrogram->spectrogramData.size() - newLines - 1) {
            std::vector <float> lineData = *it;
            if (layoutMode == LAYOUT_HORIZONTAL) {
                int prevy = plotheight;
                float prevvalue = 0.0;

                for (unsigned int ind_freq = 0; ind_freq < lineData.size()/2; ind_freq++) {
                    int y = pixelList[ind_freq]-ploty;
                    int x = plotwidth - spectrogram->spectrogramData.size() + ind_line;                    

                    if (y < prevy && y > 0 && x >= 0 && x < (int)plotwidth && prevy < (int)plotheight) {
                        //qDebug("%d %d", y, prevy);
                        float value = lineData[ind_freq];
                        int r, g, b;

                        //value = (log10(value) + 3.0) / 5.0;

                        if (logScaleAmpl) {
                            value = (1.0 / (log10(maxAmpl) - log10(minAmpl))) * ( log10(value) - log10(minAmpl) );
                        } else {
                            value = (value - minAmpl)/(maxAmpl - minAmpl);
                        }
                        if (value < 0) {
                            value = 0.0;
                        }
                        if (value > 0.01) {

                            for (int yi = y; yi < prevy; yi++) {
                                float interp_value = prevvalue + (value - prevvalue) * (float)(yi - prevy)/((float)(y - prevy));
                                evalColormap(interp_value, r, g, b);
                                int ivalue = qRgb(r, g, b);

                                if (yi >= 0 && yi < (int)plotheight)
                                image->setPixel(x, yi, ivalue);
                            }
                        }
                        prevvalue = value;
                    }
                    prevy = y;
                }
            } else if (layoutMode == LAYOUT_VERTICAL) {
                int prevx = 0;

                for (unsigned int ind_freq = 0; ind_freq < lineData.size()/2; ind_freq++) {
                    int x = pixelList[ind_freq]-plotx;
                    int y = plotheight - spectrogram->spectrogramData.size() + ind_line;

                    if (x > prevx && x < (int)plotwidth && y >= 0 && y < (int)plotheight && prevx < (int)plotwidth) {
                        //qDebug("%d %d", y, prevy);
                        float value = lineData[ind_freq];
                        int r, g, b;

                        // a + b * log10(maxAmpl) = 1.0
                        // a + b * log10(minAmpl) = 0
                        // b * (log10(maxAmpl) - log10(minAmpl) = 1.0
                        // => b = 1.0 / (log10(maxAmpl) - log10(minAmpl))
                        // a = - b * log10(minAmpl)
                        //value = (log10(value) + 3.0) / 5.0;

                        if (logScaleAmpl) {
                            value = (1.0 / (log10(maxAmpl) - log10(minAmpl))) * ( log10(value) - log10(minAmpl) );
                        } else {
                            value = (value - minAmpl)/(maxAmpl - minAmpl);
                        }

                        if (value < 0) {
                            value = 0.0;
                        }
                        if (value > 0.01) {

                            evalColormap(value, r, g, b);
                            int ivalue = qRgb(r, g, b);

                            for (int xi = prevx; xi < x; xi++)
                                if (xi >= 0 && xi < (int)plotwidth)
                                image->setPixel(xi, y, ivalue);
                        }
                    }
                    prevx = x;
                }

            }
        }
        ind_line++;
    }
}

void
QSpectrogram::drawColorbarPlot(QPainter &painter) {
    int r, g, b;
    QPen colorPen;

    int startX = plotx + plotwidth + spectrumWidth;
    for (int y = ploty; y < ploty + plotheight; y++) {
        float value = 1.0 - ((float)(y-ploty)) / ((float)plotheight);

        evalColormap(value, r, g, b);
        colorPen.setStyle(Qt::SolidLine);
        colorPen.setWidth(1);
        colorPen.setColor(QColor(r, g, b));
        painter.setPen(colorPen);

        if (drawSpectrum) {
            painter.drawLine(startX + colorBarWidth/2, y, startX + colorBarWidth - 5, y);
        }
    }
    colorPen.setColor(gridColor);
    painter.setPen(colorPen);
    painter.drawRect(startX + colorBarWidth/2, ploty, colorBarWidth/2 - 5 , plotheight);

    float logMin = log10(minAmpl);
    float logMax = log10(maxAmpl);

    int logMinInt = (int)floor(logMin);
    int logMaxInt = (int)floor(logMax);

    if (logMinInt > logMaxInt) exit(0);

    for (int logIt = logMinInt; logIt <= logMaxInt; logIt++) {
        double value = pow(10.0, (double)logIt);

        double dBvalue = 20.0 * logIt;
        if (value >= minAmpl && value <= maxAmpl) {
            int y = ploty + plotheight - (int)((double)plotheight) * ((double)logIt - logMin)/(logMax - logMin);
            //qDebug("%d %d %f %d", logMinInt, logMaxInt, value, y);
            painter.drawLine(startX + colorBarWidth/2-5, y, startX + colorBarWidth - 5, y);
            painter.drawText(QRect(startX-35, y-6, colorBarWidth, 20), Qt::AlignRight,
                              QString::number((int)dBvalue) + QString(" dB"));
        }
    }
}

void
QSpectrogram::refreshPixmap() {
    pixmap = QPixmap(size());
    pixmap.fill(backgroundColor);
    QPainter painter(&pixmap);

    if (image) {
        painter.drawImage(plotx, ploty, *image);
    }
    if (drawSpectrum) {
        drawSpectrumPlot(painter);
    }
    if (drawWaveform) {
        drawWaveformPlot(painter);
    }
    if (drawColorbar) {
        drawColorbarPlot(painter);
    }
    drawGrid(painter);
    update();
}

void
QSpectrogram::toggleColorbar(bool visible) {
   drawColorbar = visible;
   resizeEvent(0);
}

void
QSpectrogram::toggleWaveform(bool visible) {
    drawWaveform = visible;
    resizeEvent(0);
}

void
QSpectrogram::toggleSpectrum(bool visible) {
    drawSpectrum = visible;
    resizeEvent(0);
}

void
QSpectrogram::toggleTimeGrid(bool visible) {
    drawTimeGrid = visible;
    resizeEvent(0);
}

void
QSpectrogram::toggleFreqGrid(bool visible) {
    drawFreqGrid = visible;
    resizeEvent(0);
}

void
QSpectrogram::toggleLogScaleFreq(bool logscale) {
    logScaleFreq = logscale;
    resizeEvent(0);
}

void
QSpectrogram::toggleLogScaleAmpl(bool logscale) {
    logScaleAmpl = logscale;
    resizeEvent(0);
}

void
QSpectrogram::setMaxFreq(double _maxFreq) {
    maxFreq = _maxFreq;
    refreshPixmap();
}

void
QSpectrogram::setMinFreq(double _minFreq) {
    minFreq = _minFreq;
    refreshPixmap();
}

void
QSpectrogram::setMaxAmpl(double _maxAmpl) {
    maxAmpl = _maxAmpl;
    refreshPixmap();
}

void
QSpectrogram::setMinAmpl(double _minAmpl) {
    minAmpl = _minAmpl;
    refreshPixmap();
}

void
QSpectrogram::setLayoutMode(unsigned int layoutMode) {

}

void
QSpectrogram::processData(float *buffer, unsigned int bufferLength) {
    unsigned int newLines = spectrogram->processData(buffer, bufferLength);

    double minValue = 0.0;
    for (unsigned int indBuffer = 0; indBuffer < bufferLength; indBuffer++) {
        if (fabs(buffer[indBuffer]) > minValue)
            minValue = fabs(buffer[indBuffer]);
    }

    //qDebug("numLines = %d, maxAbs = %f", (int)spectrogram->spectrogramData.size(), minValue);
    //qDebug("%f %f %f", buffer[0], buffer[1], buffer[2]);
    renderImage(newLines, false);
    refreshPixmap();
}

