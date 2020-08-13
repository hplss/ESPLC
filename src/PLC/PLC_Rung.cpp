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
	if ( !obj )
		return false; //must be a valid object.

	uint16_t objID = obj->getObject()->getID();
	for ( uint16_t x = 0; x < getNumRungObjects(); x++)
	{
		if ( objID > 0 && rungObjects[x]->getObject()->getID() == objID ) //new object must have an ID greater than 0
		{
			return false; //Do not add duplicates to the vector, except for child variables
		}
	}
	
	 rungObjects.emplace_back(obj); 
	 return true;
}

bool Ladder_Rung::addInitialRungObject( shared_ptr<Ladder_OBJ_Wrapper> obj )
{
	uint16_t objID = obj->getObject()->getID();
	for ( uint16_t x = 0; x < getNumInitialRungObjects(); x++)
	{
		if ( firstRungObjects[x]->getObject()->getID() == objID )
		{
			Serial.println("Cannot add duplicate items to initial.");
			return false; //Do not add duplicates to the vector
		}
	}
	
	#ifdef DEBUG
	Serial.println(PSTR("Adding Initial"));
	#endif

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
	}
	
	//Now that each rung object knows its line state, it's time to apply any settings to outputs, etc. across the entire rung.
	for ( uint16_t x = 0; x < getNumRungObjects(); x++ )
	{
		getRungObjects()[x]->getObject()->updateObject();
	}
}
