#ifndef ASE_MESH_NORMAL
#define ASE_MESH_NORMAL
#include "ase_mesh_vertex.hpp"
#include "aseitem.hpp"

class Ase_MeshNormal : AseGenericItem
{
    Ase_Mesh_Vertex Mesh_FaceNormal;
    Ase_Mesh_Vertex Mesh_VertexNormal[3];
};

#endif // ASE_MESH_NORMAL

