/*
 * PLC_IO.cpp
 *
 * Created: 9/22/2019 4:55:00 PM
 *  Author: Andrew Ward
 The purpose of the PLC_IO Objects is to serve as the actual logic objects in a ladder rung. These objects each have their own logic operations that they perform,
 and the state of the output line (from that object to the next) is passed along to the next object in the chain. This allows it to perform necessary actions based
 on that logic state. Some objects may have multiple objects that they point to (the case for an OR statement, for example), in this case: the rung manager will 
 note the position of these 'branches' in its own object, and will remember to each point in the logic after processing the previous pathway to the output(s).
 Objects include Timers, Counters, and standard I/O (as well as virtual 'coils').
 */ 

#include "PLC_IO.h"
#include <HardwareSerial.h>
#include <esp_timer.h>

//////////////////////////////////////////////////////////////////////////
// BASECLASS OBJECT BEGIN
//////////////////////////////////////////////////////////////////////////

shared_ptr<Ladder_VAR> Ladder_OBJ::getObjectVAR( const String &id )
{
	return 0;
}

shared_ptr<Ladder_VAR> Ladder_OBJ::addObjectVAR( const String &id )
{
	//Error here? Should only be called if the derived class doesn't support the requested bit tag. Like a counter referencing the TT bit.
	return 0;
}
//////////////////////////////////////////////////////////////////////////
// OUTPUT OBJECT BEGIN
//////////////////////////////////////////////////////////////////////////
void OutputOBJ::updateObject() //Logic used to update the coil
{
	bool lineState = (getLineState()!=getLogic()) ? LOW : HIGH;
	digitalWrite(iPin, lineState);
	//Serial.print("Output State: ");
	//Serial.println(state);
	Ladder_OBJ::updateObject();
}

//////////////////////////////////////////////////////////////////////////
// INPUT OBJECT BEGIN
//////////////////////////////////////////////////////////////////////////
void InputOBJ::updateObject()
{
	Ladder_OBJ::updateObject(); //parent class - must be called last
}

void InputOBJ::setLineState( bool &state )
{
	iValue = getInput();
	if ( getType() == TYPE_INPUT ) //digital only
	{
		if(!iValue && getLogic() == LOGIC_NO) //input is low (button not pressed) and logic is normally open (default position of button is off)
			state = false; //input not activated
		else if (iValue && getLogic() == LOGIC_NC) //input is high (button is pressed), but logic is normally closed (0)
			state = false; //input not activated, only active if input is 0 in this case
	}
	
	Ladder_OBJ::setLineState( state ); //let the parent handle it from here.
}


//////////////////////////////////////////////////////////////////////////
// TIMER OBJECT BEGIN
//////////////////////////////////////////////////////////////////////////
void TimerOBJ::updateObject()
{	
	bool lineState = getLineState();
	if ( (lineState && getType() == TYPE_TON) || (!lineState && getType() == TYPE_TOF) )  //Is the pathway to this timer active?
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
		if ( getType() != TYPE_TRET) //Don't reset done bit for retentive timer. Must be done manually with reset coil 
			doneBit = false;
	}
	
	enableBit = lineState; //enable bit always matches line state. Set last.
	setState(lineState); //between enabled/disabled
	Ladder_OBJ::updateObject(); //parent class
}

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


//////////////////////////////////////////////////////////////////////////
// 