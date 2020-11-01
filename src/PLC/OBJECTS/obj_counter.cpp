#include "obj_counter.h"

//////////////////////////////////////////////////////////////////////////
// COUNTER OBJECT BEGIN
//////////////////////////////////////////////////////////////////////////
void CounterOBJ::updateObject(bool state)
{
	if ( getState() == STATE_ENABLED ) //make sure we don't increment the accumulator twice
		return; //already enabled
		
	setAccumVal( getAccumVal() + 1 ); //increment 1
	if ( iAccum >= iCount )//we're above the set limit for counting
		setState(STATE_ENABLED);
}

shared_ptr<Ladder_VAR> CounterOBJ::addObjectVAR( const String &id )
{ 
    if ( !getObjectVAR(id) ) //already exists?
    {
        shared_ptr<Ladder_VAR> var = 0;
        if ( id == bitTagEN )
            var = make_shared<Ladder_VAR>(&enableBit, id);
        else if ( id == bitTagDN )
            var = make_shared<Ladder_VAR>(&doneBit, id);
        else if ( id == bitTagPRE )
            var = make_shared<Ladder_VAR>(&iCount, id);
        else if ( id == bitTagACC)
            var = make_shared<Ladder_VAR>(&iAccum, id);

        if ( var )
        {
            #ifdef DEBUG 
            Serial.println(PSTR("Created new Counter Object Tag: ") + id ); 
            #endif
            getObjectVARs().emplace_back(var);
            return var;
        }
    }
    #ifdef DEBUG 
    Serial.println(PSTR("Failed: Object Tag: ") + id ); 
    #endif
    return 0;
}
