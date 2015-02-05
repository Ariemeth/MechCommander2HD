#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_OpenTarget_clicked()
{
    QString openFile;
    openFile = QFileDialog::getOpenFileName(this,QCoreApplication::applicationName(),QString(),tr("ASE files (*.ASE)"));
    if (openFile.isEmpty())
    {
        return;
    }
}
