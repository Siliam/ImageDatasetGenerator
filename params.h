/*
 *  File    : Params.h
 *  Desc    : A class to handle the settings of the app, esp. importing from/exporting to a file
 *  Author  : Ismail
 */

#ifndef PARAMS_H
#define PARAMS_H

#include <QTextStream>

class Params {
public:
    QString outputFileName;
    QString outputFolderName;
    int window_x, window_y, window_w, window_h;
    int dx, dy;
    int output_w, output_h, output_interpolation;
    int output_resized;
    int classesCount;
    QString classesLabels;
    int outputClassFormat, outputFilenameFormat;

    void serialize(QTextStream& out)
    {
        out << "OutputFilename: " << outputFileName << endl;
        out << "OutputFolder: " << outputFolderName << endl;
        out << "WindowParameters: " << window_x << " " << window_y << " " << window_w << " " << window_h << endl;
        out << "StrideParameters: " << dx << " " << dy << endl;
        out << "OutputParameters: " << output_w << " " << output_h << " " << output_interpolation << " " << output_resized << endl;
        out << "Classes: " << classesCount << " " << classesLabels << endl;
        out << "Formats: " << outputClassFormat << " " << outputFilenameFormat;
    }

    void deserialize(QTextStream& in)
    {
        QString s;
        in >> s >> outputFileName;
        in >> s >> outputFolderName;
        in >> s >> window_x >> window_y >> window_w >> window_h;
        in >> s >> dx >> dy;
        in >> s >> output_w >> output_h >> output_interpolation >> output_resized;
        in >> s >> classesCount >> classesLabels;
        in >> s >> outputClassFormat >> outputFilenameFormat;
    }
};


#endif // PARAMS_H
