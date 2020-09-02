#include "obj_var.h"

void Ladder_VAR::setLineState(bool &state)
{ 
    if ( state ) //active up till this point
    {
        switch(getType())
        {
            case OBJ_TYPE::TYPE_VAR_BOOL:
                state = getBoolValue();
                break;
            case OBJ_TYPE::TYPE_VAR_FLOAT:
                state = getFloatValue() > 1 ? true : false;
                break;
            case OBJ_TYPE::TYPE_VAR_INT:
                state = getIntValue() > 1 ? true : false;
                break;
            case OBJ_TYPE::TYPE_VAR_LONG:
                state = getLongValue() > 1 ? true : false;
                break;
            case OBJ_TYPE::TYPE_VAR_ULONG:
                state = getULongValue() > 1 ? true : false;
                break;
            case OBJ_TYPE::TYPE_VAR_UINT: 
                state = getUIntValue() > 1 ? true : false;
                break;
            default:
                state = false;
                break;
        }
    }

    Ladder_OBJ::setLineState(state); 
}

template <class T>
T Ladder_VAR::getValue()
{
    switch(getType())
    {
        case OBJ_TYPE::TYPE_VAR_BOOL:
        {
            if ( b_usesPtr )
                return static_cast<T>(*values.b.val_ptr);
        }
        break;
        case OBJ_TYPE::TYPE_VAR_FLOAT:
        {
            if ( b_usesPtr )
                return static_cast<T>(*values.f.val_ptr);
        }
        break;
        case OBJ_TYPE::TYPE_VAR_INT:
        {
            if ( b_usesPtr )
                return static_cast<T>(*values.i.val_ptr);
        }
        break;
        case OBJ_TYPE::TYPE_VAR_LONG:
        {
            if ( b_usesPtr )
                return static_cast<T>(*values.l.val_ptr);
        }
        break;
        case OBJ_TYPE::TYPE_VAR_ULONG:
        {
            if ( b_usesPtr )
                return static_cast<T>(*values.ul.val_ptr);
        }
        break;
        case OBJ_TYPE::TYPE_VAR_UINT: 
        {
            if ( b_usesPtr )
                return static_cast<T>(*values.ui.val_ptr);
        }
        break;
    }
    return static_cast<T>(0);
}

template <typename T>
void Ladder_VAR::setValue( const T val )
{
    switch(getType())
    {
        case OBJ_TYPE::TYPE_VAR_BOOL:
        {
            if ( b_usesPtr )
                *values.b.val_ptr = static_cast<bool>(val);
        }
        break;
        case OBJ_TYPE::TYPE_VAR_FLOAT:
        {
            if ( b_usesPtr )
                *values.f.val_ptr = static_cast<float>(val);
        }
        break;
        case OBJ_TYPE::TYPE_VAR_INT:
        {
            if ( b_usesPtr )
                *values.i.val_ptr = static_cast<int_fast32_t>(val);
        }
        break;
        case OBJ_TYPE::TYPE_VAR_LONG:
        {
            if ( b_usesPtr )
                *values.l.val_ptr = static_cast<int64_t>(val);
        }
        break;
        case OBJ_TYPE::TYPE_VAR_ULONG:
        {
            if ( b_usesPtr )
                *values.ul.val_ptr = static_cast<uint64_t>(val);
        }
        break;
        case OBJ_TYPE::TYPE_VAR_UINT:
        {
            if ( b_usesPtr )
                *values.ui.val_ptr = static_cast<uint_fast32_t>(val);
        }
        break;
    }
}