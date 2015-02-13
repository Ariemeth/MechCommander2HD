#ifndef ASEMODEL
#define ASEMODEL
#include "ase_string.hpp"
#include "ase_material_parent.hpp"
#include "ase_helperobject.hpp"
#include "ase_geomobject.hpp"

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

