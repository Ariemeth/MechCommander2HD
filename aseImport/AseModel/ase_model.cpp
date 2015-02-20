#include "../Include/AseModel/asemodel.hpp"

Ase_Model::~Ase_Model()
{
    //do stuff
}

Ase_Model::Ase_Model(QObject *parent, QString filename)
{
    QStringList localContents;
    this->setParent(parent);

    QFile inputFile(filename);
    if (inputFile.open(QIODevice::ReadWrite))
    {
        QTextStream in(&inputFile);

        while ( !in.atEnd() )
        {
            QString * curLine = new QString(in.readLine());
            QString * simple = new QString(curLine->simplified());
            localContents.append(*simple);
        }
        inputFile.close();
        this->parseLines(localContents);
    }
}

void Ase_Model::parseLines(QStringList lines)
{

    // I have a list of all the lines and parameters at this stage.

    // re-implementing the method for parsing the file as found in
    // MechCommander2HD\Source\MCLib\msl.cpp

    //    int ase_material_count  = 0;

    //    for (int i = 0; i < lines.count(); i++)
    //    {
    //        QString curLine = lines[i];
    //        if (curLine.contains(ASE_MATERIAL_COUNT))
    //        {
    //            QStringList ase_mat_cnt = curLine.split(" ");
    //            ase_material_count = ase_mat_cnt[1];

    //            QStringList localLines = lines;
    //        }

    //    }

    //have to do this as multipass.  No other way.
    // first pass, helperdata

    bool fInAseNode = false;
    bool fInHelpNode = false;
    Ase_HelperObject *local = new Ase_HelperObject();

    for (int i = 0; i < lines.count(); i++)
    {
        QString line = lines[i];
        if (line.contains(ASE_HELP_OBJECT))
        {
            fInHelpNode = true;
        }

        if (fInHelpNode)
        {

            QStringList localList = lines[i].split(" ");
            if (localList.contains("}") &&
                    !fInAseNode)
            {
                fInHelpNode = false;
            }
            if (lines[i].contains(ASE_NODE_TM))
            {
                fInAseNode = true;
            }
            if (fInAseNode)
            {
                if (localList.contains(ASE_NODE_NAME))
                {
                    local->NodeTM.Name.setString(localList[1]);
                }
                if (localList.contains(ASE_INHERIT_POS))
                {
                    local->NodeTM.InheritPos.X.setI(localList[1]);
                    local->NodeTM.InheritPos.Y.setI(localList[2]);
                    local->NodeTM.InheritPos.Z.setI(localList[3]);
                }
                if (localList.contains(ASE_INHERIT_ROT))
                {
                    local->NodeTM.InheritRot.X.setI(localList[1]);
                    local->NodeTM.InheritRot.Y.setI(localList[2]);
                    local->NodeTM.InheritRot.Z.setI(localList[3]);
                }
                if (localList.contains(ASE_INHERIT_SCL))
                {
                    local->NodeTM.InheritScl.X.setI(localList[1]);
                    local->NodeTM.InheritScl.Y.setI(localList[2]);
                    local->NodeTM.InheritScl.Z.setI(localList[3]);
                }
                if (localList.contains(ASE_TM_ROW0))
                {
                    local->NodeTM.TM_Row0.X.setF(localList[1]);
                    local->NodeTM.TM_Row0.Y.setF(localList[2]);
                    local->NodeTM.TM_Row0.Z.setF(localList[3]);
                }
                if (localList.contains(ASE_TM_ROW1))
                {
                    local->NodeTM.TM_Row1.X.setF(localList[1]);
                    local->NodeTM.TM_Row1.Y.setF(localList[2]);
                    local->NodeTM.TM_Row1.Z.setF(localList[3]);
                }
                if (localList.contains(ASE_TM_ROW2))
                {
                    local->NodeTM.TM_Row2.X.setF(localList[1]);
                    local->NodeTM.TM_Row2.Y.setF(localList[2]);
                    local->NodeTM.TM_Row2.Z.setF(localList[3]);
                }
                if (localList.contains(ASE_TM_ROW3))
                {
                    local->NodeTM.TM_Row3.X.setF(localList[1]);
                    local->NodeTM.TM_Row3.Y.setF(localList[2]);
                    local->NodeTM.TM_Row3.Z.setF(localList[3]);
                }

                if (localList.contains(ASE_TM_POS))
                {
                    local->NodeTM.TM_Pos.X.setF(localList[1]);
                    local->NodeTM.TM_Pos.Y.setF(localList[2]);
                    local->NodeTM.TM_Pos.Z.setF(localList[3]);
                }
                if (localList.contains(ASE_TM_ROTAXIS))
                {
                    local->NodeTM.TM_RotAxis.X.setF(localList[1]);
                    local->NodeTM.TM_RotAxis.Y.setF(localList[2]);
                    local->NodeTM.TM_RotAxis.Z.setF(localList[3]);
                }
                if (localList.contains(ASE_TM_ROTANGLE))
                {
                    local->NodeTM.TM_RotAngle.setF(localList[1]);
                }
                if (localList.contains(ASE_TM_SCALE))
                {
                    local->NodeTM.TM_Scale.X.setF(localList[1]);
                    local->NodeTM.TM_Scale.Y.setF(localList[2]);
                    local->NodeTM.TM_Scale.Z.setF(localList[3]);
                }
                if (localList.contains(ASE_TM_SCALEAXIS))
                {
                    local->NodeTM.TM_ScaleAxis.X.setF(localList[1]);
                    local->NodeTM.TM_ScaleAxis.Y.setF(localList[2]);
                    local->NodeTM.TM_ScaleAxis.Z.setF(localList[3]);
                }
                if (localList.contains(ASE_TM_SCALEAXISANG))
                {
                    local->NodeTM.TM_ScaleAxisAng.setF(localList[1]);
                }
                if (localList.contains("}") && fInAseNode)
                {
                    fInAseNode = false;
                }
            }
            if (localList.contains(ASE_NODE_NAME))
            {
                local->Name.setString(localList[1]);
            }
            if (localList.contains(ASE_NODE_PARENT))
            {
                local->NodeParent.setString(localList[1]);
            }
            if (localList.contains(ASE_HELPER_CLASS))
            {
                local->HelperClass.setString(localList[1]);
            }
            if (localList.contains(ASE_BOUNDINGBOX_MAX))
            {
                local->BoundingBoxMax.X.setF(localList[1]);
                local->BoundingBoxMax.Y.setF(localList[2]);
                local->BoundingBoxMax.Z.setF(localList[3]);
            }
            if (localList.contains(ASE_BOUNDINGBOX_MIN))
            {
                local->BoundingBoxMin.X.setF(localList[1]);
                local->BoundingBoxMin.Y.setF(localList[2]);
                local->BoundingBoxMin.Z.setF(localList[3]);
            }
        }
    }

}
