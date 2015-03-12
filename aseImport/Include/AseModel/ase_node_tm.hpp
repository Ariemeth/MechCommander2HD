#ifndef ASE_NODEM_TM
#define ASE_NODEM_TM

#include "ase_string.hpp"
#include "ase_threeint.hpp"
#include "ase_threefloat.hpp"
#include "aseitem.hpp"

class Ase_Node_TM : public AseGenericItem
{
public:
    void setData(QString);
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
    Ase_SingleFloat TM_RotAngle;
    Ase_ThreeFloat TM_Scale;
    Ase_ThreeFloat TM_ScaleAxis;
    Ase_SingleFloat TM_ScaleAxisAng;
};

#endif // ASE_NODEM_TM

