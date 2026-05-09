#ifndef MEMORYMANAGER_H
#define MEMORYMANAGER_H

#include "MemoryModels.h"
#include <QString>
#include <QVector>
#include <algorithm>

class MemoryManager {
public:
    MemoryManager(int totalSize);
    void addInitialHole(int base, int size);
    bool allocateProcess(Process& process, bool isBestFit);
    bool deallocateProcess(QString name);
    QVector<Hole> getHoles() const { return freeHoles; }
    QVector<Process>& getAllocatedProcesses() { return allocatedProcesses; }
    bool allocateAdditionalSegment(Process& process, Segment& seg, bool isBestFit);
    void rollbackProcess(QString name);
private:
    int totalMemorySize;
    QVector<Hole> freeHoles;
    QVector<Process> allocatedProcesses;
    void coalesceHoles();
};

#endif // MEMORYMANAGER_H