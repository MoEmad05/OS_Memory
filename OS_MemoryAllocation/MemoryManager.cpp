#include "MemoryManager.h"

MemoryManager::MemoryManager(int totalSize) : totalMemorySize(totalSize) {}

void MemoryManager::addInitialHole(int base, int size) {
    freeHoles.push_back({base, size});
    coalesceHoles(); // Ensure they merge if they touch
}

bool MemoryManager::allocateProcess(Process& process, bool isBestFit) {
    // 1. Create a backup of holes for the "rollback" scenario
    QVector<Hole> backupHoles = freeHoles;

    // 2. Try to allocate every segment
    for (int i = 0; i < process.segments.size(); ++i) {
        Segment& seg = process.segments[i];
        int targetHoleIdx = -1;

        if (isBestFit) {
            int minLeftover = 1e9; // Infinity
            for (int j = 0; j < freeHoles.size(); ++j) {
                if (freeHoles[j].size >= seg.size) {
                    int leftover = freeHoles[j].size - seg.size;
                    if (leftover < minLeftover) {
                        minLeftover = leftover;
                        targetHoleIdx = j;
                    }
                }
            }
        } else { // First-Fit
            for (int j = 0; j < freeHoles.size(); ++j) {
                if (freeHoles[j].size >= seg.size) {
                    targetHoleIdx = j;
                    break;
                }
            }
        }

        // 3. If no hole was found for this segment, ROLLBACK
        if (targetHoleIdx == -1) {
            freeHoles = backupHoles; // Restore original state
            return false; // Allocation failed
        }

        // 4. Allocate segment
        seg.base = freeHoles[targetHoleIdx].base;

        // Update the hole
        freeHoles[targetHoleIdx].base += seg.size;
        freeHoles[targetHoleIdx].size -= seg.size;

        // If hole is fully consumed, remove it
        if (freeHoles[targetHoleIdx].size == 0) {
            freeHoles.removeAt(targetHoleIdx);
        }
    }

    // If we made it here, all segments fit!
    allocatedProcesses.push_back(process);
    return true;
}
bool MemoryManager::allocateAdditionalSegment(Process& process, Segment& seg, bool isBestFit) {
    int targetHoleIdx = -1;

    // Use the SAME logic as your standard allocation to find a hole
    if (isBestFit) {
        int minLeftover = 2147483647; // Max Int
        for (int j = 0; j < freeHoles.size(); ++j) {
            if (freeHoles[j].size >= seg.size) {
                int leftover = freeHoles[j].size - seg.size;
                if (leftover < minLeftover) {
                    minLeftover = leftover;
                    targetHoleIdx = j;
                }
            }
        }
    } else { // First-Fit
        for (int j = 0; j < freeHoles.size(); ++j) {
            if (freeHoles[j].size >= seg.size) {
                targetHoleIdx = j;
                break;
            }
        }
    }

    if (targetHoleIdx == -1) return false; // Segment won't fit

    // Update segment base
    seg.base = freeHoles[targetHoleIdx].base;

    // Shrink the hole
    freeHoles[targetHoleIdx].base += seg.size;
    freeHoles[targetHoleIdx].size -= seg.size;

    if (freeHoles[targetHoleIdx].size == 0) {
        freeHoles.removeAt(targetHoleIdx);
    }

    // CRITICAL: Append the segment to the EXISTING process list
    process.segments.append(seg);
    return true;
}

void MemoryManager::rollbackProcess(QString name) {
    // This is identical to deallocate, but specifically used
    // when a segment fails to fit during the allocation phase.
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