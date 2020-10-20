#include "obj_timer.h"

//////////////////////////////////////////////////////////////////////////
// TIMER OBJECT BEGIN
//////////////////////////////////////////////////////////////////////////
void TimerOBJ::updateObject()
{	
	bool lineState = getLineState();
	if ( (lineState && getType() == OBJ_TYPE::TYPE_TON) || (!lineState && getType() == OBJ_TYPE::TYPE_TOF) )  //Is the pathway to this timer active?
	{
		uint32_t currentTime = millis();
		if ( enableBit != lineState && !ttBit ) //not already counting
		{
			ttBit = true;
			timeStart = currentTime;
		}
		if (lAccum >= lDelay )
		{
			doneBit = true;
			ttBit = false;
			lAccum = 0; //empty the accumulator
		}
		else
			lAccum = currentTime - timeStart; //update the accumulator
	}
	else //reset the timer 
	{
		ttBit = false; //timer no longer counting
		timeStart = 0;
		if ( getType() != OBJ_TYPE::TYPE_TRET) //Don't reset done bit for retentive timer. Must be done manually with reset coil 
			doneBit = false;
	}
	
	enableBit = lineState; //enable bit always matches line state. Set last.
	setState(lineState); //between enabled/disabled
	Ladder_OBJ_Logical::updateObject(); //parent class
}

shared_ptr<Ladder_VAR> TimerOBJ::addObjectVAR( const String &id )
{ 
    if ( !getObjectVAR(id) ) //proceed if it doesn't already exist
    {
        shared_ptr<Ladder_VAR> var = 0;
        if ( id == bitTagEN )
            var = make_shared<Ladder_VAR>(&enableBit, id);
        else if ( id == bitTagDN )
            var = make_shared<Ladder_VAR>(&doneBit, id);
        else if ( id == bitTagPRE )
            var = make_shared<Ladder_VAR>(&lDelay, id);
        else if ( id == bitTagTT )
            var = make_shared<Ladder_VAR>(&ttBit, id);
        else if ( id == bitTagACC)
            var = make_shared<Ladder_VAR>(&lAccum, id);

        if ( var )
        {
			getObjectVARs().emplace_back(var); //store it off.
            #ifdef DEBUG 
            Serial.println(PSTR("Created new Timer Object Tag: ") + id ); 
            #endif
            return var; 
        }
    }
    #ifdef DEBUG 
    Serial.println(PSTR("Failed: Object Tag: ") + id ); 
    #endif
    return 0; //failed to add
}

