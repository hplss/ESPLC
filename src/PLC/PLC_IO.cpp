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

void Ladder_OBJ_Accessor::handleUpdates( const String &str)
{
	//Update Record Order: <ID>,<TYPE>,<STATE>,<LOGIC>,<VALUE>,<Other args (for init - maybe)> --> Per object
	if (strBeginsWith(str, CMD_SEND_UPDATE) ) //Updates only contain data that might change between updates (omitted: logic, type)
	{
		vector<String> groupData = splitString(removeFromStr(str, CMD_SEND_UPDATE), CHAR_UPDATE_GROUP); //remove the update prefix, then break the string up

		for ( uint16_t x = 0; x < groupData.size(); x++ )
		{
			vector<String> recordData = splitString(groupData[x], CHAR_UPDATE_RECORD); //break the group string up by records

			if( !recordData.size() )
				continue;

			//[0] = ID / [1] = STATE / [2] = VALUE/Linestate
			shared_ptr<Ladder_OBJ_Logical> obj = findLadderObjByID(recordData[0]);

			if( obj ) //found the object, perform the actual updates.
			{
				obj->setState(recordData[1].toInt());
			}
		}
	}
}

void Ladder_OBJ_Accessor::handleUpdates( const vector<String> &strVec )
{
	for ( uint8_t x = 0; x < strVec.size(); x++ )
		handleUpdates(strVec[x]);
}

shared_ptr<Ladder_OBJ_Logical> Ladder_OBJ_Accessor::handleInit(const String &str)
{
	shared_ptr<Ladder_OBJ_Logical> newObj = 0;
	if(strBeginsWith(str,CMD_SEND_INIT)) //should only be initializing individual objects (not groups of them? Hmm...)
	{
		vector<String> objRecords = splitString(removeFromStr(str, CMD_SEND_INIT), CHAR_UPDATE_RECORD );
		//[0] = ID, [1] = type, [2] = state, [3] = logic, [4] = value, [5] = other args (tbd)
		//perform object creation here - hopefully piggybacking off of the CreateNewObject code.
		//newObj = createLadderObj
	}
	return newObj;
}
