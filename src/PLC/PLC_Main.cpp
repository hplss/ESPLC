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
	/*for ( uint16_t x = getNumRungs(); x > 0; x--)
	{
		if (getLadderRung(x))
			delete getLadderRung(x);
	}*/
	
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
	uint16_t currentobjID = 0; //Id number used to address objects
			  
	vector<shared_ptr<ladderOBJdata>> ladderObjects; //temporary container for information for all ladder objects created by the parser
	String scriptLine;
	
	for (uint16_t x = 0; x < strlen(script); x++) //go one char at a time.
	{
		if ( script[x] == CHAR_SPACE ) //omit spaces
			continue;
			
		if ( script[x] != CHAR_NEWLINE && script[x] != CHAR_CARRIAGE ) //Do this one line at a time.
			scriptLine += toUpper(script[x]); //convert all chars to upper case. This might be done elsewhere later (web interface code) but for now we'll do it here
			
		else if (scriptLine.length() > 1) //we've hit a newline or carriage return char and we've got a valid length
		{
			String parsedObj, //object name 
				   parsedBit; //for bit operations on objects

			shared_ptr<Ladder_Rung> pRung(new Ladder_Rung()); //create the new rung object
			vector<shared_ptr<ladderOBJdata>> pOldObjects, //indicate objects stored from the previous operator logic
											  orObjects;	//used as a container for chained or objects that are parsed
			bool nestedItem = false;
			bool bitOperator = false;
			bool bitNot = false;
				 
			uint8_t lastOP = CHAR_AND;
			uint16_t lineLength = scriptLine.length();
			
			for (uint16_t y = 0; y < lineLength; y++)
			{ 
				switch (scriptLine[y])
				{
					case CHAR_P_START:
					{
						nestedItem = true;
					}
					break;
					case CHAR_P_END:
					{
						nestedItem = false; //basically if we're working within a nest. The previous objects go unchanged.
					}
					break;
					case CHAR_AND: //We've got an and operator
					{
						shared_ptr<ladderOBJdata> pNewObj = findLadderObjByName(ladderObjects, getObjName( parsedObj ));
						//Find and generate ladder object wrapper based on existing object, then get operator instructions for object
						if ( pNewObj && pNewObj->getType() == LADDER_OBJECT )
						{
							#ifdef DEBUG
							Serial.println(PSTR("AND OP"));
							#endif
							//generateObjectConfig
							pRung->addRungObject(pNewObj->getObject()); //Add to the rung
							if ( lastOP == CHAR_OR )
								pOldObjects = orObjects; //Objects found are now the old for the next parsed object(s). Advance on AND
							
							if (pOldObjects.size()) //we've found the object
							{
								for ( uint8_t x = 0; x < pOldObjects.size(); x++ )
									pOldObjects[x]->addNextObject(getNumRungs(), pNewObj); //set this new object as the one that is processed next for the previous
								
								pOldObjects.clear(); //empty
								pOldObjects.emplace_back(pNewObj);
									
								orObjects.clear();//Also clear previous or objects
							}
							else //nothing in the old Objects list at this step.. indicates the very beginning of the script
							{
								pOldObjects.emplace_back(pNewObj);
								pRung->addInitialRungObject(pNewObj->getObject()); //needs a device configuration
							}
						}
						else
							sendError(ERR_DATA::INVALID_OBJ, parsedObj);

						parsedObj.clear();
						bitOperator = false;
						lastOP = CHAR_AND;
					}
					break;
					case CHAR_OR: //we've got an or operator
					{
						shared_ptr<ladderOBJdata> pNewObj = findLadderObjByName(ladderObjects, parsedObj);
						if ( pNewObj && pNewObj->getType() == LADDER_OBJECT )
						{
							#ifdef DEBUG 
							Serial.println("OR OP");
							#endif 
							pRung->addRungObject(pNewObj->getObject()); //add to the rung
								
							if (pOldObjects.size()) //we have some objects that we've previously parsed and processed
							{
								for ( uint8_t x = 0; x < pOldObjects.size(); x++ )
									pOldObjects[x]->addNextObject(getNumRungs(), pNewObj); //set this new object as the one that is processed next for the previous
							}
							else //only update if we haven't set an old object yet
								pRung->addInitialRungObject(pNewObj->getObject());
								
							orObjects.emplace_back(pNewObj); //save as an or object (not an old object yet)
						}	
						else
							sendError(ERR_DATA::INVALID_OBJ, parsedObj);

						parsedObj.clear();
						bitOperator = false;
						lastOP = CHAR_OR;
					}
					break;
					case CHAR_EQUALS: //we've got an = operator (for outputs or object creation)
					{
						#ifdef DEBUG
						Serial.println(PSTR("EQ OP"));
						#endif
						shared_ptr<ladderOBJdata> pObj = findLadderObjByName(ladderObjects, parsedObj);
						if (pObj && pObj->getType() == LADDER_OBJECT ) //looks like it exists, treat as an output
						{
							pRung->addRungObject(pObj->getObject()); //Add to the new rung
							if ( lastOP == CHAR_OR )
							{
								if ( !pOldObjects.size() )
									pRung->addInitialRungObject(pObj->getObject());

								pOldObjects = orObjects; //combine
								pOldObjects.emplace_back(pObj); //add current object to existing if previous statement was an OR
							}
							else
							{
								for ( uint8_t x = 0; x < pOldObjects.size(); x++ )
									pOldObjects[x]->addNextObject(getNumRungs(), pObj); //The previously parsed objects have the current parsed object added as the next logic step
								
								if ( !pRung->getNumInitialRungObjects() )
									pRung->addInitialRungObject(pObj->getObject());
									
								pOldObjects.clear(); //empty container
								pOldObjects.emplace_back(pObj);
							}
							lastOP = CHAR_EQUALS;
						}
						else if (!pObj) //doesn't exist, so create it
							ladderObjects.emplace_back(createNewObject( parsedObj, scriptLine, y, currentobjID )); //pass on the line we're parsing (contains the args), as well as the current position in the string.
							
						parsedObj.clear();
						bitOperator = false;
					}
					break;
					case CHAR_VAR_OPERATOR:
						bitOperator = true; //chars parsed are now an operator to an object variable
					break;
					case CHAR_NOT_OPERATOR:
					{
						bitNot = true;
					}
					break;
					default:
					{
						if ( !bitOperator )
							parsedObj += scriptLine[y]; //looks like we're building the name of an object so far
						else 
							parsedBit += scriptLine[y];
							
						if (y == lineLength-1) //We've reached the end. Make sure to handle the logic for the last parsed arg.
						{
							shared_ptr<ladderOBJdata> pObj = findLadderObjByName(ladderObjects, parsedObj);
							if ( pObj )
							{
								pRung->addRungObject(pObj->getObject()); //Add to the new rung
								for ( uint8_t x = 0; x < pOldObjects.size(); x++ )
									pOldObjects[x]->addNextObject(getNumRungs(), pObj);
							}
							else
								sendError(ERR_DATA::INVALID_OBJ, parsedObj);
						}
					}
					break;
				}
			}
			scriptLine.clear();
			if ( addLadderRung(pRung) )
			{
				#ifdef DEBUG
				Serial.println(PSTR("Rung Created. Objects: "));
				Serial.println(pRung->getNumRungObjects());
				#endif
			}
		}
	}
	return true; //success
}

String PLC_Main::getObjName( const String &parsedString )
{
	String objName;
	for ( uint8_t x =0; x < parsedString.length(); x++ )
	{
		if ( parsedString[x] == CHAR_VAR_OPERATOR ) //operator is always at end of name
			break;
		if ( parsedString[x] == CHAR_NOT_OPERATOR ) //not operator is always at beginning of name
			continue;
			
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

shared_ptr<ladderOBJdata> PLC_Main::findLadderObjByID( const vector<shared_ptr<ladderOBJdata>> &data, const uint16_t id ) 
{
	for ( uint16_t x = 0; x < data.size(); x++ )
	{
		shared_ptr<Ladder_OBJ> pObj = data[x]->getObject();
		if ( pObj->getID() == id)
			return data[x];
	}
	
	return 0;
}

shared_ptr<Ladder_OBJ> PLC_Main::findLadderObjByID( const uint16_t id ) //Search through all created rungs thus far. This assumes that the object was created successfully.
{
	for ( uint16_t x = 0; x < ladderRungs.size(); x++ )
	{
		for ( uint16_t y = 0; y < ladderRungs[x]->getRungObjects().size(); y++ )
		{
			shared_ptr<Ladder_OBJ>pObj = ladderRungs[x]->getRungObjects()[y];
			if ( pObj && pObj->getID() == id )
				return pObj;
		}
	}
	
	return 0;
}

shared_ptr<ladderOBJdata> PLC_Main::findLadderObjByName( const vector<shared_ptr<ladderOBJdata>> &data, const String &name )
{
	for ( uint16_t x = 0; x < data.size(); x++ )
	{
		if ( data[x]->getName() == name)
			return data[x];
	}
	
	return 0;
}

shared_ptr<ladderOBJdata> PLC_Main::createNewObject( const String &name, const String &args, uint16_t &pos, uint16_t &id)
{
	String parsed;
	vector<String> ObjArgs; //first arg should always be the type. 
	pos++; //Skip the operator (=)
	for ( ;pos < args.length(); pos++ ) //At this point, we're parsing the arguments within the () of an object. IE: OUTPUT( PIN )
	{
		switch(args[pos])
		{
			case CHAR_P_START:	
			case CHAR_P_END:
			case CHAR_COMMA:
			{
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
	
	if (ObjArgs.size())
	{
		if ( ObjArgs[0] == variableTag ) //Is this a variable type object? (Used for local data storage)
		{
			#ifdef DEBUG
			Serial.println(PSTR("NEW VARIABLE"));
			#endif
			if ( ObjArgs.size() > 1 )
				return make_shared<ladderOBJdata>( name, parseVAR(ObjArgs[1], id) );
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
				return make_shared<ladderOBJdata>( name, make_shared<InputOBJ>(id++, pin, logic) );
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
				return make_shared<ladderOBJdata>( name, make_shared<OutputOBJ>(id++, pin, logic) );
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
				return make_shared<ladderOBJdata>( name, make_shared<TimerOBJ>(id++, delay, accum, subType) );
			}
		}
		else if ( ObjArgs[0] == counterTag1 || ObjArgs[0] == counterTag2 ) //[1] = count value, [2] = accum, [3] = subtype(CTU/CTD)
		{
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
				return make_shared<ladderOBJdata>( name, make_shared<CounterOBJ>(id++, count, accum, subType) );
			}
		}
		else if ( ObjArgs[0] == virtualTag1 || ObjArgs[0] == virtualTag2 ) //[1] = logic(NO/NC)
		{
			//not yet
		}
		else
			sendError(ERR_DATA::UNKNOWN_TYPE, ObjArgs[0]);
	}
	else
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
		if (x > 18) //this is plenty of digits, more than likely needed
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
		return make_shared<Ladder_VAR>( id++, val.toFloat() );
	
	uint64_t value = atoll(val.c_str());
	if (value <= INT_MAX )
		return make_shared<Ladder_VAR>( id++, (int)value );
	else
		return make_shared<Ladder_VAR>( id++, value );
		
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