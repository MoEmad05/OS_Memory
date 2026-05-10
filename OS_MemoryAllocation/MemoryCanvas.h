#ifndef MEMORYCANVAS_H
#define MEMORYCANVAS_H

#include <QWidget>
#include <QPainter>
#include "MemoryModels.h"

class MemoryCanvas : public QWidget {
    Q_OBJECT

public:
    explicit MemoryCanvas(QWidget *parent = nullptr);

    void updateMemory(int totalSize, const QVector<Hole>& holes, const QVector<Process>& processes);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    int totalMemorySize;
    QVector<Hole> currentHoles;
    QVector<Process> currentProcesses;
};

#endif