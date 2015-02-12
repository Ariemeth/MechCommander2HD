#ifndef ASE_MESH_VERTEX
#define ASE_MESH_VERTEX

#include "ase_singleint.hpp"
#include "ase_threefloat.hpp"

class Ase_Mesh_Vertex : AseGenericItem
{
public:
    Ase_SingleInt meshNo;
    Ase_ThreeFloat meshPoints;
};

#endif // ASE_MESH_VERTEX

