#ifndef ASE_MESH_TFACE
#define ASE_MESH_TFACE

#include "ase_threeint.h"

class Ase_Mesh_Tface : AseGenericItem
{
    Ase_SingleInt faceNum;
    Ase_ThreeInt faceList;
};

#endif // ASE_MESH_TFACE

