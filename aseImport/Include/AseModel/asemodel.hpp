#ifndef ASEMODEL
#define ASEMODEL
#include "ase_string.hpp"
#include "ase_material_parent.hpp"
#include "ase_helperobject.hpp"
#include "ase_geomobject.hpp"
#include <QStringList>
#include <QFile>
#include <QTextStream>

// ASE File Parse String Macros
#define ASE_HEADER				"*3DSMAX_ASCIIEXPORT\t200"
#define ASE_OBJECT				"*GEOMOBJECT {"
#define ASE_HELP_OBJECT			"*HELPEROBJECT {"
#define ASE_MESH				"*MESH {"
#define ASE_NUM_VERTEX			"*MESH_NUMVERTEX"
#define ASE_NUM_FACE			"*MESH_NUMFACES"
#define ASE_VERTEX_LIST			"*MESH_VERTEX_LIST {"
#define ASE_FACE_LIST			"*MESH_FACE_LIST {"
#define ASE_NUM_TVERTEX			"*MESH_NUMTVERTEX"
#define ASE_TVERTLIST			"*MESH_TVERTLIST {"
#define ASE_NUM_TVFACES			"*MESH_NUMTVFACES"
#define ASE_TFACELIST			"*MESH_TFACELIST {"
#define ASE_NUM_CVERTEX			"*MESH_NUMCVERTEX"
#define ASE_CVERTLIST			"*MESH_CVERTLIST {"
#define ASE_NUM_CVFACES			"*MESH_NUMCVFACES"
#define	ASE_CFACELIST			"*MESH_CFACELIST {"
#define ASE_MESH_NORMALS		"*MESH_NORMALS {"
#define ASE_HELPER_CLASS        "*HELPER_CLASS"
#define ASE_BOUNDINGBOX_MIN     "*BOUNDINGBOX_MIN"
#define ASE_BOUNDINGBOX_MAX     "*BOUNDINGBOX_MAX"
#define ASE_NODE_TM             "*NODE_TM {"

#define ASE_NODE_NAME			"*NODE_NAME"
#define ASE_NODE_PARENT			"*NODE_PARENT"
#define ASE_NODE_POS			"*TM_POS"

#define ASE_ANIMATION			"*TM_ANIMATION {"
#define ASE_ANIM_ROT_HEADER		"*CONTROL_ROT_TRACK {"
#define ASE_ANIM_ROT_SAMPLE		"*CONTROL_ROT_SAMPLE"
#define ASE_ANIM_POS_HEADER		"*CONTROL_POS_TRACK {"
#define ASE_ANIM_POS_SAMPLE		"*CONTROL_POS_SAMPLE"
#define ASE_ANIM_FIRST_FRAME	"*SCENE_FIRSTFRAME"
#define ASE_ANIM_LAST_FRAME		"*SCENE_LASTFRAME"
#define ASE_ANIM_FRAME_SPEED	"*SCENE_FRAMESPEED"
#define ASE_ANIM_TICKS_FRAME	"*SCENE_TICKSPERFRAME"

#define ASE_FACE_NORMAL_ID		"*MESH_FACENORMAL"
#define ASE_VERTEX_NORMAL_ID	"*MESH_VERTEXNORMAL"
#define ASE_MESH_CFACE_ID		"*MESH_CFACE"
#define ASE_MESH_VERTCOL_ID		"*MESH_VERTCOL"
#define ASE_MESH_TFACE_ID		"*MESH_TFACE"
#define ASE_MESH_TVERT_ID		"*MESH_TVERT"
#define ASE_MESH_FACE_ID		"*MESH_FACE"
#define ASE_MESH_VERTEX_ID		"*MESH_VERTEX"

#define ASE_MATERIAL_COUNT		"*MATERIAL_COUNT"
#define ASE_SUBMATERIAL_COUNT	"*NUMSUBMTLS"
#define ASE_MATERIAL_ID			"*MATERIAL"
#define ASE_MATERIAL_BITMAP_ID	"*BITMAP "
#define ASE_MATERIAL_CLASS		"*MATERIAL_CLASS"
#define ASE_MATERIAL_TWOSIDED	"*MATERIAL_TWOSIDED"
#define ASE_FACE_MATERIAL_ID	"*MESH_MTLID"

#define ASE_INHERIT_POS         "*INHERIT_POS"
#define ASE_INHERIT_ROT         "*INHERIT_ROT"
#define ASE_INHERIT_SCL         "*INHERIT_SCL"
#define ASE_TM_ROW0             "*TM_ROW0"
#define ASE_TM_ROW1             "*TM_ROW1"
#define ASE_TM_ROW2             "*TM_ROW2"
#define ASE_TM_ROW3             "*TM_ROW3"
#define ASE_TM_POS              "*TM_POS"
#define ASE_TM_ROTAXIS          "*TM_ROTAXIS"
#define ASE_TM_ROTANGLE         "*TM_ROTANGLE"
#define ASE_TM_SCALE            "*TM_SCALE"
#define ASE_TM_SCALEAXIS        "*TM_SCALEAXIS"
#define ASE_TM_SCALEAXISANG     "*TM_SCALEAXISANG"


class Ase_Model : public AseGenericItem
{
public:
    Ase_Model::Ase_Model(QObject * parent = 0);
    Ase_Model::~Ase_Model();
    Ase_Model::Ase_Model(QObject*,QString);

    Ase_String header;
    Ase_String Comment;
    Ase_Material_Parent materialList;
    QList<Ase_HelperObject*> HelperObjects;
    QList<Ase_GeomObject*> GeomObjects;

private:
    void parseLines(QStringList);
};

#endif // ASEMODEL

