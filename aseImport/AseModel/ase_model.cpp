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
    // The next thing to do is to break this master list down
    // into its section headings.
    // These section headings are
    //
    // Header (first line)
    // Comment (Second line)
    // Scene (Class)
    // Material_list (class with subclasses)
    // HelperObject (class with subclass)
    // GeomObject (class with subclasses)

    // To detect subclasses, count the number of { and } braces.
    // if line contains {, openbraces ++,
    // if line contains }, openbraces--
    // if openBraces goes to 0 from higher, then start a new stringlist.
    // Pass the first line of a stringlist to the appropriate subclass to handle.

    // make sure each subclass handler can also return a qstring as data() and the correct
    //  column count.


}
