#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "MemoryCanvas.h"
#include <QIntValidator>
#include <QMessageBox> // Added this
#include <QTableWidgetItem>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    memManager = nullptr;

    ui->inputTotalSize->setRange(0, 1000000);
    ui->inputHoleAddr->setRange(0, 1000000);
    ui->inputHoleSize->setRange(0, 1000000);
    ui->inputSegSize->setRange(0, 1000000);
}

MainWindow::~MainWindow() {
    delete ui;
    if(memManager) delete memManager;
}

void MainWindow::on_btnSetTotalSize_clicked() {
    totalSize = ui->inputTotalSize->value();
    if(memManager) delete memManager;
    memManager = new MemoryManager(totalSize);
    ui->memoryCanvasWidget->updateMemory(totalSize, memManager->getHoles(), memManager->getAllocatedProcesses());
}

void MainWindow::on_btnAddHole_clicked() {
    if(!memManager) {
        QMessageBox::warning(this, "Error", "Please initialize memory size first!");
        return;
    }

    int addr = ui->inputHoleAddr->value();
    int size = ui->inputHoleSize->value();

    // VALIDATION: Check if hole exceeds total memory limits
    if (addr + size > totalSize) {
        QMessageBox::critical(this, "Boundary Error",
                              QString("The hole (End: %1) exceeds the total memory size (%2)!").arg(addr + size).arg(totalSize));
        return;
    }

    // If valid, add the hole and update the canvas
    memManager->addInitialHole(addr, size);
    ui->memoryCanvasWidget->updateMemory(totalSize, memManager->getHoles(), memManager->getAllocatedProcesses());
}

// STAGING: Adds a segment to the table, but DOES NOT allocate yet

void MainWindow::on_btnAllocate_clicked() {
    if(!memManager) return;

    QString pName = ui->inputProcName->text();
    QString sName = ui->inputSegName->text();
    int sSize = ui->inputSegSize->value();

    if(pName.isEmpty() || sName.isEmpty() || sSize <= 0) return;

    QVector<Process>& processes = memManager->getAllocatedProcesses();
    Process *existingProc = nullptr;

    for (int i = 0; i < processes.size(); ++i) {
        if (processes[i].name == pName) {
            existingProc = &processes[i];
            break;
        }
    }

    Segment s;
    s.name = sName;
    s.size = sSize;
    bool isBestFit = (ui->algorithmCombo->currentText() == "Best Fit");

    if (existingProc) {
        if (memManager->allocateAdditionalSegment(*existingProc, s, isBestFit)) {
            ui->memoryCanvasWidget->updateMemory(totalSize, memManager->getHoles(), processes);
        } else {
            // FAILURE DETECTED: Delete the entire process because this segment didn't fit
            memManager->rollbackProcess(pName);
            ui->memoryCanvasWidget->updateMemory(totalSize, memManager->getHoles(), processes);
            QMessageBox::critical(this, "Allocation Failed",
                                  QString("Segment '%1' couldn't fit. Removing the entire process '%2' to maintain atomicity.")
                                      .arg(sName).arg(pName));
        }
    } else {
        Process newP;
        newP.name = pName;
        newP.segments.append(s);

        if (memManager->allocateProcess(newP, isBestFit)) {
            ui->memoryCanvasWidget->updateMemory(totalSize, memManager->getHoles(), processes);
        } else {
            QMessageBox::critical(this, "Error", "Process cannot fit!");
        }
    }
}

void MainWindow::on_btnDeallocate_clicked() {
    if(!memManager) return;

    QString procName = ui->inputDeallocateName->text();
    if(procName.isEmpty()) return;

    // Try to deallocate. The function will return true if it found and deleted the segments.
    bool success = memManager->deallocateProcess(procName);

    if (success) {
        // Update the canvas to show the newly freed holes
        ui->memoryCanvasWidget->updateMemory(totalSize, memManager->getHoles(), memManager->getAllocatedProcesses());
        ui->inputDeallocateName->clear();
    } else {
        // Show an error so it doesn't fail silently!
        QMessageBox::warning(this, "Deallocation Error", "Could not find an allocated process named: '" + procName + "'");
    }
}