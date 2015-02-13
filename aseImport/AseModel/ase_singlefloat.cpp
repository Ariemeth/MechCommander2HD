#include "../Include/AseModel/ase_singlefloat.hpp"

void Ase_SingleFloat::setF( double dX)
{
    _X = dX;
}

double Ase_SingleFloat::getF()
{
    return _X;
}
