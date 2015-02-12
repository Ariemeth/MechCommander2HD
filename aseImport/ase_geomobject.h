#ifndef ASE_GEOMOBJECT
#define ASE_GEOMOBJECT

#include "ase_string.h"
#include "ase_node_tm.h"
#include "ase_mesh.h"

class Ase_GeomObject : public AseGenericItem
{
public:
    Ase_String Name;
    Ase_String Parent;
    Ase_Node_TM Node;
    Ase_Mesh Mesh;
    Ase_SingleInt PropMotionBlur;
    Ase_SingleInt PropCastShadow;
    Ase_SingleInt PropRecvShadow;
    Ase_SingleInt materialRef;
};

#endif // ASE_GEOMOBJECT

