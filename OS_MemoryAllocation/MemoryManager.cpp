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
bool MemoryManager::allocateAdditionalSegment(Process& process, Segment& seg, bool isBestFit) {
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
        // Compare names (trimmed to avoid issues with accidental spaces)
        if (allocatedProcesses[i].name.trimmed() == name.trimmed()) {

            // Loop through ALL segments belonging to this specific process
            for (const Segment& seg : allocatedProcesses[i].segments) {
                // Add the space back to the free holes list
                freeHoles.push_back({seg.base, seg.size});
            }

            // Remove the entire process object (and all its segments) from the list
            allocatedProcesses.removeAt(i);

            // Merge adjacent holes to prevent fragmentation
            coalesceHoles();

            return true; // Successfully found and removed all segments
        }
    }
    return false; // Process name not found
}

// Standardize on ONE merging function
void MemoryManager::coalesceHoles() {
    if (freeHoles.isEmpty()) return;

    // 1. Sort by address so neighbors are next to each other in the vector
    std::sort(freeHoles.begin(), freeHoles.end(), [](const Hole& a, const Hole& b) {
        return a.base < b.base;
    });

    // 2. Linear merge
    for (int i = 0; i < freeHoles.size() - 1; ) {
        // If current hole end >= next hole start, they are touching or overlapping
        if (freeHoles[i].base + freeHoles[i].size >= freeHoles[i+1].base) {
            int currentEnd = freeHoles[i].base + freeHoles[i].size;
            int nextEnd = freeHoles[i+1].base + freeHoles[i+1].size;

            // New size is the furthest reach of either hole
            freeHoles[i].size = std::max(currentEnd, nextEnd) - freeHoles[i].base;

            // Remove the merged neighbor
            freeHoles.removeAt(i + 1);
            // Stay at index 'i' to see if the new bigger hole touches the next one
        } else {
            i++;
        }
    }
}