#include "obj_var.h"

void Ladder_VAR::setLineState(bool &state, bool bNot)
{ 
    if ( state ) //active up till this point
    {
        bool check = (getValue<double>() > 0); //seems safe enough for now..
        state = ( bNot ? !(check) : (check) );
    }

    Ladder_OBJ_Logical::setLineState(state, bNot); 
}

void Ladder_VAR::setValue( const String &str )
{
    if ( iType == OBJ_TYPE::TYPE_VAR_FLOAT )
        setValue( str.toDouble() );
    else
        setValue( static_cast<int64_t>(strtoll(str.c_str(), NULL, 10)) );
}

String Ladder_VAR::getValueStr()
{
    String value;
    switch(iType) //get the correct value from the variable object
    {
        case OBJ_TYPE::TYPE_VAR_USHORT:
            value = getValue<uint16_t>();
        break;
        case OBJ_TYPE::TYPE_VAR_UINT:
            value = getValue<uint_fast32_t>();
        break;
        case OBJ_TYPE::TYPE_VAR_INT:
            value = getValue<int_fast32_t>();
        break;
        case OBJ_TYPE::TYPE_VAR_ULONG:
            value = intToStr(getValue<uint64_t>());
        break;
        case OBJ_TYPE::TYPE_VAR_LONG:
            value = intToStr(getValue<int64_t>());
        break;
        case OBJ_TYPE::TYPE_VAR_FLOAT:
            value = getValue<double>();
        break;
        case OBJ_TYPE::TYPE_VAR_BOOL:
            value = getValue<bool>();
        break;
        default: //default case
        break;
    }

    return value;
}

bool Ladder_VAR::operator<=(const VAR_PTR &B)
{
    if ( this->getValue<double>() <= B->getValue<double>() )
        return true;
    
    return false;
}

bool Ladder_VAR::operator<(const VAR_PTR &B)
{
    if ( this->getValue<double>() < B->getValue<double>() )
        return true;
    
    return false;
}

bool Ladder_VAR::operator>=(const VAR_PTR &B)
{
    if ( this->getValue<double>() >= B->getValue<double>() )
        return true;
    
    return false;
}

bool Ladder_VAR::operator>(const VAR_PTR &B)
{
    if ( this->getValue<double>() > B->getValue<double>() )
        return true;
    
    return false;
}

bool Ladder_VAR::operator==(const VAR_PTR &B)
{
    if ( this->getValue<double>() == B->getValue<double>() )
        return true;
    
    return false;
}

bool Ladder_VAR::operator!=(const VAR_PTR &B)
{
    return !this->operator==(B);
}

Ladder_VAR Ladder_VAR::operator=(const VAR_PTR &B)
{
    Ladder_VAR var = *B;
    return var;
}

//Global operators for Ladder_VAR objects below here
const double operator-(const VAR_PTR &var1, const VAR_PTR &var2 )
{
	return (var1->getValue<double>() - var2->getValue<double>());
}
const double operator+(const VAR_PTR &var1, const VAR_PTR &var2 )
{
	return (var1->getValue<double>() + var2->getValue<double>());
}
const double operator*(const VAR_PTR &var1, const VAR_PTR &var2 )
{
	return (var1->getValue<double>() * var2->getValue<double>());
}
const double operator/(const VAR_PTR &var1, const VAR_PTR &var2 )
{
	return (var1->getValue<double>() / var2->getValue<double>());
}
bool operator==(const VAR_PTR &var1, const VAR_PTR &var2 )
{
    return (var1->getValue<double>() == var2->getValue<double>());
}
bool operator>=(const VAR_PTR &var1, const VAR_PTR &var2 )
{
    return (var1->getValue<double>() >= var2->getValue<double>() );
}
bool operator<=(const VAR_PTR &var1, const VAR_PTR &var2 )
{
    return ( var1->getValue<double>() <= var2->getValue<double>() );
}
bool operator>(const VAR_PTR &var1, const VAR_PTR &var2 )
{
    return !operator<=(var1,var2);
} 
bool operator<(const VAR_PTR &var1, const VAR_PTR &var2 )
{
    return !operator>=(var1,var2);
}
bool operator!=(const VAR_PTR &var1, const VAR_PTR &var2 )
{
    return !operator==(var1,var2);
} 

