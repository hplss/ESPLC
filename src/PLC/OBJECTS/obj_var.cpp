#include "obj_var.h"

void Ladder_VAR::setLineState(bool &state, bool bNot)
{ 
    if ( state ) //active up till this point
    {
        switch(getType())
        {
            case OBJ_TYPE::TYPE_VAR_BOOL:
                state = (bNot ? !getBoolValue() : getBoolValue());
                break;
            case OBJ_TYPE::TYPE_VAR_FLOAT:
                {
                    bool check = (getDoubleValue() > 0);
                    state = ( bNot ? !(check) : (check) ); //turnaries yay
                }
                break;
            case OBJ_TYPE::TYPE_VAR_USHORT:
                {
                    bool check = getUShortValue();
                    state = ( bNot ? !(check) : (check) ); //unsigned cannot be less than 0
                }
                break;
            case OBJ_TYPE::TYPE_VAR_INT:
                {
                    bool check = (getIntValue() > 0);
                    state = ( bNot ? !(check) : (check) );
                }   
                break;
            case OBJ_TYPE::TYPE_VAR_UINT: 
                {
                    bool check = getUIntValue();
                    state = ( bNot ? !(check) : (check) );
                }
                break;
            case OBJ_TYPE::TYPE_VAR_LONG:
                {
                    bool check = (getLongValue() > 0);
                    state = (bNot ? !(check) : (check));
                }
                break;
            case OBJ_TYPE::TYPE_VAR_ULONG:
                {
                    bool check = (getULongValue() > 0);
                    state = (bNot ? !(check) : (check));
                }
                break;
            default:
                state = false;
                break;
        }
    }

    Ladder_OBJ_Logical::setLineState(state, bNot); 
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
            else
                return static_cast<T>(values.b.val);
        }
        break;
        case OBJ_TYPE::TYPE_VAR_FLOAT:
        {
            if ( b_usesPtr )
                return static_cast<T>(*values.d.val_ptr);
            else
                return static_cast<T>(values.d.val);
        }
        break;
        case OBJ_TYPE::TYPE_VAR_USHORT:
        {
            if ( b_usesPtr )
                return static_cast<T>(*values.us.val_ptr);
            else
                return static_cast<T>(values.us.val);
        }
        break;
        case OBJ_TYPE::TYPE_VAR_INT:
        {
            if ( b_usesPtr )
                return static_cast<T>(*values.i.val_ptr);
            else
                return static_cast<T>(values.i.val);
        }
        break;
        case OBJ_TYPE::TYPE_VAR_UINT: 
        {
            if ( b_usesPtr )
                return static_cast<T>(*values.ui.val_ptr);
            else
                return static_cast<T>(values.ui.val);
        }
        break;
        case OBJ_TYPE::TYPE_VAR_LONG:
        {
            if ( b_usesPtr )
                return static_cast<T>(*values.l.val_ptr);
            else
                return static_cast<T>(values.l.val);
        }
        break;
        case OBJ_TYPE::TYPE_VAR_ULONG:
        {
            if ( b_usesPtr )
                return static_cast<T>(*values.ul.val_ptr);
            else
                return static_cast<T>(values.ul.val);
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
            else
                values.b.val = static_cast<bool>(val);
        }
        break;
        case OBJ_TYPE::TYPE_VAR_FLOAT:
        {
            if ( b_usesPtr )
                *values.d.val_ptr = static_cast<double>(val);
            else
                values.d.val = static_cast<double>(val);
        }
        break;
        case OBJ_TYPE::TYPE_VAR_USHORT:
        {
            if ( b_usesPtr )
                *values.us.val_ptr = static_cast<uint16_t>(val);
            else
                values.us.val = static_cast<uint16_t>(val);
        }
        break;
        case OBJ_TYPE::TYPE_VAR_INT:
        {
            if ( b_usesPtr )
                *values.i.val_ptr = static_cast<int_fast32_t>(val);
            else
                values.i.val = static_cast<int_fast32_t>(val);
        }
        break;
        case OBJ_TYPE::TYPE_VAR_UINT:
        {
            if ( b_usesPtr )
                *values.ui.val_ptr = static_cast<uint_fast32_t>(val);
            else
                values.ui.val = static_cast<uint_fast32_t>(val);
        }
        break;
        case OBJ_TYPE::TYPE_VAR_LONG:
        {
            if ( b_usesPtr )
                *values.l.val_ptr = static_cast<int64_t>(val);
            else
                values.l.val = static_cast<int64_t>(val);
        }
        break;
        case OBJ_TYPE::TYPE_VAR_ULONG:
        {
            if ( b_usesPtr )
                *values.ul.val_ptr = static_cast<uint64_t>(val);
            else
                values.ul.val = static_cast<uint64_t>(val);
        }
        break;
    }
}