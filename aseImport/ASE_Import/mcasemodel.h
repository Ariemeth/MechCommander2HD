#ifndef MCASEMODEL
#define MCASEMODEL

#include <QStandardItemModel>
#include <QFile>

class McAseModel : public QStandardItemModel
{
    Q_OBJECT
public:
    McAseModel(QObject* parent =0);
    McAseModel(QObject *, QString);
    virtual ~McAseModel();
};

#endif // MCASEMODEL

