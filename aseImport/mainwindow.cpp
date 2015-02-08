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
           lineitem->invisibleRootItem();
           // For every line in the file, we need to scan for a { and if present
           // create a block with a title, and everything under it grouped together.
           // Close the block when a } is found.

           // Each line that does not have a { or } should be all in one row.

           QStandardItem * topItems = new QStandardItem();

         while ( !in.atEnd() )
           {
              QString curLine = in.readLine();
              curLine = curLine.simplified();
              QStringList lineSplit = curLine.split(" ");
//              QList<QStandardItem*> subList;
              QStandardItem * itemRow = new QStandardItem();
              itemRow->setData(lineSplit[0],Qt::DisplayRole);
              QList<QStandardItem*> dummy;
              for (int i=1; i < lineSplit.count(); i++)
              {
                  QStandardItem * add = new QStandardItem(lineSplit[i]);
                  dummy.append(add);
              }
              itemRow->appendColumn(dummy);
              topItems->appendRow(itemRow);
           }
           inputFile.close();
           lineitem->appendRow(topItems);
           ui->targetView->setModel(lineitem);
        }

}
