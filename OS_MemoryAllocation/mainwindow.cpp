#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "MemoryCanvas.h"
#include <QIntValidator>
#include <QMessageBox>
#include <QTableWidgetItem>
#include <QHeaderView>

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

    // Make table columns stretch nicely
    ui->tableSegments->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
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
    ui->tableSegments->setRowCount(0); // Reset table on new memory init
}

void MainWindow::on_btnAddHole_clicked() {
    if(!memManager) {
        QMessageBox::warning(this, "Error", "Please initialize memory size first!");
        return;
    }

    int addr = ui->inputHoleAddr->value();
    int size = ui->inputHoleSize->value();

    if (addr + size > totalSize) {
        QMessageBox::critical(this, "Boundary Error",
                              QString("The hole (End: %1) exceeds the total memory size (%2)!").arg(addr + size).arg(totalSize));
        return;
    }

    memManager->addInitialHole(addr, size);
    ui->memoryCanvasWidget->updateMemory(totalSize, memManager->getHoles(), memManager->getAllocatedProcesses());
}

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

    // Helper lambda to clean the table if a rollback happens
    auto removeProcessFromTable = [&](const QString& name) {
        for(int i = ui->tableSegments->rowCount() - 1; i >= 0; --i) {
            if(ui->tableSegments->item(i, 0)->text() == name) {
                ui->tableSegments->removeRow(i);
            }
        }
    };

    Segment s;
    s.name = sName;
    s.size = sSize;
    bool isBestFit = (ui->algorithmCombo->currentText() == "Best Fit");
    bool success = false;

    if (existingProc) {
        // Enforce the segment limit defined in the SpinBox (Optional)
        if (existingProc->segments.size() >= ui->inputMaxSegments->value()) {
            QMessageBox::warning(this, "Limit Reached", "You have already reached the maximum segments set for this process.");
            return;
        }

        if (memManager->allocateAdditionalSegment(*existingProc, s, isBestFit)) {
            success = true;
        } else {
            memManager->rollbackProcess(pName);
            removeProcessFromTable(pName); // Clear from table visually
            ui->memoryCanvasWidget->updateMemory(totalSize, memManager->getHoles(), processes);
            QMessageBox::critical(this, "Allocation Failed",
                                  QString("Segment '%1' couldn't fit. Removing the entire process '%2' to maintain atomicity.")
                                      .arg(sName).arg(pName));
            return;
        }
    } else {
        Process newP;
        newP.name = pName;
        newP.segments.append(s);

        if (memManager->allocateProcess(newP, isBestFit)) {
            success = true;
        } else {
            QMessageBox::critical(this, "Error", "Segment cannot fit!");
            return;
        }
    }

    if (success) {
        // Update Canvas
        ui->memoryCanvasWidget->updateMemory(totalSize, memManager->getHoles(), processes);

        // Save successfully allocated segment into the table
        int row = ui->tableSegments->rowCount();
        ui->tableSegments->insertRow(row);
        ui->tableSegments->setItem(row, 0, new QTableWidgetItem(pName));
        ui->tableSegments->setItem(row, 1, new QTableWidgetItem(sName));
        ui->tableSegments->setItem(row, 2, new QTableWidgetItem(QString::number(sSize) + " KB"));

        // Clear the segment inputs for a smoother user experience
        ui->inputSegName->clear();
        ui->inputSegSize->setValue(0);
        ui->inputSegName->setFocus();
    }
}

void MainWindow::on_btnDeallocate_clicked() {
    if(!memManager) return;

    QString procName = ui->inputDeallocateName->text();
    if(procName.isEmpty()) return;

    bool success = memManager->deallocateProcess(procName);

    if (success) {
        ui->memoryCanvasWidget->updateMemory(totalSize, memManager->getHoles(), memManager->getAllocatedProcesses());

        // Remove the deallocated process rows from the table
        for(int i = ui->tableSegments->rowCount() - 1; i >= 0; --i) {
            if(ui->tableSegments->item(i, 0)->text() == procName) {
                ui->tableSegments->removeRow(i);
            }
        }

        ui->inputDeallocateName->clear();
    } else {
        QMessageBox::warning(this, "Deallocation Error", "Could not find an allocated process named: '" + procName + "'");
    }
}