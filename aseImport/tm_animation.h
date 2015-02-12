#ifndef TM_ANIMATION
#define TM_ANIMATION

#include "ase_string.h"
#include "ase_control_rot.h"

class Tm_Animation : public AseGenericItem
{
public:
    Ase_String NodeName;
    Ase_Control_Rot TmAnimation;
};

#endif // TM_ANIMATION

