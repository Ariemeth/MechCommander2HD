#include "../Include/AseModel/ase_singlefloat.hpp"

void Ase_SingleFloat::setF( QString dX)
{
    _X = dX.toDouble();
}

double Ase_SingleFloat::getF()
{
    return _X;
}
