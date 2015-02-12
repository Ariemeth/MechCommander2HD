#ifndef ASE_SCENE
#define ASE_SCENE
#include "ase_string.h"
#include "ase_singleint.hpp"
#include "ase_threefloat.hpp"

class ase_Scene : public AseGenericItem
{
public:
    Ase_String fileName;
    Ase_SingleInt firstFrame;
    Ase_SingleInt lastFrame;
    Ase_SingleInt frameSpeed;
    Ase_SingleInt ticksPerFrame;
    Ase_ThreeFloat backgroundStatic;
    Ase_ThreeFloat ambientStatic;
};

#endif // ASE_SCENE

