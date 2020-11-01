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
                    if ( bNot ) //inverted logic
                        state = getDoubleValue() > 1 ? false : true;
                    else
                        state = getDoubleValue() > 1 ? true : false;
                }
                break;
            case OBJ_TYPE::TYPE_VAR_INT:
                {
                    if ( bNot )
                        state = getIntValue() > 1 ? false : true;
                    else
                        state = getIntValue() > 1 ? true : false;
                }   
                break;
            case OBJ_TYPE::TYPE_VAR_LONG:
                {
                    if ( bNot )
                        state = getLongValue() > 1 ? false : true;
                    else
                        state = getLongValue() > 1 ? true : false;
                }
                break;
            case OBJ_TYPE::TYPE_VAR_ULONG:
                {
                    if ( bNot )
                        state = getULongValue() > 1 ? false : true;
                    else
                        state = getULongValue() > 1 ? true : false;
                }
                break;
            case OBJ_TYPE::TYPE_VAR_UINT: 
                {
                    if ( bNot )
                        state = getUIntValue() > 1 ? false : true;
                    else
                        state = getUIntValue() > 1 ? true : false;
                }
                
                break;
            default:
                state = false;
                break;
        }
    }

    Ladder_OBJ::setLineState(state, bNot); 
}
