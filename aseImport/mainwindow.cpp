#include "mainwindow.h"
#include "ui_mainwindow.h"
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
           QStandardItemModel* lineitem = new QStandardItemModel(this);
           while ( !in.atEnd() )
           {
              QString line = in.readLine();
              line.remove("\"");
              line = line.simplified();
              QStringList split = line.split(" ");
              QList<QStandardItem*> list;
              foreach (QString item, split)
              {
                  QStandardItem * subItem = new QStandardItem(item);
                  list << subItem;
              }
              lineitem->appendRow(list);

           }
           inputFile.close();

           ui->targetView->setModel(lineitem);
        }

}
