#ifndef ASE_CONTROL_ROT
#define ASE_CONTROL_ROT
#include "ase_mesh_vertex.hpp"
#include "aseitem.hpp"

class Ase_Control_Rot : public AseGenericItem
{
public:
    QList<Ase_Mesh_Vertex*> ControlRotSample;
};

#endif // ASE_CONTROL_ROT

