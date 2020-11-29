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
#include "./OBJECTS/obj_var.h"
#include "./CORE/UICore.h"

extern UICore Core;

//////////////////////////////////////////////////////////////////////////
// BASECLASS OBJECT BEGIN
//////////////////////////////////////////////////////////////////////////

shared_ptr<Ladder_VAR> Ladder_OBJ::getObjectVAR( const String &id )
{
	for ( uint16_t x = 0; x < getObjectVARs().size(); x++ )
	{
		if ( getObjectVARs()[x]->getID() == id )
			return getObjectVARs()[x];
	}

	return 0;
}

void Ladder_OBJ_Accessor::handleUpdates( const String &str)
{
	//Update Record Order: <ID>,<VALUE> -- only updating Ladder_VAR objects that are locally stored, for now
	if (strContains(str, CMD_SEND_UPDATE) && strContains(str, CHAR_QUERY_END) ) //Updates only contain data that might change between updates (omitted: logic, type)
	{
		vector<String> groupData = splitString(removeFromStr(str, {CMD_SEND_UPDATE, CHAR_QUERY_END} ), CHAR_UPDATE_GROUP); //remove the update prefix, then break the string up

		for ( uint16_t x = 0; x < groupData.size(); x++ )
		{
			vector<String> recordData = splitString(groupData[x], CHAR_UPDATE_RECORD); //break the group string up by records

			if ( recordData.size() > 1 )
			{
				shared_ptr<Ladder_VAR> pVar = getObjectVAR( recordData[0] ); //attempt to find the (hopefully) existing var.

				if ( pVar ) //&& recordData[1].length()
					pVar->setValue( recordData[1] );
				else
				{
					//some error.. couldn't update the object because it doesn't exist here? Init problem? 
				}
			}
		}
	}
}

void Ladder_OBJ_Accessor::handleUpdates( const vector<String> &strVec )
{
	for ( uint8_t x = 0; x < strVec.size(); x++ ) //won't quite work in its current form because of start and end chars not both being present.
		handleUpdates(strVec[x]);
}

shared_ptr<Ladder_OBJ_Logical> Ladder_OBJ_Accessor::handleInit(const String &str)
{
	shared_ptr<Ladder_OBJ_Logical> newObj = 0;
	Core.sendMessage(PSTR("Received Reply from remote: ") + str);
	if(strContains(str, CMD_SEND_INIT) && strContains(str, CHAR_QUERY_END)) //should only be initializing individual objects (not groups of them? Hmm...)
	{
		vector<String> initObjects = splitString(removeFromStr(str, {CMD_SEND_INIT, CHAR_QUERY_END} ), CHAR_UPDATE_GROUP ); //just in case there are multiple items
		for ( uint16_t x = 0; x < initObjects.size(); x++ )
		{
			//Record ID's: [0] = ID, [1] = type, [2] = state, [3] = logic, [4] = value, [5] = other args (tbd)
			vector<String> objRecords = splitString(initObjects[x], CHAR_UPDATE_RECORD );
			//Right now, the only objects supported are LADDER_VAR objects. <ID><TYPE><VALUE>
			if ( objRecords.size() < 3 || objRecords.size() > 3 ) //invalid number of record given 
				return 0;
			
			int32_t tempInt = objRecords[1].toInt();
			if ( tempInt < 0 || !objRecords[0].length() ) //basic check for now -- improve this later.
				return 0; //invalid type

			shared_ptr<Ladder_VAR> newVar = 0;
			OBJ_TYPE varType = static_cast<OBJ_TYPE>(tempInt);
			switch (varType)
			{
				case OBJ_TYPE::TYPE_VAR_BOOL:
				newVar = make_shared<Ladder_VAR>( static_cast<bool>(objRecords[2].toInt()), objRecords[0] ); //store the entire identifier for now
				break;
				case OBJ_TYPE::TYPE_VAR_USHORT:
				newVar = make_shared<Ladder_VAR>( static_cast<uint16_t>(objRecords[2].toInt()), objRecords[0] );
				break;
				case OBJ_TYPE::TYPE_VAR_FLOAT:
				newVar = make_shared<Ladder_VAR>( static_cast<double>(objRecords[2].toDouble()), objRecords[0] );
				break;
				case OBJ_TYPE::TYPE_VAR_INT:
				newVar = make_shared<Ladder_VAR>( static_cast<int_fast32_t>(objRecords[2].toInt()), objRecords[0] );
				break;
				case OBJ_TYPE::TYPE_VAR_UINT:
				newVar = make_shared<Ladder_VAR>( static_cast<uint_fast32_t>(objRecords[2].toInt()), objRecords[0] );
				break;
				case OBJ_TYPE::TYPE_VAR_LONG:
				newVar = make_shared<Ladder_VAR>( parseInt(objRecords[2]), objRecords[0] );
				break;
				case OBJ_TYPE::TYPE_VAR_ULONG:
				newVar = make_shared<Ladder_VAR>( static_cast<uint64_t>(strtoull(objRecords[2].c_str(), NULL, 10)), objRecords[0] );
				break;
				case OBJ_TYPE::TYPE_VAR_STRING:
				newVar = make_shared<Ladder_VAR>( objRecords[2], objRecords[0] ); 
				break;
				default:
				break;
			}

			if ( newVar )
			{
				getObjectVARs().emplace_back(newVar); //Store in the local container for Ladder Var objects
				Core.sendMessage(PSTR("Created new remote var: ") + objRecords[0] );
				newObj = newVar;
			}
		}
	}

	return newObj;
}
