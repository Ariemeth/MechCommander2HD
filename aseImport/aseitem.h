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

class Ase_ThreeFloat : AseGenericItem
{

public:
    Ase_SingleFloat X;
    Ase_SingleFloat Y;
    Ase_SingleFloat Z;
};

class Ase_SingleInt : AseGenericItem
{

public:
    int getI();
    void setI(int);
private:
    int _X;
};

class Ase_ThreeInt : AseGenericItem
{
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

    bool hasQuotation();
    void setQuotation(bool);
private:
    QString _String;
    bool _Quotation;
};

class Ase_Mesh_Vertex : AseGenericItem
{
public:
    Ase_SingleInt meshNo;
    Ase_ThreeFloat meshPoints;
};

class Ase_Mesh_Face : AseGenericItem
{
public:
    Ase_SingleInt meshNum;
    Ase_ThreeInt ABC_face;
    Ase_ThreeInt AB_BC_CA_face;
    Ase_SingleInt mesh_smoothing;
    Ase_SingleInt mesh_mtlid;
};

class Ase_Mesh_Tface : AseGenericItem
{
    Ase_SingleInt faceNum;
    Ase_ThreeInt faceList;
};

class Ase_MeshNormal : AseGenericItem
{
    Ase_Mesh_Vertex Mesh_FaceNormal;
    Ase_Mesh_Vertex Mesh_VertexNormal[3];
};

#endif // ASEITEM

