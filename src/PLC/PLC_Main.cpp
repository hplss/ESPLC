/*
 * PLC_Main.cpp
 *
 * Created: 9/28/2019 5:35:08 PM
 *  Author: Andrew Ward
 */ 
#include "PLC_Main.h"
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
/*
So basically we need to establish a way to define the objects that are present in the script. This includes all inputs/outputs/timers/counters etc. This also initializes these objects with the default 
arguments that dictate the logic calculations they use. 
Defining Syntax Ex: NAME=TYPE(ARGS)

Logic Syntax Ex: IN1 + IN2 = OUT

If an object is referenced without being initialized, fall out with an error

Probably going to need a struct that stores an object's name, as well as the assigned ID. also maybe the pointer to that object after it's created? (necessary? .. hmm)
*/
//TODO: For NOT (/) objects and .BIT arguments, a dummy object needs to be created. .BIT access individual variables in the object, and / gives an inverted state value.
bool PLC_Main::parseScript(const char *script)
{
	resetAll(); //Purge all previous ladder logic objects before applying new script.
	  
	String scriptLine; //container for parsed characters
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
			if ( !parseLine(scriptLine) )
			{
				parsedLadderObjects.clear();
				return false; //error ocurred somewhere?
			}

			scriptLine.clear(); //empty the container for the next line
		}
	}

	parsedLadderObjects.clear(); //empty the parsed objects vector once we're done.
	return true; //success
}
bool PLC_Main::parseLine( const String &scriptLine )
{
	shared_ptr<parserHelperObject> helper = make_shared<parserHelperObject>( scriptLine ); //create an instance of our parser helper
	
	for (helper->getLinePos() = 0; helper->getLinePos() < helper->getLineLength(); helper->getLinePos()++)
	{ 
		switch (helper->getCurrentLineChar() )
		{
			case CHAR_P_START: //for nested objects
			{
			}
			break;
			case CHAR_P_END: //also for nested objects (not implemented yet)
			{
			}
			break;
			case CHAR_AND: //We've got an AND operator
			{
				#ifdef DEBUG
				Serial.println(PSTR("AND OP"));
				#endif
				if ( !parser_ANDOP( helper ) ) //perform AND OP computations
					return false;
			}
			break;
			case CHAR_OR: //we've got an OR operator
			{
				#ifdef DEBUG 
				Serial.println("OR OP");
				#endif 
				if ( !parser_OROP( helper ) ) //perform OP OP logic computations
					return false;
			}
			break;
			case CHAR_EQUALS: //we've got an = operator (for outputs or object creation)
			{
				#ifdef DEBUG
				Serial.println(PSTR("EQ OP"));
				#endif
				if (!parser_EQOP( helper ))
					return false;
			}
			break;
			case CHAR_VAR_OPERATOR: // For Bit operators for existing objects. Example: Timer1.DN, where DN is the DONE bit for the timer.
				helper->setBitOP(true); //chars parsed are now an operator to an object variable (or bit tag)
			break;
			case CHAR_NOT_OPERATOR: //Indicates NOT logic
			{
				helper->setNotOP( !helper.get()->getNotOP() ); //invert from previous
			}
			break;
			default: //standard case, which simply entails appending the current character in the line to our temporary string used in the parser.
			{
				if (!parser_Default( helper ))
					return false;
			}
			break;
		}
	}

	if ( addLadderRung(helper->getRung()) )
	{
		#ifdef DEBUG
		Serial.print(PSTR("Rung Created. Objects: "));
		Serial.println(helper->getRung()->getNumRungObjects());
		#endif
	}
	return true; //success
}
bool PLC_Main::parser_Default( shared_ptr<parserHelperObject> helper )
{
	if ( !helper->getBitOP() )
		helper->appendToObjName( helper->getCurrentLineChar()); //looks like we're building the name of an object so far
	else //nope, it's a bit, so build the bit ID tag string instead.
		helper->appendToBitName( helper->getCurrentLineChar());
		
	if (helper->getLinePos() == helper->getLineLength()-1) //We've reached the end (not including the terminating char, etc). Make sure to handle the logic for the last parsed arg.
	{
		shared_ptr<Ladder_OBJ_Wrapper> newOBJWrapper = helper->createNewWrapper(findLadderObjByName(helper->getParsedObjectStr()));
		if ( newOBJWrapper ) //an object exists? 
		{
			for ( uint8_t x = 0; x < helper->getNumPreviousObjects(); x++ )
			{
				if ( !helper->getPreviousObjects()[x]->addNextObject(getNumRungs(), newOBJWrapper) )
					return false;
			}
		} 
		else //We're at the end, and we can't find an object based on the string that has been parsed from the remaining characters?
		{
			sendError(ERR_DATA::ERR_INVALID_OBJ, helper->getParsedObjectStr());
			return false;
		}
	}
	return true;
}
bool PLC_Main::parser_ANDOP( shared_ptr<parserHelperObject> helper )
{
	shared_ptr<Ladder_OBJ_Wrapper> newOBJWrapper = helper->createNewWrapper(findLadderObjByName( helper->getParsedObjectStr()));
	//Find and generate ladder object wrapper based on existing object, then get operator instructions for object
	if ( newOBJWrapper )
	{
		if ( helper->getLastOP() == CHAR_OR )
			helper->orToPreviousObjects(); //OR Objects already found are now the previous objects for the next (future) parsed object(s). Advance on AND OP
		
		if (helper->getNumPreviousObjects()) //we have at least one previously parsed and initialized object
		{
			for ( uint8_t x = 0; x < helper->getNumPreviousObjects(); x++ )
			{
				if (!helper->getPreviousObjects()[x]->addNextObject(getNumRungs(), newOBJWrapper)) //set this current object as the one that is processed next (inline) by the previous object
					return false;
			}
			
			helper->setPreviousObject(newOBJWrapper); //explicitly set the inputted object as the previous object (clears others)
				
			helper->getOrObjects().clear(); //Also clear previous or objects
		}
		else //nothing in the old Objects list at this step.. indicates the very beginning of the rung logic
		{
			helper->addPreviousObject(newOBJWrapper);
			if (!helper->getRung()->addInitialRungObject(newOBJWrapper)) //needs a device configuration
				return false;
		}
	}
	else
	{
		sendError(ERR_DATA::ERR_INVALID_OBJ, helper->getParsedObjectStr());
		return false;
	}

	helper->reset();
	helper->setLastOP( CHAR_AND );
	return true; //success
}

bool PLC_Main::parser_OROP( shared_ptr<parserHelperObject> helper )
{
	shared_ptr<Ladder_OBJ_Wrapper> newOBJWrapper = helper->createNewWrapper(findLadderObjByName( helper->getParsedObjectStr()));
	if ( newOBJWrapper ) //if it exists, perform some configurations and organization for the wrapper.
	{
		if (helper->getNumPreviousObjects()) //we have some objects that we've previously parsed and processed
		{
			for ( uint8_t x = 0; x < helper->getNumPreviousObjects(); x++ )
			{
				if (!helper->getPreviousObjects()[x]->addNextObject(getNumRungs(), newOBJWrapper)) //set this new object as the one that is processed next for the previous
					return false;
			}
		}
		else //only update if we haven't set an old object yet
		{
			if ( !helper->getRung()->addInitialRungObject(newOBJWrapper) )
				return false;
		}
		helper->addOrObject(newOBJWrapper); //save as an or object (not a "previous" object yet)
	}	
	else
	{
		sendError(ERR_DATA::ERR_INVALID_OBJ, helper->getParsedObjectStr());
		return false;
	}

	helper->reset();
	helper->setLastOP(CHAR_OR);
	return true; //success
}

bool PLC_Main::parser_EQOP( shared_ptr<parserHelperObject> helper )
{
	shared_ptr<Ladder_OBJ_Wrapper> newOBJWrapper = helper->createNewWrapper(findLadderObjByName(helper->getParsedObjectStr()));
	if ( newOBJWrapper ) //looks like it exists, treat as an output
	{
		if ( helper->getLastOP() == CHAR_OR )
		{
			if ( !helper->getNumPreviousObjects() ) //no previously parsed objects stored?
			{
				if ( !helper->getRung()->addInitialRungObject(newOBJWrapper) ) //must be an initial object in the rung.
					return false;
			}

			helper->orToPreviousObjects(); //merge or objects into previous objects list, and clear the OR list.
			helper->addPreviousObject(newOBJWrapper);
		}
		else
		{
			for ( uint8_t x = 0; x < helper->getNumPreviousObjects(); x++ )
			{
				if (!helper->getPreviousObjects()[x]->addNextObject(getNumRungs(), newOBJWrapper)) //The previously parsed objects have the current parsed object added as the next logic step
					return false;
			}
			
			if ( !helper->getRung()->getNumInitialRungObjects() )
			{
				if ( !helper->getRung()->addInitialRungObject(newOBJWrapper))
					return false;
			}
				
			helper->setPreviousObject(newOBJWrapper);
		}
		helper->setLastOP( CHAR_EQUALS);
	}
	else  //doesn't exist, so create it and add to the list of parsed objects
	{
		shared_ptr<ladderOBJdata> newObj = createNewObject(helper);
		if ( newObj )
			parsedLadderObjects.emplace_back(newObj); //place the newly reated object into the list of parsed (and created) ladder objects.
		else
			return false;
	}
		
	helper->reset();
	return true; //success
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

shared_ptr<ladderOBJdata> PLC_Main::createNewObject( shared_ptr<parserHelperObject> &helper )
{
	String parsed;
	vector<String> ObjArgs; //first arg ([0]) should always be the object type identifier. 
	helper->getLinePos()++; //Skip the operator (=)
	for ( ;helper->getLinePos() < helper->getParsedLineStr().length(); helper->getLinePos()++ ) //At this point, we're parsing the arguments within the () of an object. IE: OUTPUT( PIN )
	{
		switch(helper->getParsedLineStr()[helper->getLinePos()])
		{
			case CHAR_P_START:	
			case CHAR_P_END:
			case CHAR_COMMA:
			{
				#ifdef DEBUG
				Serial.println(parsed);
				#endif
				ObjArgs.push_back(parsed); 
				parsed.clear();
				continue;
			}
			break;
			default:
			{
				parsed += helper->getParsedLineStr()[helper->getLinePos()];
			}
		}
	}
	
	if (ObjArgs.size()) //must have at least one arg (first indictes the object type)
	{
		if ( ObjArgs[0] == virtualTag1 || ObjArgs[0] == virtualTag2 ) //Is this a variable type object? (Used for local data storage in memory, to facilitate communication between objects)
		{
			#ifdef DEBUG
			Serial.println(PSTR("NEW VIRTUAL"));
			#endif
			if ( ObjArgs.size() > 1 )
				return make_shared<ladderOBJdata>( helper->getParsedObjectStr(), parseVAR(ObjArgs[1], currentObjID) ); //WIP
		}
		else if ( ObjArgs[0] == inputTag1 || ObjArgs[0] == inputTag2 )
		{
			return make_shared<ladderOBJdata>(helper->getParsedObjectStr(), createInputOBJ(ObjArgs));
		}
		else if ( ObjArgs[0] == outputTag1 || ObjArgs[0] == outputTag2 ) 
		{
			return make_shared<ladderOBJdata>(helper->getParsedObjectStr(), createOutputOBJ(ObjArgs));
		}
		else if ( ObjArgs[0] == timerTag1 || ObjArgs[0] == timerTag2 ) 
		{
			return make_shared<ladderOBJdata>(helper->getParsedObjectStr(), createTimerOBJ(ObjArgs));
		}
		else if ( ObjArgs[0] == counterTag1 || ObjArgs[0] == counterTag2 ) 
		{
			return make_shared<ladderOBJdata>(helper->getParsedObjectStr(), createCounterOBJ(ObjArgs));
		}
		else //if we don't find a valid object type
			sendError(ERR_DATA::ERR_UNKNOWN_TYPE, ObjArgs[0]);
	}
	else //not enough arguments to create a new ladder object
		sendError(ERR_DATA::ERR_INSUFFICIENT_ARGS);
	
	return 0;
}

shared_ptr<Ladder_OBJ> PLC_Main::createInputOBJ( const vector<String> &args )
{
	#ifdef DEBUG
	Serial.println(PSTR("NEW INPUT"));
	#endif
	uint8_t pin = 0, logic = LOGIC_NO, type = TYPE_INPUT;
	if ( args.size() > 3 )
		logic = parseLogic(args[3]);

	if ( args.size() > 2 )
	{
		if ( args[2] == "A" || args[2] == typeTagAnalog ) //Explicitly declare an analog input, otherwise assume it's a digital in only
			type = TYPE_INPUT_ANALOG;
		else if (args[2] == "D" || args[2] == typeTagDigital )
			type = TYPE_INPUT;
	}
	
	if ( args.size() > 1 ) //needed
	{
		pin = args[1].toInt(); 
		if ( isValidPin(pin, type) )
		{
			shared_ptr<InputOBJ> newObj(new InputOBJ(currentObjID++, pin, type, logic));
			ladderObjects.emplace_back(newObj); //add to the list of global shared pointers for later reference.
			setClaimedPin(pin); //set the pin as claimed for this object.
			return newObj;
		}
	}

	return 0;
}

shared_ptr<Ladder_OBJ> PLC_Main::createOutputOBJ( const vector<String> &args )
{
	#ifdef DEBUG
	Serial.println(PSTR("NEW OUTPUT"));
	#endif
	uint8_t pin = 0, logic = LOGIC_NO;
	if ( args.size() > 2 ) //caveat here is that outputs would NEVER normally be normally closed (ON)
		logic = parseLogic(args[2]);
		
	if ( args.size() > 1 )
	{
		pin = args[1].toInt();
		if ( isValidPin(pin, TYPE_OUTPUT) )
		{
			shared_ptr<OutputOBJ> newObj(new OutputOBJ(currentObjID++, pin, logic));
			ladderObjects.emplace_back(newObj);
			setClaimedPin( pin ); //claim the pin for this object.
			return newObj;
		}
	}

	return 0;
}
shared_ptr<Ladder_OBJ> PLC_Main::createTimerOBJ( const vector<String> &args )
{
	#ifdef DEBUG
	Serial.println(PSTR("NEW TIMER"));
	#endif
	uint32_t delay = 0, accum = 0, subType = TYPE_TON;
	if ( args.size() > 3 )
	{	  
		if ( args[3] == typeTagTOF )
			subType = TYPE_TOF;
		else if ( args[3] == typeTagTON )
			subType = TYPE_TON;
		else
			sendError(ERR_DATA::ERR_UNKNOWN_ARGS, args[0] + args[3]); 
	}
	if ( args.size() > 2 )
		accum = args[2].toInt(); //some output verification tests here

	if ( args.size() > 1 ) //must have at least one arg
	{
		delay = args[1].toInt();
		if (delay > 0) //Must have a valid delay time. 
		{
			shared_ptr<TimerOBJ> newObj(new TimerOBJ(currentObjID++, delay, accum, subType ));
			ladderObjects.emplace_back(newObj);
			return newObj;
		}
	}

	return 0;
}
shared_ptr<Ladder_OBJ> PLC_Main::createCounterOBJ( const vector<String> &args )
{
	#ifdef DEBUG
	Serial.println(PSTR("NEW COUNTER"));
	#endif
	uint16_t count = 0, accum = 0; 
	uint8_t subType = TYPE_CTU; 
	if ( args.size() > 3 )
	{
		subType = TYPE_CTU; //default
		if ( args[3] == typeTagCTU )
			subType = TYPE_CTU;
		else if ( args[3] == typeTagCTD )
			subType = TYPE_CTD;
		else
			sendError(ERR_DATA::ERR_UNKNOWN_ARGS, args[0] + args[3]);
	}
	if ( args.size() > 2 )
		accum = args[2].toInt(); //some output verification tests here?
		
	if ( args.size() > 1 )
	{
		count = args[1].toInt();
		shared_ptr<CounterOBJ> newObj(new CounterOBJ(currentObjID++, count, accum, subType));
		ladderObjects.emplace_back(newObj);
		return newObj;
	}
	return 0;
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

//TODO - Implement some way where a user can easily dictate which variable type to use for memory purposes, otherwise default to auto-detection (maybe always estimate high? 64-bit?).
shared_ptr<Ladder_VAR> PLC_Main::parseVAR( const String &arg, uint16_t &id )
{
	bool isFloat = false;
	String val;
	for (uint8_t x = 0; x < arg.length(); x++ )
	{
		if (x > 18) //this is plenty of digits, more than will ever likely be needed
			break;
			
		if ( (arg[x] >= '0' && arg[x] <= '9') || arg[x] == CHAR_VAR_OPERATOR)
		{
			if ( arg[x] == CHAR_VAR_OPERATOR )
			{
				if ( isFloat )// multiple '.' chars should be ignored
					continue;
					
				isFloat = true;
			}
			
			val += arg[x];
		}
	}
	
	if ( !val.length() )
		return 0;
		
	if (isFloat)
		return make_shared<Ladder_VAR>( val.toFloat(), id++ );
	
	int64_t value = atoll(val.c_str());
	if ( value < 0 )
		return make_shared<Ladder_VAR>( static_cast<int_fast32_t>(value), id++ ); //must be signed
	else if (value <= INT_MAX )
		return make_shared<Ladder_VAR>( static_cast<uint_fast32_t>(value), id++ ); //we can use unsigned
	else
		return make_shared<Ladder_VAR>( static_cast<uint64_t>(value), id++ ); //assume a long (geater than 32 bits)
		
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
		case ERR_DATA::ERR_INVALID_BIT:
		{
			error = err_failed_creation + CHAR_SPACE + err_invalid_bit;
		}
		case ERR_DATA::ERR_INVALID_OBJ:
		{
			error = err_failed_creation + CHAR_SPACE + err_unknown_obj;
		}
		case ERR_DATA::ERR_PIN_INVALID:
		{
			error = err_pin_invalid;
		}
		case ERR_DATA::ERR_PIN_TAKEN:
		{
			error = err_pin_taken;
		}
		break;
	}

	if ( info.length() )
		Core.sendMessage(error + CHAR_SPACE + info, PRIORITY_HIGH);
	else
		Core.sendMessage(error, PRIORITY_HIGH);

	resetAll(); //Since we have hit an error, just purge all objects from the PLC script. Since it can't be used anyway.
}