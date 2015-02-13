#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QAbstractItemModel>
#include <QFile>
#include <QTextStream>
//#include "Include/AseModel/asemodel.hpp"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    QStringList * targetContents;
    QStringList * importContents;


private slots:
    void on_OpenTarget_clicked();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
