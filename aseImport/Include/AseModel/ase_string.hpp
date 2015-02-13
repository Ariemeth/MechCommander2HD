#ifndef ASE_STRING
#define ASE_STRING
#include "aseitem.hpp"

class Ase_String : AseGenericItem // useful for things like NODE_NAME and NODE_PARENT
{
public:
    QString getString();
    void setString(QString);

    bool hasQuotation();
    void setQuotation(bool);
private:
    QString _String;
    bool _Quotation;
};

#endif // ASE_STRING

