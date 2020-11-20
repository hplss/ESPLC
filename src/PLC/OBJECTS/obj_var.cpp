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
    if ( getType() == OBJ_TYPE::TYPE_VAR_FLOAT )
        setValue( str.toDouble() );
    else
        setValue( static_cast<int64_t>(strtoll(str.c_str(), NULL, 10)) );
}

String Ladder_VAR::getValueStr()
{
    String value;
    switch(getType()) //get the correct value from the variable object
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