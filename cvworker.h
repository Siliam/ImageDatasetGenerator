/*
 *  File    : CVWorker.h
 *  Desc    : Handles opening files and extracting regions of interest from imported photos
 *  Author  : Ismail
 */

#ifndef CVWORKER_H
#define CVWORKER_H

#include <QObject>
#include <QImage>
#include <QString>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui//highgui.hpp>

#include "params.h"

class CVWorker : public QObject
{
    Q_OBJECT
private:
    cv::Mat _frameOriginal;
    cv::Mat _frameProcessed;
    cv::Mat _cropped;
    cv::Rect _roi;
    int _curX, _curY;

    void process();
    void drawRect(int x, int y, int w, int h, int thickness = 3);

public:
    explicit CVWorker(QObject *parent = 0);
    ~CVWorker();

signals:
    void sendFrame(QImage frameProcessed);
    void sendROI(QImage ROI);
    void sendError(QString error);

public slots:
    void receiveFilename(QString filename);
    void receiveSaveROI(QString filename, int x, int y, int w, int h);
    void receiveUpdatedWindow(int x, int y, int w, int h, int w2, int h2, int i);
    void receiveSaveWindow(QString filename);
};

#endif // OPENCVWORKER_H
