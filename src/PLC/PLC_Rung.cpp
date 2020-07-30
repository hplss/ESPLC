/*
 * PLC_Rung.cpp
 *
 * Created: 9/24/2019 2:33:45 PM
 *  Author: Andrew
 */ 

#include "PLC_Rung.h"
#include <HardwareSerial.h>

Ladder_Rung::~Ladder_Rung()
{
	#ifdef DEBUG
	Serial.println(PSTR("Rung Destructor"));
	#endif
	rungObjects.clear(); //Clear the vector of objects.
	firstRungObjects.clear();
}

bool Ladder_Rung::addRungObject( shared_ptr<Ladder_OBJ_Wrapper> obj )
{
	for ( uint16_t x = 0; x < getNumRungObjects(); x++)
	{
		if ( obj->getObject()->getID() > 0 && rungObjects[x]->getObject()->getID() == obj->getObject()->getID() ) //new object must have an ID greater than 0
			return false; //Do not add duplicates to the vector, except for child variables
	}
	
	 rungObjects.emplace_back(obj); 
	 return true;
}

bool Ladder_Rung::addInitialRungObject( shared_ptr<Ladder_OBJ_Wrapper> obj )
{
	#ifdef DEBUG
	Serial.print(PSTR("Adding Initial"));
	#endif

	for ( uint16_t x = 0; x < getNumInitialRungObjects(); x++)
	{
		if ( firstRungObjects[x]->getObject()->getID() == obj->getObject()->getID() )
		{
			return false; //Do not add duplicates to the vector
		}
	}
	
	firstRungObjects.emplace_back(obj);
	return true;
}

shared_ptr<Ladder_OBJ_Wrapper> Ladder_Rung::getRungObjectByID(uint16_t id)
{
	for ( uint16_t x = 0; x < getNumRungObjects(); x++ )
	{
		if ( rungObjects[x]->getObject()->getID() == id )
			return rungObjects[x];
	}
	
	return 0; //Found nothing
}

void Ladder_Rung::processRung( uint16_t rungNum ) //Begins the process 
{
	if ( !getNumRungObjects() ) //just in case
	{
		#ifdef DEBUG
		Serial.println(PSTR("No rung objects available.")); //Debug purposes
		#endif
		return;
	}
	if ( !getNumInitialRungObjects() )
	{
		#ifdef DEBUG
		Serial.println(PSTR("No initial rung objects available.")); //debug purposes
		#endif
		return;
	}
	
	//begin the update process 
	//Line state is always true at the beginning of the rung. From this point, the objects should handle all logic operations on their own until all pathways are checked
	for ( uint8_t x = 0; x < getNumInitialRungObjects(); x++ )
	{
		firstRungObjects[x]->getObject()->setLineState(rungNum, true);
		/*Ladder_OBJ *rungObj = firstRungObjects[x]->getObject().get();
		switch( rungObj->getType() )
		{
			case OBJ_TYPE::TYPE_INPUT: //Go to next input
				static_cast<InputOBJ *>(rungObj)->setLineState(rungNum, true);
				break;
		
			case OBJ_TYPE::TYPE_OUTPUT: //We've reached an output. So assume we just set it to high.
				static_cast<OutputOBJ *>(rungObj)->setLineState(rungNum, true);
				break;
		
		
			case OBJ_TYPE::TYPE_TOF: //Looks like we've reached a timer. Set timer bits as appropriate then move on (Timer counted as an output)
			case OBJ_TYPE::TYPE_TON:
				static_cast<TimerOBJ *>(rungObj)->setLineState(rungNum, true);
				break;
		
			case OBJ_TYPE::TYPE_CTD:
			case OBJ_TYPE::TYPE_CTU:
				static_cast<CounterOBJ *>(rungObj)->setLineState(rungNum, true);
				break;
			
			case OBJ_TYPE::TYPE_VAR_BOOL:
			case OBJ_TYPE::TYPE_VAR_FLOAT:
			case OBJ_TYPE::TYPE_VAR_INT:
			case OBJ_TYPE::TYPE_VAR_LONG:
			case OBJ_TYPE::TYPE_VAR_ULONG:
			case OBJ_TYPE::TYPE_VAR_UINT:
				static_cast<Ladder_VAR *>(rungObj)->setLineState(rungNum, true);
				break;

			case OBJ_TYPE::TYPE_REMOTE:
				static_cast<Remote_Ladder_OBJ *>(rungObj)->setLineState(rungNum, true);
				break;
		
			default:
			break;
		}*/

	}
	
	//Now that each rung object knows its line state, it's time to apply any settings to outputs, etc. across the entire rung.
	for ( uint16_t x = 0; x < getNumRungObjects(); x++ )
	{
		getRungObjects()[x]->getObject()->updateObject();
		/*Ladder_OBJ *rungObj = getRungObjects()[x]->getObject().get();
		if ( !rungObj ) //just in case?
			break;
				
		switch( rungObj->getType() )
		{
			case OBJ_TYPE::TYPE_INPUT: //Go to next input
				static_cast<InputOBJ *>(rungObj)->updateObject();
				break;

			case OBJ_TYPE::TYPE_OUTPUT: //We've reached an output. So assume we just set it to high.
				static_cast<OutputOBJ *>(rungObj)->updateObject();
				break;
			
			case OBJ_TYPE::TYPE_TOF: //Looks like we've reached a timer. Set timer bits as appropriate then move on (Timer counted as an output)
			case OBJ_TYPE::TYPE_TON:
				static_cast<TimerOBJ *>(rungObj)->updateObject();
				break;
			
			case OBJ_TYPE::TYPE_CTD:
			case OBJ_TYPE::TYPE_CTU:
				static_cast<CounterOBJ *>(rungObj)->updateObject();
				break;

			case OBJ_TYPE::TYPE_VAR_BOOL:
			case OBJ_TYPE::TYPE_VAR_FLOAT:
			case OBJ_TYPE::TYPE_VAR_INT:
			case OBJ_TYPE::TYPE_VAR_LONG:
			case OBJ_TYPE::TYPE_VAR_ULONG:
			case OBJ_TYPE::TYPE_VAR_UINT:
				static_cast<Ladder_VAR *>(rungObj)->updateObject();
				break;

			case OBJ_TYPE::TYPE_REMOTE:
				static_cast<Remote_Ladder_OBJ *>(rungObj)->updateObject();
				break;
			
			
			default:
			break;
		}*/

	}
}
