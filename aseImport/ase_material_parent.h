#ifndef ASE_MATERIAL_PARENT
#define ASE_MATERIAL_PARENT
#include "ase_material.h"

class Ase_Material_Parent : public AseGenericItem
{
public:
    int materialCount();
    QList<Ase_Material*> materialList;
};

#endif // ASE_MATERIAL_PARENT

