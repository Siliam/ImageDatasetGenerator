/*
 *  File    : MainWindow.cpp
 *  Desc    :
 *  Author  : Ismail
 */

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtGui>
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent): QMainWindow(parent), ui(new Ui::MainWindow)
{
    worker = new CVWorker();

    setupUI();
    setupParams(default_parameters);
    setupConnections();
}

void MainWindow::setupParams(Params p)
{
    ui->lblOutputFilename->setText(p.outputFileName);
    ui->txtOutputFolder->setText(p.outputFolderName);

    ui->sbX->setValue(p.window_x);
    ui->sbY->setValue(p.window_y);
    ui->sbW->setValue(p.window_w);
    ui->sbH->setValue(p.window_h);
    ui->sbDx->setValue(p.dy);
    ui->sbDy->setValue(p.dx);

    ui->sbOutputW->setValue(p.output_w);
    ui->sbOutputH->setValue(p.output_h);
    ui->cmbInterpolation->setCurrentIndex(p.output_interpolation);
    ui->sbOutputW->setEnabled(p.output_resized);
    ui->sbOutputH->setEnabled(p.output_resized);
    ui->cbResizeOutput->setChecked(p.output_resized);

    ui->sbNumClasses->setValue(p.classesCount);
    setLabels(p.classesLabels);
    _classes = p.classesLabels;
    ui->cmbOutputClass->setCurrentIndex(p.outputClassFormat);
    ui->cmbOutputFormat->setCurrentIndex(p.outputFilenameFormat);
}

void MainWindow::setupUI()
{
    ui->setupUi(this);
    ui->lblView = new QLabel();
    ui->lblView->setBackgroundRole(QPalette::Base);
    ui->lblView->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    ui->lblView->installEventFilter(this);

    ui->scrollArea->setBackgroundRole(QPalette::Dark);
    ui->scrollArea->setWidget(ui->lblView);
    ui->scrollArea->setWidgetResizable(false);

    ui->lblViewOutput = new QLabel();
    ui->lblViewOutput->setBackgroundRole(QPalette::Base);
    ui->lblViewOutput->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

    ui->scrollAreaOutput->setBackgroundRole(QPalette::Dark);
    ui->scrollAreaOutput->setWidget(ui->lblViewOutput);
    ui->scrollAreaOutput->setWidgetResizable(false);

    ui->tblClasses->horizontalHeader()->hide();
    ui->tblClasses->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tblClasses->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tblClasses->verticalHeader()->hide();
    ui->tblClasses->verticalHeader()->setDefaultSectionSize(21);
    ui->tblClasses->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tblClasses->setEnabled(false);

    ui->btnNext->hide();
}

void MainWindow::setupConnections()
{
    connect(ui->actionLoadImage, SIGNAL(triggered(bool)), this, SLOT(receiveChoosePicture()));
    connect(ui->actionLoadFolder, SIGNAL(triggered(bool)), this, SLOT(receiveChooseInputFolder()));
    connect(ui->actionQuit, SIGNAL(triggered(bool)), this, SLOT(close()));

    connect(ui->btnConfirm, SIGNAL(clicked()), this, SLOT(receiveStart()));
    connect(ui->btnReset, SIGNAL(clicked()), this, SLOT(receiveReset()));
    connect(ui->btnNext, SIGNAL(clicked()), this, SLOT(receiveNextImage()));

    connect(ui->btnSave, SIGNAL(clicked(bool)), this, SLOT(receiveSaveAction()));
    connect(ui->btnSkip, SIGNAL(clicked(bool)), this, SLOT(receiveSkipAction()));

    // Handled by the MainWindow::eventFilter
    connect(this, SIGNAL(sendViewClicked(int,int)), this, SLOT(receiveViewClicked(int,int)));

    connect(ui->cbOriginalSize, SIGNAL(clicked(bool)), this, SLOT(receiveToggleOriginalSize()));
    connect(ui->cbResizeOutput, SIGNAL(clicked(bool)), this, SLOT(receiveCheckResizeOutput()));
    connect(ui->btnSetLabels, SIGNAL(clicked(bool)), this,SLOT(receiveSetLabels()));
    connect(ui->btnOutputFolder, SIGNAL(clicked(bool)), this, SLOT(receiveSelectOutputFolder()));
    connect(ui->btnOutputFilename, SIGNAL(clicked(bool)), this, SLOT(receiveSelectOutputFilename()));

    connect(ui->tblClasses, SIGNAL(cellClicked(int,int)), this, SLOT(receiveClassCellClicked(int,int)));

    connect(ui->actionExportParameters, SIGNAL(triggered(bool)), this, SLOT(receiveExportParameters()));
    connect(ui->actionImportParameters, SIGNAL(triggered(bool)), this, SLOT(receiveImportParameters()));

    // connect(ui->btnWindowParams, SIGNAL(clicked(bool)), this, SLOT(receiveUpdateWindowClick()));


    connect(this, SIGNAL(sendFilename(QString)), worker, SLOT(receiveFilename(QString)));
    connect(worker, SIGNAL(sendFrame(QImage)), this, SLOT(receiveFrame(QImage)));
    connect(worker, SIGNAL(sendROI(QImage)), this, SLOT(receiveOutput(QImage)));
    connect(worker, SIGNAL(sendError(QString)), this, SLOT(receiveError(QString)));
    connect(this, SIGNAL(sendUpdateWindow(int, int, int, int, int, int, int)),
            worker, SLOT(receiveUpdatedWindow(int, int, int, int, int, int, int)));
    connect(this, SIGNAL(sendSaveWindow(QString)), worker, SLOT(receiveSaveWindow(QString)));
}




void MainWindow::receiveChoosePicture()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Select an image"),
                                                    _defaultFolder,
                                                    tr("Image Files (*.png *.jpg *.bmp *.tif)"));
    if(!filename.isEmpty())
        _activeFile = filename;
}

void MainWindow::receiveChooseInputFolder()
{
    _inputDir = QFileDialog::getExistingDirectory(this, tr("Choose a directory to batch-process"),
                                                _defaultFolder,
                                                QFileDialog::ShowDirsOnly);
    if(!_inputDir.isEmpty())
    {
         ui->btnNext->show();
        _fileIndex = 0;
        _files = QDir(_inputDir).entryInfoList();
        for(;_fileIndex < _files.size() && _files[_fileIndex].isDir(); _fileIndex++);
        if(_fileIndex < _files.size())
        {
            _activeFile = _files[_fileIndex].filePath();
            emit sendFilename(_activeFile);
        }
        else
        {
            receiveError("No (more) files to process");
        }
    }
}

void MainWindow::receiveFrame(QImage frame)
{
    ui->lblWidth->setText(QString("Width : %1").arg(frame.width()) + " px");
    ui->lblHeight->setText(QString("Height : %1").arg(frame.height()) +  " px");
    ui->lblView->setPixmap(QPixmap::fromImage(frame));
    ui->lblView->adjustSize();
}

void MainWindow::receiveViewClicked(int x, int y)
{
    if(_ok)
    {
        int w = ui->sbW->value();
        int h = ui->sbH->value();
        int W = ui->lblView->pixmap()->width();
        int H = ui->lblView->pixmap()->height();
        x = (x + w > W) ? x = W - w : x;
        y = (y + h > H) ? y = H - h - 1 : y;

        ui->sbX->setValue(x);
        ui->sbY->setValue(y);
        receiveUpdateWindowClick();
    }
}

void MainWindow::receiveToggleOriginalSize()
{
    if(ui->cbOriginalSize->isChecked())
    {
        ui->lblView->resize(ui->lblView->pixmap()->size());
        ui->lblView->setScaledContents(false);
    }
    else
    {
        ui->lblView->resize(ui->scrollArea->size() * 0.99);
        ui->lblView->setScaledContents(true);
    }
}

bool MainWindow::eventFilter( QObject* watched, QEvent* event ) {
    if ( watched != ui->lblView )
        return false;
    if ( event->type() != QEvent::MouseButtonPress )
        return false;
    const QMouseEvent* const me = static_cast<const QMouseEvent*>( event );
    const QPoint p = me->pos();
    emit sendViewClicked(p.x(), p.y());

    return false;
}

void MainWindow::receiveUpdateWindowClick()
{
    int x = ui->sbX->value();
    int y = ui->sbY->value();
    int w = ui->sbW->value();
    int h = ui->sbH->value();
    int w2 = ui->sbOutputW->value() * ui->cbResizeOutput->isChecked();
    int h2 = ui->sbOutputH->value() * ui->cbResizeOutput->isChecked();
    int i = ui->cmbInterpolation->currentIndex();

    if(_ok)
        emit sendUpdateWindow(x, y, w, h, w2, h2, i);
}

void MainWindow::receiveCheckResizeOutput()
{
    ui->sbOutputW->setEnabled(ui->cbResizeOutput->isChecked());
    ui->sbOutputH->setEnabled(ui->cbResizeOutput->isChecked());
}

void MainWindow::receiveSelectOutputFolder()
{
    _outputDir = QFileDialog::getExistingDirectory(this, tr("Choose output directory"),
                                                QDir::currentPath(),
                                                QFileDialog::ShowDirsOnly);
    ui->txtOutputFolder->setText(_outputDir);
}

void MainWindow::receiveSetLabels()
{
    QInputDialog inputDialog;
    inputDialog.setOptions(QInputDialog::NoButtons);
    bool ok = false;
    QString classes;
    int counter = 0;
    int nClasses = ui->sbNumClasses->value();

    for(int i = 0; i < nClasses; i++)
    {
        QString label = inputDialog.getText(NULL ,"Choose label", QString("Label %1 : ").arg(i),
                                            QLineEdit::Normal, QString("L%1").arg(i+1), &ok);
        if(ok && !label.isEmpty())
        {
            counter++;
            classes += label + ";";
        }
    }
    classes.truncate(classes.size()-1);

    if(counter == nClasses)
    {
        _classes = classes;
        _p.classesCount = nClasses;
        setLabels(_classes);
    }
}

void MainWindow::setLabels(QString classes)
{
    QStringListModel* lm = new QStringListModel();
    lm->setStringList(classes.split(";"));
    ui->lstClasses->setModel(lm);

    int nClasses = classes.split(";").size();
    ui->tblClasses->setColumnCount((nClasses-1)/15 + 1);
    ui->tblClasses->setRowCount((nClasses > 15) ? 15 : nClasses);

    int i = 0;
    for(QString s : classes.split(";"))
    {
        ui->tblClasses->setItem(i%15, i/15, new QTableWidgetItem(s));
        i++;
    }
}

void MainWindow::receiveSelectOutputFilename()
{
    _outputFilename = QFileDialog::getSaveFileName(this, tr("Select where to store the output"),
                                                    QDir::currentPath());
    if(!_outputFilename.isEmpty())
        ui->lblOutputFilename->setText(_outputFilename);
}

void MainWindow::receiveOutput(QImage frame)
{
    ui->lblViewOutput->setPixmap(QPixmap::fromImage(frame));
    ui->lblViewOutput->adjustSize();
}

void MainWindow::receiveError(QString error)
{
    QMessageBox mb;
    mb.warning(this, "Error", error);
}

void MainWindow::receiveReset()
{
    setupParams(default_parameters);
}


void MainWindow::receiveExportParameters()
{
    QString filename = QFileDialog::getSaveFileName(this, tr("Export as"),
                                                     QDir::currentPath() + "/params.cfg",
                                                     tr("Config file (*.cfg)"));
    if(!filename.isEmpty())
    {
        QFile fout(filename);
        if(fout.open(QIODevice::WriteOnly))
        {
            QTextStream out(&fout);
            out.setCodec("UTF-8");
            saveParameters(_p);
            _p.serialize(out);
            fout.close();
        }
        else
        {
            receiveError("Error opening the file");
        }
    }
}

void MainWindow::receiveImportParameters()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Import from"),
                                                     QDir::currentPath(),
                                                     tr("Config file (*.cfg)"));
    QFile fin(filename);
    if(fin.open(QIODevice::ReadOnly))
    {
        QTextStream in(&fin);
        in.setCodec("UTF-8");
        _p.deserialize(in);
        setupParams(_p);
        fin.close();
    }
    else
    {
        receiveError("Error opening the file");
    }
}

bool MainWindow::saveParameters(Params& p)
{
    if(_activeFile.isEmpty())
        receiveError("Please choose a file or a folder to process.");
    else if(ui->txtOutputFolder->text().isEmpty())
        receiveError("Please specify your output folder");
    else if(ui->lblOutputFilename->text().isEmpty())
        receiveError("Please specify your output filename");
    else
    {
        p.outputFileName = ui->lblOutputFilename->text();
        p.outputFolderName = ui->txtOutputFolder->text();

        p.window_x = ui->sbX->value();
        p.window_y = ui->sbY->value();
        p.window_w = ui->sbW->value();
        p.window_h = ui->sbH->value();
        p.dy = ui->sbDx->value();
        p.dx = ui->sbDy->value();

        p.output_w = ui->sbOutputW->value();
        p.output_h = ui->sbOutputH->value();
        p.output_interpolation = ui->cmbInterpolation->currentIndex();
        p.output_resized = ui->cbResizeOutput->isChecked();

        p.classesCount = ui->sbNumClasses->value();
        p.classesLabels = _classes;
        p.outputClassFormat = ui->cmbOutputClass->currentIndex();
        p.outputFilenameFormat =ui->cmbOutputFormat->currentIndex();

        return true;
    }
    return false;
}

void MainWindow::receiveStart()
{
    if(saveParameters(_p))
    {
        ui->btnSave->setEnabled(true);
        ui->btnSkip->setEnabled(true);
        ui->tblClasses->setEnabled(true);
        ui->tblClasses->setCurrentCell(0, 0);
        _curIter = ui->sbCurCase->value();
        _curClass = 0;
        _ok = true;
        if(_outputFile.isOpen())
            _outputFile.close();
        _outputFile.setFileName(_p.outputFileName);
        _outputFile.open(QIODevice::WriteOnly | QIODevice::Append);

        emit sendFilename(_activeFile);
        emit sendUpdateWindow(_p.window_x, _p.window_y, _p.window_w, _p.window_h,
                              _p.output_resized*_p.output_w, _p.output_resized*_p.output_h, _p.output_interpolation);
    }
}



void MainWindow::receiveNextImage()
{
    if(!_inputDir.isEmpty())
    {
        _fileIndex++;
        for(; _fileIndex < _files.size() && _files[_fileIndex].isDir(); _fileIndex++);
        if(_fileIndex >= _files.size())
        {
            receiveError("No more files in this folder.");
        }
        else
        {
            _activeFile = _files[_fileIndex].filePath();
            ui->cbOriginalSize->setChecked(true);
            emit sendFilename(_activeFile);
        }
    }
}

void MainWindow::receiveSaveAction()
{
    int fnf = ui->cmbOutputFormat->currentIndex(); // file name format

    QString tmp, label = ui->tblClasses->item(_curClass%15, _curClass/15)->text();

    switch(fnf){
        case 0: tmp = ""; break;
        case 1: tmp = QString("%1_").arg(_curClass); break;
        case 2: tmp = QString("%1_").arg(label); break;
    }

    QString filename = QString("%1\\%2%3.png").arg(_p.outputFolderName).arg(tmp).arg(_curIter++);
    emit sendSaveWindow(filename);

    QTextStream s(&_outputFile);
    if(ui->cmbOutputClass->currentIndex() == 0)
        s << _curClass << "\r\n";
    else
        s << label << "\r\n";

    ui->sbCurCase->setValue(_curIter);
    receiveSkipAction();
}

void MainWindow::receiveSkipAction()
{
    int w = ui->sbW->value(), h = ui->sbH->value();
    int w2 = ui->sbOutputW->value() * ui->cbResizeOutput->isChecked();
    int h2 = ui->sbOutputH->value() * ui->cbResizeOutput->isChecked();
    int i = ui->cmbInterpolation->currentIndex();
    int x, y;

    if(slide(x, y, w, h))
        emit sendUpdateWindow(x, y, w, h, w2, h2, i);
}

bool MainWindow::slide(int& x, int& y, const int w1, const int h1)
{
    int dx = ui->sbDx->value();
    int dy = ui->sbDy->value();
    int curX = ui->sbX->value();
    int curY = ui->sbY->value();
    int W = ui->lblView->pixmap()->width();
    int H = ui->lblView->pixmap()->height();

    if(curX == W - w1)
    {
        if(curY < H - h1 - 1 && curY + h1 + dy > H)
            curY = H - h1 - 1;
        else
            curY += dy;
        curX = 0;
    }
    else
    {
        if(curX + w1 + dx > W)
            curX = W - w1;
        else
            curX += dx;
    }

    if(curY <= H - h1)
    {
        x = curX;
        y = curY;
        ui->sbX->setValue(x);
        ui->sbY->setValue(y);
        return true;
    }
    return false;
}

void MainWindow::receiveClassCellClicked(int row,int col)
{
    int c = col*15 + row;
    if(c < _p.classesCount)
        _curClass = c;
    else
        receiveError(QString("Empty cell %1 %2 %3").arg(row).arg(col).arg(_p.classesCount));
}


MainWindow::~MainWindow()
{
    delete ui;
    delete worker;
}
