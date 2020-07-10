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
	ladderRungs.clear(); //Empty the vector
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
			parseLine(scriptLine);
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
			case CHAR_P_START:
			{
			}
			break;
			case CHAR_P_END:
			{
			}
			break;
			case CHAR_AND: //We've got an AND operator
			{
				#ifdef DEBUG
				Serial.println(PSTR("AND OP"));
				#endif
				parser_ANDOP( helper ); //perform AND OP computations
			}
			break;
			case CHAR_OR: //we've got an OR operator
			{
				#ifdef DEBUG 
				Serial.println("OR OP");
				#endif 
				parser_OROP( helper ); //perform OP OP logic computations
			}
			break;
			case CHAR_EQUALS: //we've got an = operator (for outputs or object creation)
			{
				#ifdef DEBUG
				Serial.println(PSTR("EQ OP"));
				#endif
				parser_EQOP( helper );
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
				parser_Default( helper );
			}
			break;
		}
	}

	if ( addLadderRung(helper->getRung()) )
	{
		#ifdef DEBUG
		Serial.println(PSTR("Rung Created. Objects: "));
		Serial.println(helper->getRung()->getNumRungObjects());
		#endif
	}
	return true; //success
}
void PLC_Main::parser_Default( shared_ptr<parserHelperObject> helper )
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
				helper->getPreviousObjects()[x]->addNextObject(getNumRungs(), newOBJWrapper);
		} 
		else //We're at the end, and we can't find an object based on the string that has been parsed from the remaining characters?
			sendError(ERR_DATA::INVALID_OBJ, helper->getParsedObjectStr());
	}
}
void PLC_Main::parser_ANDOP( shared_ptr<parserHelperObject> helper )
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
				helper->getPreviousObjects()[x]->addNextObject(getNumRungs(), newOBJWrapper); //set this current object as the one that is processed next (inline) by the previous object
			
			helper->setPreviousObject(newOBJWrapper); //explicitly set the inputted object as the previous object (clears others)
				
			helper->getOrObjects().clear(); //Also clear previous or objects
		}
		else //nothing in the old Objects list at this step.. indicates the very beginning of the rung logic
		{
			helper->addPreviousObject(newOBJWrapper);
			helper->getRung()->addInitialRungObject(newOBJWrapper); //needs a device configuration
		}
	}
	else
		sendError(ERR_DATA::INVALID_OBJ, helper->getParsedObjectStr());

	helper->reset();
	helper->setLastOP( CHAR_AND );
}

void PLC_Main::parser_OROP( shared_ptr<parserHelperObject> helper )
{
	shared_ptr<Ladder_OBJ_Wrapper> newOBJWrapper = helper->createNewWrapper(findLadderObjByName( helper->getParsedObjectStr()));
	if ( newOBJWrapper ) //if it exists, perform some configurations and organization for the wrapper.
	{
		if (helper->getNumPreviousObjects()) //we have some objects that we've previously parsed and processed
		{
			for ( uint8_t x = 0; x < helper->getNumPreviousObjects(); x++ )
				helper->getPreviousObjects()[x]->addNextObject(getNumRungs(), newOBJWrapper); //set this new object as the one that is processed next for the previous
		}
		else //only update if we haven't set an old object yet
		{
			helper->getRung()->addInitialRungObject(newOBJWrapper);
		}
		helper->addOrObject(newOBJWrapper); //save as an or object (not a "previous" object yet)
	}	
	else
	{
		sendError(ERR_DATA::INVALID_OBJ, helper->getParsedObjectStr());
	}

	helper->reset();
	helper->setLastOP(CHAR_OR);
}

void PLC_Main::parser_EQOP( shared_ptr<parserHelperObject> helper )
{
	shared_ptr<Ladder_OBJ_Wrapper> newOBJWrapper = helper->createNewWrapper(findLadderObjByName(helper->getParsedObjectStr()));
	if ( newOBJWrapper ) //looks like it exists, treat as an output
	{
		if ( helper->getLastOP() == CHAR_OR )
		{
			if ( !helper->getNumPreviousObjects() ) //no previously parsed objects stored?
				helper->getRung()->addInitialRungObject(newOBJWrapper); //must be an initial object in the rung.

			helper->orToPreviousObjects(); //merge or objects into previous objects list, and clear the OR list.
			helper->addPreviousObject(newOBJWrapper);
		}
		else
		{
			for ( uint8_t x = 0; x < helper->getNumPreviousObjects(); x++ )
				helper->getPreviousObjects()[x]->addNextObject(getNumRungs(), newOBJWrapper); //The previously parsed objects have the current parsed object added as the next logic step
			
			if ( !helper->getRung()->getNumInitialRungObjects() )
				helper->getRung()->addInitialRungObject(newOBJWrapper);
				
			helper->setPreviousObject(newOBJWrapper);
		}
		helper->setLastOP( CHAR_EQUALS);
	}
	else  //doesn't exist, so create it and add to the list of parsed objects
		parsedLadderObjects.emplace_back(createNewObject( helper->getParsedObjectStr(), helper->getParsedLineStr(), helper->getLinePos() )); //pass on the line we're parsing (contains the args), as well as the current position in the string.
		
	helper->reset();
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

shared_ptr<Ladder_OBJ_Wrapper> PLC_Main::findLadderObjByID( const uint16_t id ) //Search through all created rungs thus far. This assumes that the object was created successfully.
{
	for ( uint16_t x = 0; x < ladderRungs.size(); x++ )
	{
		for ( uint16_t y = 0; y < ladderRungs[x]->getRungObjects().size(); y++ )
		{
			shared_ptr<Ladder_OBJ_Wrapper>pObj = ladderRungs[x]->getRungObjects()[y];
			if ( pObj && pObj->getObject()->getID() == id )
				return pObj;
		}
	}
	
	return 0;
}

shared_ptr<Ladder_OBJ> PLC_Main::findLadderObjByName( const String &name )
{
	for ( uint16_t x = 0; x < parsedLadderObjects.size(); x++ )
	{
		if ( parsedLadderObjects[x]->getName() == name)
			return parsedLadderObjects[x]->getObject();
	}
	
	return 0;
}

shared_ptr<ladderOBJdata> PLC_Main::createNewObject( const String &name, const String &args, uint16_t &pos)
{
	String parsed;
	vector<String> ObjArgs; //first arg should always be the object type. 
	pos++; //Skip the operator (=)
	for ( ;pos < args.length(); pos++ ) //At this point, we're parsing the arguments within the () of an object. IE: OUTPUT( PIN )
	{
		switch(args[pos])
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
				parsed += args[pos];
			}
		}
	}
	
	if (ObjArgs.size()) //must have some valid number of args available
	{
		if ( ObjArgs[0] == variableTag ) //Is this a variable type object? (Used for local data storage in memory, to facilitate communication between objects)
		{
			#ifdef DEBUG
			Serial.println(PSTR("NEW VARIABLE"));
			#endif
			if ( ObjArgs.size() > 1 )
				return make_shared<ladderOBJdata>( name, parseVAR(ObjArgs[1], currentObjID) );
		}
		else if ( ObjArgs[0] == inputTag1 || ObjArgs[0] == inputTag2 )//[1] = input pin, [2] = logic
		{
			#ifdef DEBUG
			Serial.println(PSTR("NEW INPUT"));
			#endif
			uint8_t pin = 0, logic = LOGIC_NO;
			if ( ObjArgs.size() > 2 ) //needed
				logic = parseLogic(ObjArgs[2]);
			
			if ( ObjArgs.size() > 1 ) //needed
			{
				pin = ObjArgs[1].toInt(); //could be an invalid pin
				return make_shared<ladderOBJdata>( name, make_shared<InputOBJ>(currentObjID++, pin, logic) );
			}
		}
		else if ( ObjArgs[0] == outputTag1 || ObjArgs[0] == outputTag2 ) //[1] = output pin, [2] = NO/NC
		{
			#ifdef DEBUG
			Serial.println(PSTR("NEW OUTPUT"));
			#endif
			uint8_t pin = 0, logic = LOGIC_NO;
			if ( ObjArgs.size() > 2 ) //caveat here is that outputs would NEVER normally be normally closed (ON)
				logic = parseLogic(ObjArgs[2]);
				
			if ( ObjArgs.size() > 1 )
			{
				pin = ObjArgs[1].toInt();
				return make_shared<ladderOBJdata>( name, make_shared<OutputOBJ>(currentObjID++, pin, logic) );
			}
		}
		else if ( ObjArgs[0] == timerTag1 || ObjArgs[0] == timerTag2 ) //[1] = delay(ms), [2]= accum default(ms), [3] = subtype(TON/TOF)
		{
			#ifdef DEBUG
			Serial.println(PSTR("NEW TIMER"));
			#endif
			uint32_t delay = 0, accum = 0, subType = TYPE_TON;
			if ( ObjArgs.size() > 3 )
			{	  
				if ( ObjArgs[3] == typeTagTOF )
					subType = TYPE_TOF;
				else if ( ObjArgs[3] == typeTagTON )
					subType = TYPE_TON;
				else
					sendError(ERR_DATA::UNKNOWN_ARGS, ObjArgs[0] + ObjArgs[3]);
			}
			if ( ObjArgs.size() > 2 )
				accum = ObjArgs[2].toInt(); //some output verification tests here

			if ( ObjArgs.size() > 1 ) //must have at least one arg
			{
				delay = ObjArgs[1].toInt();
				return make_shared<ladderOBJdata>( name, make_shared<TimerOBJ>(currentObjID++, delay, accum, subType) );
			}
		}
		else if ( ObjArgs[0] == counterTag1 || ObjArgs[0] == counterTag2 ) //[1] = count value, [2] = accum, [3] = subtype(CTU/CTD)
		{
			#ifdef DEBUG
			Serial.println(PSTR("NEW COUNTER"));
			#endif
			uint16_t count = 0, accum = 0; 
			uint8_t subType = TYPE_CTU; 
			if ( ObjArgs.size() > 3 )
			{
				subType = TYPE_CTU; //default
				if ( ObjArgs[3] == typeTagCTU )
					subType = TYPE_CTU;
				else if ( ObjArgs[3] == typeTagCTD )
					subType = TYPE_CTD;
				else
					sendError(ERR_DATA::UNKNOWN_ARGS, ObjArgs[0] + ObjArgs[3]);
			}
			if ( ObjArgs.size() > 2 )
				accum = ObjArgs[2].toInt(); //some output verification tests here
				
			if ( ObjArgs.size() > 1 )
			{
				count = ObjArgs[1].toInt();
				return make_shared<ladderOBJdata>( name, make_shared<CounterOBJ>(currentObjID++, count, accum, subType) );
			}
		}
		else if ( ObjArgs[0] == virtualTag1 || ObjArgs[0] == virtualTag2 ) //[1] = logic(NO/NC)
		{
			//not yet
		}
		else //if we don't find a valid object type
			sendError(ERR_DATA::UNKNOWN_TYPE, ObjArgs[0]);
	}
	else //not enough arguments to create a new ladder object
		sendError(ERR_DATA::INSUFFICIENT_ARGS);
	
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
		
	/*if (isFloat)
		return make_shared<Ladder_VAR>( val.toFloat(), id++ );
	
	int64_t value = atoll(val.c_str());
	if ( value < 0 )
		return make_shared<Ladder_VAR>( static_cast<int_fast32_t>(value), id++ ); //must be signed
	else if (value <= INT_MAX )
		return make_shared<Ladder_VAR>( static_cast<uint_fast32_t>(value), id++ ); //we can use unsigned
	else
		return make_shared<Ladder_VAR>( static_cast<uint64_t>(value), id++ ); //assume a long (geater than 32 bits)
		*/
	return 0;
}

void PLC_Main::sendError( uint8_t err, const String &info )
{
	String error;
	switch (err)
	{
		case ERR_DATA::CREATION_FAILED:
		{
			error = err_failed_creation;
		}
		break;
		case ERR_DATA::UNKNOWN_TYPE:
		{
			error = err_failed_creation + CHAR_SPACE + err_unknown_type;
		}
		break;
		case ERR_DATA::INSUFFICIENT_ARGS:
		{
			error = err_failed_creation + CHAR_SPACE + err_insufficient_args;
		}
		break;
		case ERR_DATA::UNKNOWN_ARGS:
		{
			error = err_failed_creation + CHAR_SPACE + err_unknown_args;
		}
		break;
	}

	if ( info.length() )
		Core.sendMessage(error + CHAR_SPACE + info, PRIORITY_HIGH);
	else
		Core.sendMessage(error, PRIORITY_HIGH);

	resetAll(); //Since we have hit an error, just purge all objects from the PLC script. Since it can't be used anyway.
}