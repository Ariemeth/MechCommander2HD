#ifndef ASEMODEL
#define ASEMODEL
#include "ase_string.h"
#include "ase_material_parent.h"
#include "ase_helperobject.h"
#include "ase_geomobject.h"

class Ase_Model : public AseGenericItem
{
public:
    Ase_String header;
    Ase_String Comment;
    Ase_Material_Parent materialList;
    QList<Ase_HelperObject*> HelperObjects;
    QList<Ase_GeomObject*> GeomObjects;
};

#endif // ASEMODEL

