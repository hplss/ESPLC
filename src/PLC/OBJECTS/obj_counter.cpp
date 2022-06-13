#include "obj_counter.h"

//////////////////////////////////////////////////////////////////////////
// COUNTER OBJECT BEGIN
//////////////////////////////////////////////////////////////////////////
void CounterOBJ::updateObject(bool state)
{
	if ( enableBit == getLineState() )
		return; //same state
	
    if ( iType == OBJ_TYPE::TYPE_COUNTER_UP ) //Count up to a specific value
    {
	    ptr_accum->setValue( ptr_accum + 1 ); 
        if ( ptr_accum >= ptr_count ) 
        {
            doneBit = true;
        }
    }
    else if (iType == OBJ_TYPE::TYPE_COUNTER_DOWN )
    {
        ptr_accum->setValue( ptr_accum - 1 );
        if ( ptr_accum <= ptr_count ) 
        {
            doneBit = true;
        }
    }

    enableBit = getLineState();
}

VAR_PTR CounterOBJ::getObjectVAR( const String &id )
{ 
    VAR_PTR var = Ladder_OBJ_Logical::getObjectVAR(id);
    if ( !var ) //already exists?
    {
        
        if ( id == bitTagEN )
            var = make_shared<Ladder_VAR>(&enableBit, id);
        else if ( id == bitTagDN )
            var = make_shared<Ladder_VAR>(&doneBit, id);
        else if ( id == bitTagPRE )
            return ptr_count;
        else if ( id == bitTagACC)
            return ptr_accum;

        if ( var )
        {
            #ifdef DEBUG 
            Serial.println(PSTR("Created new Counter Object Tag: ") + id ); 
            #endif
            getObjectVARs().emplace_back(var);
        }
    }

    if ( !var )
    {
        #ifdef DEBUG 
        Serial.println(PSTR("Failed: Object Tag: ") + id ); 
        #endif
    }
    
    return var;
}
