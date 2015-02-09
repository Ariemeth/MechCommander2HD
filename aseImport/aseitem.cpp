#include "aseitem.h"

void AseGenericItem::getType()
{
    return _typeName;
}

QString AseGenericItem::setType(QString name)
{
 _typeName = name;
}

void Ase_SingleFloat::setF( double dX)
{
    _X = dX;
}

double Ase_SingleFloat::getF()
{
    return _X;
}

void Ase_SingleInt::setI(int dX)
{
    _X = dX;
}

int Ase_SingleInt::getI()
{
    return _X;
}

void Ase_String::setString(QString string)
{
    _String = string;
}

QString Ase_String::getString()
{
    return _String;
}
