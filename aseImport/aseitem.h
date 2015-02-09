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

class Ase_SingleFloat : QObject // Useful for TM_ROTATE_ANGLE
{
    Q_OBJECT
public:
    void setF(double);
    double getF();

private:
    double _X;
};

class Ase_ThreeFloat : QObject
{
    Q_OBJECT
public:
    Ase_SingleFloat X;
    Ase_SingleFloat Y;
    Ase_SingleFloat Z;
};

class Ase_SingleInt : QObject
{
    Q_OBJECT
public:
    int getI();
    void setI(int);
private:
    int _X;
};

class Ase_ThreeInt : QObject
{
    Q_OBJECT
public:
    Ase_SingleInt X;
    Ase_SingleInt Y;
    Ase_SingleInt Z;
};

class Ase_String : AseGenericItem // useful for things like NODE_NAME and NODE_PARENT
{
public:
    QString getString();
    void setString(QString);
private:
    QString _String;
};

class Ase_Mesh_Vertex : AseGenericItem
{
public:
    Ase_Mesh_Vertex::Ase_Mesh_Vertex(QObject *parent = 0);
    Ase_SingleInt meshNo;
    Ase_ThreeFloat meshPoints;
};

#endif // ASEITEM

