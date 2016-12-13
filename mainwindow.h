/*
 *  File    : MainWindow.h
 *  Desc    : Standard Qt GUI unit. Handles UI and events
 *  Author  : Ismail
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtGui>
#include <QFile>
#include <cvworker.h>

#include "params.h"


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
private:
    Ui::MainWindow *ui;
    CVWorker* worker;

    // Parameters
    QString _activeFile = "";
    QString _inputDir, _outputDir, _outputFilename;
    QFile   _outputFile;
    QString _classes;
    QString _defaultFolder = "P:\\ML\\classifier1\\datasets\\AHDBPage"; // QDir::currentPath();
    QFileInfoList _files;
    int _fileIndex = 0;
    bool _ok = false;

    Params _p;
    Params default_parameters = {
        "",             // outputFileName
        "",             // outputFolderName
        0, 0, 128, 128, // Sliding window's x, y, width, height
        64, 64,         // X and Y steps
        16, 16, 0,      // Resized output's width, height, and interpolation mode
        true,           // resize?
        2,              // number of Classes
        "L1;L2",        // Label of Classes
        0, 0            // default formats for output
    };

    // Operations
    int _curClass = 0;
    int _curIter =  1;

    // Private methods
    bool eventFilter(QObject *obj, QEvent *event);
    bool saveParameters(Params& p);
    void setLabels(QString classes);
    bool slide(int& x, int& y, const int w1, const int h1);

    // Setup Methods
    void setupUI();
    void setupConnections();
    void setupParams(Params p);

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

signals:
    void sendFilename(QString filename);
    void sendViewClicked(int x, int y);

    void sendExport(QString filename, int x, int y, int w, int h);
    void sendUpdateWindow(int x, int y, int w, int h, int w2, int h2, int interpolation);
    void sendSaveWindow(QString filename);

public slots:
    void receiveChoosePicture();
    void receiveFrame(QImage frame);
    void receiveViewClicked(int x, int y);
    void receiveToggleOriginalSize();
    void receiveUpdateWindowClick();
    void receiveCheckResizeOutput();
    void receiveSetLabels();
    void receiveChooseInputFolder();
    void receiveSelectOutputFolder();
    void receiveSelectOutputFilename();
    void receiveOutput(QImage frame);
    void receiveNextImage();
    void receiveError(QString error);
    void receiveReset();
    void receiveExportParameters();
    void receiveImportParameters();
    void receiveStart();

    void receiveClassCellClicked(int row,int col);
    void receiveSaveAction();
    void receiveSkipAction();

};

#endif // MAINWINDOW_H
