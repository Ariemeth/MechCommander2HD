
#include "McAseModel.h"

McAseModel::McAseModel(QObject * parent, QString fileName)
{
    this->parent(parent);
    QFile * file = new QFile(fileNmae);
    file->open(QIODevice::ReadWrite);

    //http://makc.googlecode.com/svn/trunk/flash/alternativa3d_ase/src/EmbeddedASEParser.as - template code

}
