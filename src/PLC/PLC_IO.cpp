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

void Ladder_OBJ::getNextObj( uint16_t rungNum ) //perform internal logic, then get the next objects.
{
	pair<itr, itr> rungObjects = nextObj.equal_range(rungNum); //Find all objects for the current rung op
	for ( itr it = rungObjects.first; it != rungObjects.second; it++ )//handle each object attached to this object.
	{
		it->second->getObject()->setLineState(rungNum, getLineState());
		/*Ladder_OBJ *Obj = it->second->getObject().get();
		switch( it->second->getObject()->getType() )
		{
			case OBJ_TYPE::TYPE_INPUT: //Go to next input
				static_cast<InputOBJ *>(Obj)->setLineState(rungNum, getLineState());
				break;
			
			case OBJ_TYPE::TYPE_OUTPUT: //We've reached an output. So assume we just set it to high.
				static_cast<OutputOBJ *>(Obj)->setLineState(rungNum, getLineState());
				break;
			
			case OBJ_TYPE::TYPE_TOF: //Looks like we've reached a timer. Set timer bits as appropriate then move on (Timer counted as a noutput)
			case OBJ_TYPE::TYPE_TON:
				static_cast<TimerOBJ *>(Obj)->setLineState(rungNum, getLineState());
				break;
			
			case OBJ_TYPE::TYPE_CTD:
			case OBJ_TYPE::TYPE_CTU:
				static_cast<CounterOBJ *>(Obj)->setLineState(rungNum, getLineState());
				break;

			case OBJ_TYPE::TYPE_VAR_BOOL:
			case OBJ_TYPE::TYPE_VAR_FLOAT:
			case OBJ_TYPE::TYPE_VAR_INT:
			case OBJ_TYPE::TYPE_VAR_LONG:
			case OBJ_TYPE::TYPE_VAR_ULONG:
			case OBJ_TYPE::TYPE_VAR_UINT:
				static_cast<Ladder_VAR *>(Obj)->setLineState(rungNum, getLineState());
				break;
			
			case OBJ_TYPE::TYPE_REMOTE:
				static_cast<Remote_Ladder_OBJ *>(Obj)->setLineState(rungNum, getLineState());
				break;

			default:
			break;
		}*/
	}
}


bool Ladder_OBJ::addNextObject( uint16_t rungNum, shared_ptr<Ladder_OBJ_Wrapper> obj ) //Parallel objects that are next in the rung can also be added
{
	//Some tests to make sure we can do that? Not sure what those tests would be yet. 
	nextObj.emplace(rungNum,obj);
	return true;
}

bool Ladder_OBJ::addNextObject( uint16_t rungNum, vector<shared_ptr<Ladder_OBJ_Wrapper>> &objects )
{
	for (uint8_t x = 0; x < objects.size(); x++ )
		nextObj.emplace(rungNum,objects[x]);
		
	objects.clear(); //empty here, just to make sure we don't add the exact same objects to something else later
	return true;
}

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
	bool state = getLineState()!=getLogic() ? LOW : HIGH;
	digitalWrite(iPin, state );
	//Serial.print("Output State: ");
	//Serial.println(state);
	Ladder_OBJ::updateObject();
}

//////////////////////////////////////////////////////////////////////////
// INPUT OBJECT BEGIN
//////////////////////////////////////////////////////////////////////////
void InputOBJ::updateObject()
{
	setState(getLineState());//store the most recent state
	Ladder_OBJ::updateObject(); //parent class - must be called last
}

void InputOBJ::setLineState(uint16_t rung, bool state )
{
	iValue = getInput();
	if ( getType() == TYPE_INPUT ) //digital only
	{
		if ( state ) //looks like we have a high state coming in to this object from the previous on the rung, let's see if we pass the state tests for this object.
		{
			if(!iValue && getLogic() == LOGIC_NO) //input is low (button not pressed) and logic is normally open (default position of button is off)
				state = false; //input not activated
			else if (iValue && getLogic() == LOGIC_NC) //input is high (button is pressed), but logic is normally closed (0)
				state = false; //input not activated, only active if input is 0 in this case
		}
	}
	
	Ladder_OBJ::setLineState( rung, state ); //let the parent handle it from here.
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
void CounterOBJ::updateObject()
{
	if ( getState() == STATE_ENABLED ) //make sure we don't increment the accumulator twice
		return; //already enabled
		
	setAccumVal( getAccumVal() + 1 ); //increment 1
	if ( iAccum >= iCount )//we're above the set limit for counting
		setState(STATE_ENABLED);
}


//////////////////////////////////////////////////////////////////////////
// 