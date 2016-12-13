/*
 *  File    : CVWorker.cpp
 *  Desc    :
 *  Author  : Ismail
 */

#include "cvworker.h"
#include <QDebug>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

CVWorker::CVWorker(QObject *parent) : QObject(parent) { }

CVWorker::~CVWorker() { }


void CVWorker::receiveFilename(QString filename)
{
    try{
        _frameOriginal = cv::imread(filename.toStdString());
    }
    catch (...) {
        emit sendError("Invalid file !");
        return;
    }

    process();

    QImage output((uchar *)_frameProcessed.data,
                  _frameProcessed.cols,
                  _frameProcessed.rows,
                  _frameProcessed.step,
                  QImage::Format_RGB888);
    emit sendFrame(output);
}

void CVWorker::receiveSaveROI(QString filename, int x, int y, int w, int h)
{
    if(x + w <= _frameOriginal.cols && y + h < _frameOriginal.rows)
    {
        _roi.x = x;
        _roi.y = y;
        _roi.width = w;
        _roi.height = h;
        cv::Mat cropped = _frameOriginal(_roi);
        imwrite(filename.toStdString(),cropped);
    }
    else
    {
        printf("Dimensions error\n");
    }
}

void CVWorker::receiveUpdatedWindow(int x, int y, int w, int h, int w2, int h2, int i)
{
    process();
    if(x + w <= _frameProcessed.cols && y + h < _frameProcessed.rows)
    {
        _roi.x = x;
        _roi.y = y;
        _roi.width = w;
        _roi.height = h;
        _cropped = _frameProcessed(_roi);

        int im = cv::INTER_AREA     * (i == 0) +
                 cv::INTER_NEAREST  * (i == 1) +
                 cv::INTER_LINEAR   * (i == 2) +
                 cv::INTER_CUBIC    * (i == 3) +
                 cv::INTER_LANCZOS4 * (i == 4);

        if(w2 > 0 && h2 > 0)
            cv::resize(_cropped, _cropped, cv::Size(w2, h2), 0, 0, im);

        QImage output((const unsigned char *) _cropped.data,
                      _cropped.cols,
                      _cropped.rows,
                      _cropped.step,
                      QImage::Format_RGB888);

        drawRect(x, y, w, h);   // also updates the window
        emit sendROI(output);
    }
}

void CVWorker::receiveSaveWindow(QString filename)
{
    if(!filename.isEmpty())
    {
        cv::cvtColor(_cropped, _cropped, cv::COLOR_BGR2RGB);
        imwrite(filename.toStdString(), _cropped);
    }
}

void CVWorker::drawRect(int x, int y, int w, int h, int thickness)
{
    cv::rectangle(_frameProcessed, cv::Point(x, y), cv::Point(x + w, y + h), cv::Scalar(255, 0, 0), thickness);
    QImage output((const unsigned char *)_frameProcessed.data,
                  _frameProcessed.cols,
                  _frameProcessed.rows,
                  _frameProcessed.step,
                  QImage::Format_RGB888);
    emit sendFrame(output);
}

void CVWorker::process()
{
    cv::cvtColor(_frameOriginal, _frameProcessed, cv::COLOR_BGR2RGB);
}
