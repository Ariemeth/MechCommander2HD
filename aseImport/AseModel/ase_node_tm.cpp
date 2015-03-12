#include "../include/AseModel/asemodel.hpp"

void Ase_Node_TM::setData(QString string)
{
    QStringList line = string.split(" ");

    if (line.contains(ASE_NODE_NAME))
    {
        this->NodeTM.Name.setString(line[1]);
    }
    if (line.contains(ASE_INHERIT_POS))
    {
        this->NodeTM.InheritPos.X.setI(line[1]);
        this->NodeTM.InheritPos.Y.setI(line[2]);
        this->NodeTM.InheritPos.Z.setI(line[3]);
    }
    if (line.contains(ASE_INHERIT_ROT))
    {
        this->NodeTM.InheritRot.X.setI(line[1]);
        this->NodeTM.InheritRot.Y.setI(line[2]);
        this->NodeTM.InheritRot.Z.setI(line[3]);
    }
    if (line.contains(ASE_INHERIT_SCL))
    {
        this->NodeTM.InheritScl.X.setI(line[1]);
        this->NodeTM.InheritScl.Y.setI(line[2]);
        this->NodeTM.InheritScl.Z.setI(line[3]);
    }
    if (line.contains(ASE_TM_ROW0))
    {
        this->NodeTM.TM_Row0.X.setF(line[1]);
        this->NodeTM.TM_Row0.Y.setF(line[2]);
        this->NodeTM.TM_Row0.Z.setF(line[3]);
    }
    if (line.contains(ASE_TM_ROW1))
    {
        this->NodeTM.TM_Row1.X.setF(line[1]);
        this->NodeTM.TM_Row1.Y.setF(line[2]);
        this->NodeTM.TM_Row1.Z.setF(line[3]);
    }
    if (line.contains(ASE_TM_ROW2))
    {
        this->NodeTM.TM_Row2.X.setF(line[1]);
        this->NodeTM.TM_Row2.Y.setF(line[2]);
        this->NodeTM.TM_Row2.Z.setF(line[3]);
    }
    if (line.contains(ASE_TM_ROW3))
    {
        this->NodeTM.TM_Row3.X.setF(line[1]);
        this->NodeTM.TM_Row3.Y.setF(line[2]);
        this->NodeTM.TM_Row3.Z.setF(line[3]);
    }

    if (line.contains(ASE_TM_POS))
    {
        this->NodeTM.TM_Pos.X.setF(line[1]);
        this->NodeTM.TM_Pos.Y.setF(line[2]);
        this->NodeTM.TM_Pos.Z.setF(line[3]);
    }
    if (line.contains(ASE_TM_ROTAXIS))
    {
        this->NodeTM.TM_RotAxis.X.setF(line[1]);
        this->NodeTM.TM_RotAxis.Y.setF(line[2]);
        this->NodeTM.TM_RotAxis.Z.setF(line[3]);
    }
    if (line.contains(ASE_TM_ROTANGLE))
    {
        this->NodeTM.TM_RotAngle.setF(line[1]);
    }
    if (line.contains(ASE_TM_SCALE))
    {
        this->NodeTM.TM_Scale.X.setF(line[1]);
        this->NodeTM.TM_Scale.Y.setF(line[2]);
        this->NodeTM.TM_Scale.Z.setF(line[3]);
    }
    if (line.contains(ASE_TM_SCALEAXIS))
    {
        this->NodeTM.TM_ScaleAxis.X.setF(line[1]);
        this->NodeTM.TM_ScaleAxis.Y.setF(line[2]);
        this->NodeTM.TM_ScaleAxis.Z.setF(line[3]);
    }
    if (line.contains(ASE_TM_SCALEAXISANG))
    {
        this->NodeTM.TM_ScaleAxisAng.setF(line[1]);
    }
}
