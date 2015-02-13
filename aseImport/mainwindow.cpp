#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "Include\AseModel\asemodel.hpp"
#include <QMessageBox>
#include <QStandardItem>

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
    QStringList * localContents = new QStringList();
    QString openFile;
    openFile = QFileDialog::getOpenFileName(this,QCoreApplication::applicationName(),QString(),tr("ASE files (*.ASE)"));
    if (openFile.isEmpty())
    {
        return;
    }
    QFile inputFile(openFile);
    if (inputFile.open(QIODevice::ReadWrite))
    {
        QTextStream in(&inputFile);

        while ( !in.atEnd() )
        {
            QString * curLine = new QString(in.readLine());
            QString * simple = new QString(curLine->simplified());
            localContents->append(*simple);
        }
        inputFile.close();
    }
    targetContents = localContents;
    Ase_Model newModel;
    newModel.header.setString("thisName");
}
