/*
 * PLC_Main.cpp
 *
 * Created: 9/28/2019 5:35:08 PM
 *  Author: Andrew Ward
 */ 
#include "PLC_Main.h"
#include "PLC_Parser.h"
#include <HardwareSerial.h>

void PLC_Main::resetAll()
{
	currentObjID = 1; //reset the counter
	ladderRungs.clear(); //Empty created ladder rungs vector
	ladderObjects.clear(); //Empty the created ladder logic objects vector
	generatePinMap(); //reset and fill the pinmap
}

char toUpper( char x )
{
	if ( x > 96 && x < 123) //must be a valid char
	return x - 32;
	
	return x; //do nothing
}

String &toUpper(String &strn)
{
	strn.toUpperCase();
	return strn; //pass it along
}

void PLC_Main::generatePinMap()
{
	pinMap.clear(); //empty first, just in case.
	//Fill in the pin map with the appropriate pin relationships.
	pinMap.emplace(0, PIN_TYPE::PIN_O );
	pinMap.emplace(1, PIN_TYPE::PIN_INVALID ); //TX
	pinMap.emplace(2, PIN_TYPE::PIN_AIO );
	pinMap.emplace(3, PIN_TYPE::PIN_INVALID ); //RX
	pinMap.emplace(4, PIN_TYPE::PIN_AIO );
	pinMap.emplace(5, PIN_TYPE::PIN_IO );

	for ( uint8_t x = 6; x < 11; x++ )
		pinMap.emplace(x, PIN_TYPE::PIN_INVALID );

	pinMap.emplace(12, PIN_TYPE::PIN_AIO );
	pinMap.emplace(13, PIN_TYPE::PIN_AIO );
	pinMap.emplace(14, PIN_TYPE::PIN_AIO );
	pinMap.emplace(15, PIN_TYPE::PIN_AIO );

	for ( uint8_t x = 16; x < 23; x++ )
	{
		if ( x == 20) //no GPIO 20
			continue; 

		pinMap.emplace(x, PIN_TYPE::PIN_IO );
	}

	pinMap.emplace(25, PIN_TYPE::PIN_AIO );
	pinMap.emplace(26, PIN_TYPE::PIN_AIO );
	pinMap.emplace(27, PIN_TYPE::PIN_AIO );
	pinMap.emplace(32, PIN_TYPE::PIN_AIO );
	pinMap.emplace(33, PIN_TYPE::PIN_AIO );
	pinMap.emplace(34, PIN_TYPE::PIN_AI );
	pinMap.emplace(35, PIN_TYPE::PIN_AI );
	pinMap.emplace(36, PIN_TYPE::PIN_AI );
	pinMap.emplace(39, PIN_TYPE::PIN_AI );
}

String PLC_Main::getObjName( const String &parsedString )
{
	String objName;
	for ( uint8_t x =0; x < parsedString.length(); x++ )
	{
		if ( parsedString[x] == CHAR_VAR_OPERATOR ) //operator is always at end of name
			break; //end the loop
		if ( parsedString[x] == CHAR_NOT_OPERATOR ) //not operator is always at beginning of name
			continue; //skip the char
			
		objName += parsedString[x];
	}
	return objName;
}

void PLC_Main::processLogic()
{
	if ( !getNumRungs() )
		return;

	for (uint16_t x = 0; x < getNumRungs(); x++)
	{
		getLadderRung(x)->processRung(x);
	}
}

bool PLC_Main::addLadderRung(shared_ptr<Ladder_Rung> rung)
{
	if ( !rung->getRungObjects().size() ) //no objects in the rung?
		return false;
		
	//looks like we're good here	
	ladderRungs.emplace_back(rung);
	return true;
}

shared_ptr<Ladder_OBJ> PLC_Main::findLadderObjByID( const uint16_t id ) //Search through all created objects thus far. This assumes that the object was created successfully.
{
	for ( uint16_t x = 0; x < ladderObjects.size(); x++ )
	{
		shared_ptr<Ladder_OBJ>pObj = ladderObjects[x];
		if ( pObj && pObj->getID() == id )
			return pObj;
	}
	
	return 0; //default
}

shared_ptr<Ladder_OBJ> PLC_Main::findLadderObjByName( const String &name ) //only works in the parsing phase.
{
	for ( uint16_t x = 0; x < parsedLadderObjects.size(); x++ )
	{
		if ( parsedLadderObjects[x]->getName() == name)
			return parsedLadderObjects[x]->getObject();
	}
	
	return 0;
}

bool PLC_Main::parseScript(const char *script)
{
	resetAll(); //Purge all previous ladder logic objects before applying new script.
	  
	String scriptLine; //container for parsed characters
	uint16_t iLine = 0;
	for (uint16_t x = 0; x < strlen(script); x++) //go one char at a time.
	{
		if ( script[x] == CHAR_SPACE ) //omit spaces
			continue;
			
		if ( script[x] != CHAR_NEWLINE && script[x] != CHAR_CARRIAGE ) //Do this one line at a time.
		{
			scriptLine += toUpper(script[x]); //convert all chars to upper case. This might be done elsewhere later (web interface code) but for now we'll do it here
		}
		else if (scriptLine.length() > 1) //we've hit a newline or carriage return char and we've got a valid length
		{
			iLine++; //Looks like we have a valid line
			shared_ptr<PLC_Parser> parser = make_shared<PLC_Parser>(scriptLine, getNumRungs() );
			if ( !parser->parseLine() )
			{
				parsedLadderObjects.clear();
				sendError( ERR_PARSER_FAILED, PSTR("At Line: ") + String(iLine));
				return false; //error ocurred somewhere?
			}

			scriptLine.clear(); //empty the container for the next line
		}
	}

	parsedLadderObjects.clear(); //empty the parsed objects vector once we're done.
	return true; //success
}

shared_ptr<ladderOBJdata> PLC_Main::createNewObject(const String &name, const vector<String> &ObjArgs )
{
	if (ObjArgs.size() > 1) //must have at least one arg (first indictes the object type)
	{
		String type = ObjArgs[0];
		if ( type == variableTag1 || type == variableTag2 ) //Is this a variable type object? (Used for local data storage in memory, to facilitate communication between objects)
		{
			return make_shared<ladderOBJdata>(name, createVariableOBJ(ObjArgs));
		}
		else if ( type == inputTag1 || type == inputTag2 )
		{
			return make_shared<ladderOBJdata>(name, createInputOBJ(ObjArgs));
		}
		else if ( type == outputTag1 || type == outputTag2 ) 
		{
			return make_shared<ladderOBJdata>(name, createOutputOBJ(ObjArgs));
		}
		else if ( type == timerTag1 || type == timerTag2 ) 
		{
			return make_shared<ladderOBJdata>(name, createTimerOBJ(ObjArgs));
		}
		else if ( type == counterTag1 || type == counterTag2 ) 
		{
			return make_shared<ladderOBJdata>(name, createCounterOBJ(ObjArgs));
		}
		/*else if ( type == counterTag2 ) 
		{
			return make_shared<ladderOBJdata>(name, createMathOBJ(ObjArgs));
		}*/
		else //if we don't find a valid object type
			sendError(ERR_DATA::ERR_UNKNOWN_TYPE, type);
	}
	else //not enough arguments to create a new ladder object
		sendError(ERR_DATA::ERR_INSUFFICIENT_ARGS, name );
	
	return 0;
}

shared_ptr<Ladder_OBJ> PLC_Main::createInputOBJ( const vector<String> &args )
{
	uint8_t pin = 0, logic = LOGIC_NO, type = TYPE_INPUT, numArgs = args.size();

	if ( numArgs > 3 )
		logic = parseLogic(args[3]);

	if ( numArgs > 2 )
	{
		if ( args[2] == "A" || args[2] == typeTagAnalog ) //Explicitly declare an analog input, otherwise assume it's a digital in only
			type = TYPE_INPUT_ANALOG;
		else if (args[2] == "D" || args[2] == typeTagDigital )
			type = TYPE_INPUT;
	}
	
	if ( numArgs > 1 ) //needed
	{
		pin = args[1].toInt(); 
		if ( isValidPin(pin, type) )
		{
			shared_ptr<InputOBJ> newObj(new InputOBJ(currentObjID++, pin, type, logic));
			ladderObjects.emplace_back(newObj); //add to the list of global shared pointers for later reference.
			setClaimedPin(pin); //set the pin as claimed for this object.
			#ifdef DEBUG
			Serial.println(PSTR("NEW INPUT"));
			#endif
			return newObj;
		}
	}

	return 0;
}

shared_ptr<Ladder_OBJ> PLC_Main::createOutputOBJ( const vector<String> &args )
{
	uint8_t pin = 0, logic = LOGIC_NO, numArgs = args.size();
	if ( numArgs > 2 ) //caveat here is that outputs would NEVER normally be normally closed (ON)
		logic = parseLogic(args[2]);
		
	if ( numArgs > 1 )
	{
		pin = args[1].toInt();
		if ( isValidPin(pin, TYPE_OUTPUT) )
		{
			shared_ptr<OutputOBJ> newObj(new OutputOBJ(currentObjID++, pin, logic));
			ladderObjects.emplace_back(newObj);
			setClaimedPin( pin ); //claim the pin for this object.
			#ifdef DEBUG
			Serial.println(PSTR("NEW OUTPUT"));
			#endif
			return newObj;
		}
	}

	return 0;
}

shared_ptr<Ladder_OBJ> PLC_Main::createTimerOBJ( const vector<String> &args )
{
	uint8_t numArgs = args.size();
	uint32_t delay = 0, accum = 0, subType = TYPE_TON;
	if ( numArgs > 3 )
	{	  
		if ( args[3] == typeTagTOF )
			subType = TYPE_TOF;
		else if ( args[3] == typeTagTON )
			subType = TYPE_TON;
		else
			sendError(ERR_DATA::ERR_UNKNOWN_ARGS, args[0] + args[3]); 
	}
	if ( numArgs > 2 )
		accum = args[2].toInt(); //some output verification tests here

	if ( numArgs > 1 ) //must have at least one arg
	{
		delay = args[1].toInt();
		if (delay > 1) //Must have a valid delay time. 
		{
			shared_ptr<TimerOBJ> newObj(new TimerOBJ(currentObjID++, delay, accum, subType ));
			ladderObjects.emplace_back(newObj);
			#ifdef DEBUG
			Serial.println(PSTR("NEW TIMER"));
			#endif
			return newObj;
		}
	}

	return 0;
}
shared_ptr<Ladder_OBJ> PLC_Main::createCounterOBJ( const vector<String> &args )
{
	uint16_t count = 0, accum = 0; 
	uint8_t subType = TYPE_CTU, numArgs = args.size();
	if ( numArgs > 3 )
	{
		subType = TYPE_CTU; //default
		if ( args[3] == typeTagCTU )
			subType = TYPE_CTU;
		else if ( args[3] == typeTagCTD )
			subType = TYPE_CTD;
		else
			sendError(ERR_DATA::ERR_UNKNOWN_ARGS, args[0] + args[3]);
	}
	if ( numArgs > 2 )
		accum = args[2].toInt(); //some output verification tests here?
		
	if ( numArgs > 1 )
	{
		count = args[1].toInt();
		shared_ptr<CounterOBJ> newObj(new CounterOBJ(currentObjID++, count, accum, subType));
		ladderObjects.emplace_back(newObj);
		#ifdef DEBUG
		Serial.println(PSTR("NEW COUNTER"));
		#endif
		return newObj;
	}
	return 0;
}

//TODO - Implement some way where a user can easily dictate which variable type to use for memory purposes, otherwise default to auto-detection (maybe always estimate high? 64-bit?).
shared_ptr<Ladder_OBJ> PLC_Main::createVariableOBJ( const vector<String> &args )
{
	if ( args.size() > 1 )
	{
		bool isFloat = false;
		String val;
		for (uint8_t x = 0; x < args[1].length(); x++ )
		{
			if (x > 18) //this is plenty of digits, more than will ever likely be needed
				break;
				
			if ( (args[1][x] >= '0' && args[1][x] <= '9') || args[1][x] == '.' || ( args[1][x] == '-' && x == 0 ) )
			{
				if ( args[1][x] == '.' )
				{
					if ( isFloat )// multiple '.' chars should be ignored
						continue;
						
					isFloat = true;
				}
				
				val += args[1][x];
			}
		}
		
		if ( !val.length() )
			return 0;

		#ifdef DEBUG
		Serial.println(PSTR("NEW VARIABLE"));
		#endif
			
		if (isFloat)
			return make_shared<Ladder_VAR>( val.toFloat(), currentObjID++ );
		
		int64_t value = atoll(val.c_str());
		if ( value < 0 && abs( value ) <= INT_MAX )
			return make_shared<Ladder_VAR>( static_cast<int_fast32_t>(value), currentObjID++ ); //must be signed
		else if (value <= INT_MAX )
			return make_shared<Ladder_VAR>( static_cast<uint_fast32_t>(value), currentObjID++ ); //we can use unsigned
		else
			return make_shared<Ladder_VAR>( static_cast<uint64_t>(value), currentObjID++ ); //assume a long (geater than 32 bits)
	}

	return 0;
}

shared_ptr<Ladder_OBJ> PLC_Main::createMathOBJ( const vector<String> & args)
{
	return 0;
}

bool PLC_Main::isValidPin( uint8_t pin, uint8_t type)
{
	std::map<uint8_t, uint8_t>::iterator pinitr = pinMap.find(pin);
	if (pinitr != pinMap.end())
	{
		if ( pinitr->second == PIN_TYPE::PIN_TAKEN )
		{
			sendError( ERR_DATA::ERR_PIN_TAKEN, String(pin) );
			return false;
		}
		if ( pinitr->second == PIN_TYPE::PIN_INVALID )
		{
			sendError( ERR_DATA::ERR_PIN_INVALID, String(pin) );
			return false;
		}

		if ( ( type == TYPE_INPUT_ANALOG && ( pinitr->second == PIN_AI || pinitr->second == PIN_AIO ) ) 
			|| ( type == TYPE_INPUT &&  ( pinitr->second == PIN_AI || pinitr->second == PIN_AIO || pinitr->second == PIN_IO || pinitr->second == PIN_I ) ) )
			{
				return true;
			}
		else if ( type == TYPE_OUTPUT && ( pinitr->second == PIN_AIO || pinitr->second == PIN_IO || pinitr->second == PIN_O ) )
		{
			return true;
		}
	}

	return false; //invalid pin - does not exist
}

uint8_t PLC_Main::parseLogic( const String &arg )
{
	uint8_t logic;
	if ( arg == logicTagNO )
		logic = LOGIC_NO;
	else if ( arg == logicTagNC )
		logic = LOGIC_NC;
	else
	{
		logic = arg.toInt(); //maybe we've passed in an integer value?
		if ( !logic ) //check
		{
			//error here
			logic = LOGIC_NO;
		}
	}
	
	return logic;
}

bool PLC_Main::setClaimedPin(uint8_t pin)
{
	std::map<uint8_t, uint8_t>::iterator pinitr = pinMap.find(pin);
	if (pinitr != pinMap.end())
	{
		pinitr->second = PIN_TAKEN;
		return true;
	}
	return false; //shouldn't happen
}

void PLC_Main::sendError( uint8_t err, const String &info )
{
	String error;
	switch (err)
	{
		case ERR_DATA::ERR_CREATION_FAILED: //generic
		{
			error = err_failed_creation;
		}
		break;
		case ERR_DATA::ERR_UNKNOWN_TYPE:
		{
			error = err_failed_creation + CHAR_SPACE + err_unknown_type;
		}
		break;
		case ERR_DATA::ERR_INSUFFICIENT_ARGS:
		{
			error = err_failed_creation + CHAR_SPACE + err_insufficient_args;
		}
		break;
		case ERR_DATA::ERR_UNKNOWN_ARGS:
		{
			error = err_failed_creation + CHAR_SPACE + err_unknown_args;
		}
		break;
		case ERR_DATA::ERR_INVALID_BIT:
		{
			error = err_failed_creation + CHAR_SPACE + err_invalid_bit;
		}
		break;
		case ERR_DATA::ERR_INVALID_OBJ:
		{
			error = err_failed_creation + CHAR_SPACE + err_unknown_obj;
		}
		break;
		case ERR_DATA::ERR_PIN_INVALID:
		{
			error = err_pin_invalid;
		}
		break;
		case ERR_DATA::ERR_PIN_TAKEN:
		{
			error = err_pin_taken;
		}
		break;
		case ERR_DATA::ERR_PARSER_FAILED:
		{
			error = err_parser_failed;
		}
		break;
	}

	if ( info.length() )
		Core.sendMessage(error + CHAR_SPACE + info, PRIORITY_HIGH);
	else
		Core.sendMessage(error, PRIORITY_HIGH);

	resetAll(); //Since we have hit an error, just purge all objects from the PLC script. Since it can't be used anyway.
}