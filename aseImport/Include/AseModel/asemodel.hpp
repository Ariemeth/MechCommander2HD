#ifndef ASEMODEL
#define ASEMODEL
#include "ase_string.hpp"
#include "ase_material_parent.hpp"
#include "ase_helperobject.hpp"
#include "ase_geomobject.hpp"
#include <QStringList>
#include <QFile>
#include <QTextStream>

class Ase_Model : public AseGenericItem
{
public:
    Ase_Model::Ase_Model(QObject * parent = 0);
    Ase_Model::~Ase_Model();
    Ase_Model::Ase_Model(QObject*,QString);

    Ase_String header;
    Ase_String Comment;
    Ase_Material_Parent materialList;
    QList<Ase_HelperObject*> HelperObjects;
    QList<Ase_GeomObject*> GeomObjects;

private:
    void parseLines(QStringList);
};

#endif // ASEMODEL

