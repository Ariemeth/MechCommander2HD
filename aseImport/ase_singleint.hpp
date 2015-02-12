#ifndef ASE_SINGLEINT_HPP
#define ASE_SINGLEINT_HPP

#include "aseitem.h"

class Ase_SingleInt : AseGenericItem
{

public:
    int getI();
    void setI(int);
private:
    int _X;
};

#endif // ASE_SINGLEINT_HPP

