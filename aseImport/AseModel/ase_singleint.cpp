#include "../Include/AseModel/ase_singleint.hpp"

void Ase_SingleInt::setI(QString dX)
{
    _X = dX.toInt();
}

int Ase_SingleInt::getI()
{
    return _X;
}
