#ifndef TM_ANIMATION
#define TM_ANIMATION

#include "ase_string.hpp"
#include "ase_control_rot.hpp"
#include "aseitem.hpp"

class Tm_Animation : public AseGenericItem
{
public:
    Ase_String NodeName;
    Ase_Control_Rot TmAnimation;
};

#endif // TM_ANIMATION

