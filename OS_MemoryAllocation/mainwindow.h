#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
#include "MemoryManager.h" // Include your logic header

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // These names should match your button names in the UI Designer
    void on_btnSetTotalSize_clicked();
    void on_btnAddHole_clicked();
    void on_btnAllocate_clicked();
    void on_btnDeallocate_clicked();
private:
    Ui::MainWindow *ui;
    MemoryManager *memManager; // The logic engine
    int totalSize = 0;
};
#endif