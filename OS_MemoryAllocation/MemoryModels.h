#ifndef MEMORYMODELS_H
#define MEMORYMODELS_H

#include <QString>
#include <QVector>

struct Hole {
    int base;
    int size;
};

struct Segment {
    QString name;
    int size;
    int base = -1;
};

struct Process {
    QString name;
    QVector<Segment> segments;
};

#endif