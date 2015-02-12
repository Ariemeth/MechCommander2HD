#ifndef ASE_NODE_TM
#define ASE_NODE_TM

#include "ase_string.h"
#include "ase_threeint.h"
#include "ase_threefloat.hpp"

class Ase_Node_TM : public AseGenericItem
{
public:
    Ase_String Name;
    Ase_ThreeInt InheritPos;
    Ase_ThreeInt InheritRot;
    Ase_ThreeInt InheritScl;
    Ase_ThreeFloat TM_Row0;
    Ase_ThreeFloat TM_Row1;
    Ase_ThreeFloat TM_Row2;
    Ase_ThreeFloat TM_Row3;
    Ase_ThreeFloat TM_Pos;
    Ase_ThreeFloat TM_RotAxis;
    Ase_SingleFloat rotAngle;
    Ase_ThreeFloat Scale;
    Ase_ThreeFloat ScaleAxis;
    Ase_SingleFloat ScaleAxisAng;
};

#endif // ASE_NODE_TM

