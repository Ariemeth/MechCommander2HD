#ifndef ASE_MAPDIFFUSE
#define ASE_MAPDIFFUSE

#include "ase_string.hpp"
#include "ase_singlefloat.hpp"
#include "ase_singleint.hpp"
#include "aseitem.hpp"

class ase_mapDiffuse : public AseGenericItem
{
public:
    Ase_String Name;
    Ase_String Class;
    Ase_SingleInt subno;
    Ase_SingleFloat amount;
    Ase_String bitmap;
    Ase_String type;
    Ase_SingleFloat uvwUoffset;
    Ase_SingleFloat uvwVoffset;
    Ase_SingleFloat uvwUtiling;
    Ase_SingleFloat uvwVtiling;
    Ase_SingleFloat uvwAngle;
    Ase_SingleFloat uvwBlur;
    Ase_SingleFloat uvwBlurOffset;
    Ase_SingleFloat uvwNouseAmt;
    Ase_SingleFloat uvwNouseSize;
    Ase_SingleInt uvwNoiseLevel;
    Ase_SingleFloat uvwNoisePhase;
    Ase_String bitmapFilter;
};

#endif // ASE_MAPDIFFUSE

