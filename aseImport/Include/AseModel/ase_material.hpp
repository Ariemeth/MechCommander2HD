#ifndef ASE_MATERIAL
#define ASE_MATERIAL

#include "ase_string.hpp"
#include "ase_threefloat.hpp"
#include "ase_mapdiffuse.hpp"
#include "aseitem.hpp"

class Ase_Material : public AseGenericItem
{
public:
    Ase_String Name;
    Ase_String Class;
    Ase_ThreeFloat Ambient;
    Ase_ThreeFloat Diffuse;
    Ase_ThreeFloat Specular;
    Ase_SingleFloat Shine;
    Ase_SingleFloat ShineStrength;
    Ase_SingleFloat Transparency;
    Ase_SingleFloat wireSize;
    Ase_String shading;
    Ase_SingleFloat XpFalloff;
    Ase_SingleFloat SelfIllum;
    Ase_String falloff;
    Ase_String XP_Type;
    ase_mapDiffuse mapDiffuse;
};

#endif // ASE_MATERIAL

