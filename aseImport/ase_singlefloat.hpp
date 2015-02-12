#ifndef ASE_SINGLEFLOAT_HPP
#define ASE_SINGLEFLOAT_HPP
#include "aseitem.h"

class Ase_SingleFloat : AseGenericItem // Useful for TM_ROTATE_ANGLE
{
    Q_OBJECT
public:
    void setF(double);
    double getF();

private:
    double _X;
};

#endif // ASE_SINGLEFLOAT_HPP

