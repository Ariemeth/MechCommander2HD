#ifndef ASE_SINGLEINT_HPP
#define ASE_SINGLEINT_HPP

#include "aseitem.hpp"

class Ase_SingleInt : AseGenericItem
{

public:
    int getI();
    void setI(QString);
private:
    int _X;
};

#endif // ASE_SINGLEINT_HPP

