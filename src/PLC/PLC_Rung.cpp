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
	rungObjects.clear(); //Clear the vector of objects.
	firstRungObjects.clear();
}

bool Ladder_Rung::addRungObject( OBJ_WRAPPER_PTR obj )
{
	if ( !obj )
		return false; //must be a valid object.
	
	 rungObjects.emplace_back(obj); 
	 return true;
}
bool Ladder_Rung::addInitialRungObject( const vector<OBJ_WRAPPER_PTR> &vec )
{
	for ( uint8_t x = 0; x < vec.size(); x++ )
	{
		if ( !addInitialRungObject(vec[x]) )
			return false;
	}
	
	return true; //default return path 
}
bool Ladder_Rung::addInitialRungObject( OBJ_WRAPPER_PTR obj )
{
	#ifdef DEBUG
	Serial.println(PSTR("Adding Initial"));
	#endif

	firstRungObjects.emplace_back(obj);
	return true;
}

OBJ_WRAPPER_PTR Ladder_Rung::getRungObjectByID(const String &id)
{
	for ( uint16_t x = 0; x < getNumRungObjects(); x++ )
	{
		if ( rungObjects[x]->pObj->sObjID == id )
			return rungObjects[x];
	}
	
	return 0; //Found nothing
}

void Ladder_Rung::processRung( uint16_t rungNum ) //Begins the process 
{
	//begin the update process 
	//Line state is always true at the beginning of the rung. From this point, the objects should handle all logic operations on their own until all pathways are checked
	for ( uint8_t x = 0; x < getNumInitialRungObjects(); x++ )
	{
		firstRungObjects[x]->setLineState(true);
	}
}
