#include "../Include/AseModel/ase_string.hpp"

void Ase_String::setString(QString string)
{
    if (string.contains("\""))
    {
        setQuotation(true);
    }
    _String = string;
}

QString Ase_String::getString()
{
    return _String;
}

