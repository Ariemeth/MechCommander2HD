#ifndef ASE_HELPEROBJECT
#define ASE_HELPEROBJECT

#include "ase_string.h"
#include "ase_node_tm.h"
#include "tm_animation.h"

class Ase_HelperObject : public AseGenericItem
{
public:
    Ase_String Name;
    Ase_String HelperClass;
    Ase_String NodeParent; //optional.
    Ase_Node_TM NodeTM;
    Ase_ThreeFloat BoundingBoxMin;
    Ase_ThreeFloat BoundingBoxMax;
    Tm_Animation TmAnimation;
};

#endif // ASE_HELPEROBJECT

