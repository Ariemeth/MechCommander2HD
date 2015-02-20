#ifndef ASE_SINGLEFLOAT_HPP
#define ASE_SINGLEFLOAT_HPP
#include "aseitem.hpp"

class Ase_SingleFloat : AseGenericItem // Useful for TM_ROTATE_ANGLE
{
    Q_OBJECT
public:
    void setF(QString);
    double getF();

private:
    double _X;
};

#endif // ASE_SINGLEFLOAT_HPP

