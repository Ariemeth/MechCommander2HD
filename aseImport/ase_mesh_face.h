#ifndef ASE_MESH_FACE
#define ASE_MESH_FACE
#include "ase_threeint.h"

class Ase_Mesh_Face : AseGenericItem
{
public:
    Ase_SingleInt meshNum;
    Ase_ThreeInt ABC_face;
    Ase_ThreeInt AB_BC_CA_face;
    Ase_SingleInt mesh_smoothing;
    Ase_SingleInt mesh_mtlid;
};

#endif // ASE_MESH_FACE

