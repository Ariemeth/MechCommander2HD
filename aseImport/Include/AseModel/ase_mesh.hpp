#ifndef ASE_MESH
#define ASE_MESH
#include "ase_singleint.hpp"
#include "ase_mesh_vertex.hpp"
#include "ase_mesh_face.hpp"
#include "ase_mesh_tface.hpp"
#include "ase_mesh_normal.hpp"
#include "aseitem.hpp"

class Ase_Mesh : public AseGenericItem
{
public:
   Ase_SingleInt timeValue;

   int Mesh_NumVertex();
   QList<Ase_Mesh_Vertex*> MeshVertexList;

   int Mesh_NumFaces();
   QList<Ase_Mesh_Face*> MeshFaceList;

   int Mesh_NumTVertex();
   QList<Ase_Mesh_Vertex> MeshTVertList;

   int Mesh_NumTVFaces();
   QList<Ase_Mesh_Tface*> MeshTFaceList;

   int Mesh_NumCVertex();
   QList<Ase_Mesh_Vertex*> Mesh_CVertList;

   int Mesh_NumCVFaces();
   QList<Ase_Mesh_Tface*> Mesh_CFaceList;

   int numMeshNormals();
   QList<Ase_MeshNormal*>MeshNormals;
};

#endif // ASE_MESH

