#include "MemoryManager.h"

MemoryManager::MemoryManager(int totalSize) : totalMemorySize(totalSize) {}

void MemoryManager::addInitialHole(int base, int size) {
    freeHoles.push_back({base, size});
    coalesceHoles();
}

bool MemoryManager::allocateProcess(Process& process, bool isBestFit) {
    QVector<Hole> backupHoles = freeHoles;

    for (int i = 0; i < process.segments.size(); ++i) {
        Segment& seg = process.segments[i];
        int targetHoleIdx = -1;

        if (isBestFit) {
            int minLeftover = 1e9;
            for (int j = 0; j < freeHoles.size(); ++j) {
                if (freeHoles[j].size >= seg.size) {
                    int leftover = freeHoles[j].size - seg.size;
                    if (leftover < minLeftover) {
                        minLeftover = leftover;
                        targetHoleIdx = j;
                    }
                }
            }
        } else {
            for (int j = 0; j < freeHoles.size(); ++j) {
                if (freeHoles[j].size >= seg.size) {
                    targetHoleIdx = j;
                    break;
                }
            }
        }

        if (targetHoleIdx == -1) {
            freeHoles = backupHoles;
            return false;
        }

        seg.base = freeHoles[targetHoleIdx].base;

        freeHoles[targetHoleIdx].base += seg.size;
        freeHoles[targetHoleIdx].size -= seg.size;

        if (freeHoles[targetHoleIdx].size == 0) {
            freeHoles.removeAt(targetHoleIdx);
        }
    }

    allocatedProcesses.push_back(process);
    return true;
}
bool MemoryManager::allocateAdditionalSegment(Process& process,
                        Segment& seg, bool isBestFit) {
    int targetHoleIdx = -1;
    if (isBestFit) {
        int minLeftover = 2147483647;
        for (int j = 0; j < freeHoles.size(); ++j) {
            if (freeHoles[j].size >= seg.size) {
                int leftover = freeHoles[j].size - seg.size;
                if (leftover < minLeftover) {
                    minLeftover = leftover;
                    targetHoleIdx = j;
                }
            }
        }
    } else {
        for (int j = 0; j < freeHoles.size(); ++j) {
            if (freeHoles[j].size >= seg.size) {
                targetHoleIdx = j;
                break;
            }
        }
    }
    if (targetHoleIdx == -1) return false;
    seg.base = freeHoles[targetHoleIdx].base;
    freeHoles[targetHoleIdx].base += seg.size;
    freeHoles[targetHoleIdx].size -= seg.size;
    if (freeHoles[targetHoleIdx].size == 0) {
        freeHoles.removeAt(targetHoleIdx);
    }
    process.segments.append(seg);
    return true;
}

void MemoryManager::rollbackProcess(QString name) {

    for (int i = 0; i < allocatedProcesses.size(); ++i) {
        if (allocatedProcesses[i].name == name) {
            for (const Segment& seg : allocatedProcesses[i].segments) {
                freeHoles.push_back({seg.base, seg.size});
            }
            allocatedProcesses.removeAt(i);
            coalesceHoles();
            return;
        }
    }
}

bool MemoryManager::deallocateProcess(QString name) {
    for (int i = 0; i < allocatedProcesses.size(); ++i) {
        if (allocatedProcesses[i].name.trimmed() == name.trimmed()) {
            for (const Segment& seg : allocatedProcesses[i].segments) {
                freeHoles.push_back({seg.base, seg.size});
            }
            allocatedProcesses.removeAt(i);
            coalesceHoles();
            return true;
        }
    }
    return false;
}

void MemoryManager::coalesceHoles() {
    if (freeHoles.isEmpty()) return;

    std::sort(freeHoles.begin(), freeHoles.end(), [](const Hole& a, const Hole& b) {
        return a.base < b.base;
    });
    for (int i = 0; i < freeHoles.size() - 1; ) {
        if (freeHoles[i].base + freeHoles[i].size >= freeHoles[i+1].base) {
            int currentEnd = freeHoles[i].base + freeHoles[i].size;
            int nextEnd = freeHoles[i+1].base + freeHoles[i+1].size;
            freeHoles[i].size = std::max(currentEnd, nextEnd) - freeHoles[i].base;
            freeHoles.removeAt(i + 1);
        } else {
            i++;
        }
    }
}