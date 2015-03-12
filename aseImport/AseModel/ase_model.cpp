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
            continue;
        }

        if (fInHelpNode)
        {
            if (lines[i].contains(ASE_NODE_TM))
            {
                fInAseNode = true;
                continue;
            }
            if (fInAseNode)
            {                
                if (line.contains("}") && fInAseNode)
                {
                    fInAseNode = false;
                    continue;
                }
                local->NodeTM.setData(line);
                continue;
            }
            if (line.contains("}") &&
                    !fInAseNode)
            {
                fInHelpNode = false;
                continue;
            }
            local->setData(line);
            continue;
        }
    }

}
