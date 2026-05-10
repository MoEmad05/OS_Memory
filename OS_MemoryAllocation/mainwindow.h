#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
#include "MemoryManager.h"

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
    void on_btnSetTotalSize_clicked();
    void on_btnAddHole_clicked();
    void on_btnAllocate_clicked();
    void on_btnDeallocate_clicked();
private:
    Ui::MainWindow *ui;
    MemoryManager *memManager;
    int totalSize = 0;
};
#endif