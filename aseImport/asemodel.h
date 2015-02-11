#ifndef ASEMODEL
#define ASEMODEL
#include "aseitem.h"

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

class Ase_Material_Parent : public AseGenericItem
{
public:
    int materialCount();
    QList<Ase_Material*> materialList;
};

/////////////////////////////////////////////

class Ase_Node_TM : public AseGenericItem
{
public:
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
    Ase_SingleFloat rotAngle;
    Ase_ThreeFloat Scale;
    Ase_ThreeFloat ScaleAxis;
    Ase_SingleFloat ScaleAxisAng;
};

class Ase_HelperObject : public AseGenericItem
{
public:
    Ase_String Name;
    Ase_String HelperClass;
    Ase_String NodeParent; //optional.
    Ase_Node_TM NodeTM;
    Ase_ThreeFloat BoundingBoxMin;
    Ase_ThreeFloat BoundingBoxMax;
};

////////////////////////////////////////////

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
};

class Ase_GeomObject : public AseGenericItem
{
public:
    Ase_String Name;
    Ase_String Parent;
    Ase_Node_TM Node;

};



#endif // ASEMODEL

