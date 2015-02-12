#ifndef ASEITEM
#define ASEITEM
#include <QObject>

class AseGenericItem : public QObject
{
    Q_OBJECT
public:
    QString getType();
    void setType(QString);
private:
    QString _typeName;
};

#endif // ASEITEM

