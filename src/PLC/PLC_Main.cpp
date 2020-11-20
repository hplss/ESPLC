/*
 * PLC_Main.cpp
 *
 * Created: 9/28/2019 5:35:08 PM
 *  Author: Andrew Ward
 */ 
#include "PLC_Main.h"
#include "PLC_Parser.h"
#include <HardwareSerial.h>

//other object includes
#include "OBJECTS/MATH/obj_math_basic.h"
#include "OBJECTS/obj_var.h"
#include "OBJECTS/obj_input_basic.h"
#include "OBJECTS/obj_output_basic.h"
#include "OBJECTS/obj_timer.h"
#include "OBJECTS/obj_counter.h"
#include "OBJECTS/obj_oneshot.h"
#include "ACCESSORS/acc_remote.h"
//

void PLC_Main::resetAll()
{
	ladderRungs.clear(); //Empty created ladder rungs vector
	ladderObjects.clear(); //Empty the created ladder logic objects vector
	accessorObjects.clear(); // Empty the accessor objects vector
	ladderVars.clear(); //Empty the created ladder vars vector
	generatePinMap(); //reset and fill the pinmap
	generatePWMMap(); //generate the list of available PWM channels for outputs
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
	pinMap.emplace(0, PIN_TYPE::PIN_O ); //WiFi dependent
	pinMap.emplace(1, PIN_TYPE::PIN_INVALID ); //TX
	pinMap.emplace(2, PIN_TYPE::PIN_AIO ); //WiFi dependent
	pinMap.emplace(3, PIN_TYPE::PIN_INVALID ); //RX
	pinMap.emplace(4, PIN_TYPE::PIN_AIO ); //WiFi dependent
	pinMap.emplace(5, PIN_TYPE::PIN_IO );

	for ( uint8_t x = 6; x < 11; x++ )
		pinMap.emplace(x, PIN_TYPE::PIN_INVALID );

	pinMap.emplace(12, PIN_TYPE::PIN_AIO ); //WiFi dependent
	pinMap.emplace(13, PIN_TYPE::PIN_AIO ); //WiFi dependent
	pinMap.emplace(14, PIN_TYPE::PIN_AIO ); //WiFi dependent
	pinMap.emplace(15, PIN_TYPE::PIN_AIO ); //WiFi dependent

	for ( uint8_t x = 16; x < 23; x++ )
	{
		if ( x == 20) //no GPIO 20
			continue; 

		pinMap.emplace(x, PIN_TYPE::PIN_IO );
	}

	pinMap.emplace(25, PIN_TYPE::PIN_AIO ); //WiFi dependent 
	pinMap.emplace(26, PIN_TYPE::PIN_AIO ); //WiFi dependent 
	pinMap.emplace(27, PIN_TYPE::PIN_AIO ); //WiFi dependent
	pinMap.emplace(32, PIN_TYPE::PIN_AIO );
	pinMap.emplace(33, PIN_TYPE::PIN_AIO );
	pinMap.emplace(34, PIN_TYPE::PIN_AI );
	pinMap.emplace(35, PIN_TYPE::PIN_AI );
	pinMap.emplace(36, PIN_TYPE::PIN_AI );
	pinMap.emplace(39, PIN_TYPE::PIN_AI );
}

void PLC_Main::generatePWMMap()
{
	pwmMap.clear(); //just in case
	for ( uint8_t x = 0; x < 15; x++ )
	{
		pwmMap.emplace(x, PWM_STATUS::PWM_AVAILABLE);
	}
}
int8_t PLC_Main::reservePWMChannel()
{
	for ( uint8_t x = 0; x < 15; x++ )
	{
		if ( pwmMap[x] == PWM_STATUS::PWM_AVAILABLE )
		{
			pwmMap[x] = PWM_STATUS::PWM_TAKEN; //reserve the channel
			return x;
		}
	}

	return -1; //default path indicates an error (cannot reserve)
}

String PLC_Main::getObjName( const String &parsedString )
{
	String objName;
	for ( uint8_t x = 0; x < parsedString.length(); x++ )
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
	//Update accessor objects first in the scan, as the state in some of the Ladder_OBJ_Logical objects they contain may be of use.
	for ( uint8_t x = 0; x < getNumAccessors(); x++ )
	{
		getAccessorObjects()[x]->updateObject();
	}
	//

	for (uint16_t x = 0; x < getNumRungs(); x++) //iterate through all available rungs
	{
		getLadderRungs()[x]->processRung(x); //perform logic 'scan' on the selected rung
	}

	if ( getRemoteServer() ) //handle the web server (if applicable)
	{
		getRemoteServer()->processRequests(); //handle any remote requests/etc.
	}
		
	//After the logic scans, the object's state is known. Perform the update on the objects (for some objects, this is the "action" function.)
	for ( uint16_t y = 0; y < ladderObjects.size(); y++ )
		ladderObjects[y]->updateObject(); 
}

bool PLC_Main::addLadderRung(shared_ptr<Ladder_Rung> rung)
{
	if ( !rung->getNumRungObjects() || !rung->getNumInitialRungObjects() ) //no objects in the rung?
	{
		return false; //error here? invalid number of necessary rung objects
	}
		
	//looks like we're good here	
	ladderRungs.emplace_back(rung);
	return true;
}

shared_ptr<Ladder_OBJ_Logical> PLC_Main::findLadderObjByID( const String &id ) //Search through all created objects thus far. This assumes that the object was created successfully.
{
	for ( uint16_t x = 0; x < ladderObjects.size(); x++ )
	{
		shared_ptr<Ladder_OBJ_Logical> pObj = ladderObjects[x];
		if ( pObj && pObj->getID() == id )
			return pObj;
	}
	
	return 0; //default
}

shared_ptr<Ladder_OBJ_Accessor> PLC_Main::findAccessorByID( const String &id )
{
	for ( uint8_t x = 0; x < accessorObjects.size(); x++ )
	{
		shared_ptr<Ladder_OBJ_Accessor> pObj = accessorObjects[x];
		if ( pObj && pObj->getID() == id )
			return pObj;
	}

	return 0;
}

shared_ptr<Ladder_VAR> PLC_Main::findLadderVarByID( const String &id ) 
{
	if ( strContains(id, CHAR_VAR_OPERATOR) ) //are we looking into a specific object that has already initialized? curently only supports local objects and not accessors
	{
		vector<String> argVec = splitString(id, CHAR_VAR_OPERATOR); //create a vector of strings to poll with
		if ( argVec.size() > 1) //make sure number of elements is valid, must have an object ID and a variable ID
		{
			shared_ptr<Ladder_OBJ> currentObj = findLadderObjByID(argVec[0]); //use the first index of the vector to find the existing object.
			if ( currentObj ) //must exist
				return currentObj->getObjectVAR(argVec[1]);
		}
	}
	else //This is a variable object that was explicitly declared in the script.
	{
		for ( uint16_t x = 0; x < ladderVars.size(); x++ )
		{
			shared_ptr<Ladder_VAR> pObj = ladderVars[x];
			if ( pObj && pObj->getID() == id )
				return pObj;
		}
	}
	
	return 0; //default
}

bool PLC_Main::parseScript(const char *script)
{
	resetAll(); //Purge all previous ladder logic objects before applying new script, also generate a new pinmap.

	String scriptLine; //container for parsed characters
	uint16_t iLine = 0;
	for (uint16_t x = 0; x <= strlen(script); x++) //go one char at a time.
	{
		if ( script[x] == CHAR_SPACE ) //omit spaces
			continue;
			
		if ( script[x] != CHAR_NEWLINE && script[x] != CHAR_CARRIAGE && x < strlen(script) ) //Do this one line at a time.
		{
			scriptLine += toUpper(script[x]); //convert all chars to upper case. This might be done elsewhere later (web interface code) but for now we'll do it here
		}
		else if (scriptLine.length() > 1) //we've hit a newline or carriage return char and we've got a valid length
		{
			iLine++; //Looks like we have a valid line
			shared_ptr<PLC_Parser> parser = make_shared<PLC_Parser>(scriptLine, getNumRungs() );
			if ( !parser->parseLine() )
			{
				sendError( ERR_DATA::ERR_PARSER_FAILED, PSTR("At Line: ") + String(iLine));
				return false; //error ocurred somewhere?
			}

			scriptLine.clear(); //empty the container for the next line
		}
	}

	pinMap.clear(); //free some memory
	pwmMap.clear();
	return true; //success
}

shared_ptr<Ladder_OBJ> PLC_Main::createNewLadderObject(const String &name, const vector<String> &ObjArgs )
{
	if ( name.length() > MAX_PLC_OBJ_NAME ) //name is too long. Gotta keep memory in mind
	{
		sendError(ERR_DATA::ERR_NAME_TOO_LONG, name );
	}
	else
	{
		if (ObjArgs.size() >= 1) //must have at least one arg (first indictes the object type)
		{
			String type = ObjArgs[0];
			if ( type == variableTag1 || type == variableTag2 ) //Is this a variable type object? (Used for local data storage in memory, to facilitate communication between objects)
			{
				return createVariableOBJ(name, ObjArgs);
			}
			else if ( type == inputTag1 || type == inputTag2 )
			{
				return createInputOBJ(name, ObjArgs);
			}
			else if ( type == outputTag1 || type == outputTag2 ) 
			{
				return createOutputOBJ(name, ObjArgs);
			}
			else if ( type == timerTag1 || type == timerTag2 ) 
			{
				return createTimerOBJ(name, ObjArgs);
			}
			else if ( type == counterTag1 || type == counterTag2 ) 
			{
				return createCounterOBJ(name, ObjArgs);
			}
			else if ( type == mathTag ) 
			{
				return createMathOBJ(name, ObjArgs);
			}
			else if ( type == remoteTag )
			{
				return createRemoteClient(name, ObjArgs);
			}
			else if ( type == oneshotTag )
			{
				return createOneshotOBJ();
			}
			else //if we don't find a valid object type
				sendError(ERR_DATA::ERR_UNKNOWN_TYPE, type);
		}
		else //not enough arguments to create a new ladder object
			sendError(ERR_DATA::ERR_INSUFFICIENT_ARGS, name );
	}
	
	return 0;
}

shared_ptr<Ladder_OBJ_Logical> PLC_Main::createInputOBJ( const String &id, const vector<String> &args )
{
	uint8_t pin = 0, logic = LOGIC_NO, numArgs = args.size();

	OBJ_TYPE type = OBJ_TYPE::TYPE_INPUT;

	if ( numArgs > 3 )
		logic = parseLogic(args[3]); //normally open or normally closed.

	if ( numArgs > 2 )
	{
		if ( args[2] == "A" || args[2] == typeTagAnalog ) //Explicitly declare an analog input, otherwise assume it's a digital in only
			type = OBJ_TYPE::TYPE_INPUT_ANALOG;
		else if (args[2] == "D" || args[2] == typeTagDigital )
			type = OBJ_TYPE::TYPE_INPUT;
		else
			sendError( ERR_DATA::ERR_UNKNOWN_ARGS, args[2] ); 
		
	}
	
	if ( numArgs > 1 ) //needed
	{
		pin = args[1].toInt(); 
		if ( isValidPin(pin, type) )
		{
			shared_ptr<InputOBJ> newObj(new InputOBJ(id, pin, type, logic));
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

shared_ptr<Ladder_OBJ_Logical> PLC_Main::createOutputOBJ( const String &id, const vector<String> &args )
{
	uint8_t pin = 0, logic = LOGIC_NO, numArgs = args.size();
	OBJ_TYPE type = OBJ_TYPE::TYPE_OUTPUT; //default
	uint16_t duty_cycle = 0;
	uint8_t resolution = 10;
	int8_t pwm_channel = 0; //signed because possible -1 (error) value

	double frequency = 5000;

	if ( numArgs > 6 ) //PWM resolution (bits)
	{
		int32_t tempInt = args[6].toInt();
		if ( tempInt > 0 && tempInt <= 16 ) //max 16 bit
			resolution = tempInt;
		else
			sendError(ERR_DATA::ERR_OUT_OF_RANGE, args[6] );
	}

	if ( numArgs > 5 ) //frequency
	{
		double tempDbl = args[5].toDouble();
		if ( tempDbl > 0 && tempDbl < CPU_CLK_FREQ/exp2(resolution) ) //80Mhz may or may not be the right value here. Look into later.
			frequency = tempDbl;
		else
			sendError(ERR_DATA::ERR_OUT_OF_RANGE, args[5] );
	}

	if ( numArgs > 4 ) //duty cycle
	{
		int32_t tempInt = args[4].toInt();
		if ( tempInt > 0 && tempInt < exp2(resolution) ) 
			duty_cycle = tempInt;
		else
			sendError(ERR_DATA::ERR_OUT_OF_RANGE, args[4] );
	}

	if ( numArgs > 3 )
		logic = parseLogic(args[3]);

	if ( numArgs > 2 ) //caveat here is that outputs would NEVER normally be normally closed (ON)
	{
		if ( args[2] == typeTagPWM || args[2] == "P" ) //PWM output
			type = OBJ_TYPE::TYPE_OUTPUT_PWM;
		else if ( args[2] == typeTagDigital || args[2] == "D" )
			type = OBJ_TYPE::TYPE_OUTPUT;
		else
			sendError(ERR_DATA::ERR_UNKNOWN_ARGS, args[2]);
	}
		
	if ( numArgs > 1 )
	{
		pin = args[1].toInt();
		if ( isValidPin(pin, OBJ_TYPE::TYPE_OUTPUT) ) //all outputs can also use PWM
		{
			if ( type == OBJ_TYPE::TYPE_OUTPUT_PWM )
			{
				pwm_channel = reservePWMChannel(); //get the next available pwm channel and return it.
				if ( pwm_channel < 0 )
				{
					sendError(ERR_DATA::ERR_OUT_OF_RANGE, String(pwm_channel) );
					return 0; //must be able to reserve a PWM channel for PWM outputs
				}
			}

			shared_ptr<OutputOBJ> newObj(new OutputOBJ(id, pin, type, logic, pwm_channel, duty_cycle, frequency, resolution));
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

shared_ptr<Ladder_OBJ_Logical> PLC_Main::createTimerOBJ( const String &id, const vector<String> &args )
{
	uint8_t numArgs = args.size();
	uint32_t delay = 0, accum = 0;
	OBJ_TYPE subType = OBJ_TYPE::TYPE_TIMER_ON;

	if ( numArgs > 3 )
	{	  
		if ( args[3] == typeTagTOF )
			subType = OBJ_TYPE::TYPE_TIMER_OFF;
		else if ( args[3] == typeTagTON )
			subType = OBJ_TYPE::TYPE_TIMER_ON;
		else
			sendError(ERR_DATA::ERR_UNKNOWN_ARGS, args[0] + args[3]); 
	}
	if ( numArgs > 2 )
		accum = args[2].toInt(); //some output verification tests here

	if ( numArgs > 1 ) //must have at least one arg
	{
		delay = args[1].toInt(); //verification tests? 
		if (delay > 1) //Must have a valid delay time. 
		{
			shared_ptr<TimerOBJ> newObj(new TimerOBJ(id, delay, accum, subType ));
			ladderObjects.emplace_back(newObj);
			#ifdef DEBUG
			Serial.println(PSTR("NEW TIMER"));
			#endif
			return newObj;
		}
	}

	return 0;
}

shared_ptr<Ladder_OBJ_Logical> PLC_Main::createCounterOBJ( const String &id, const vector<String> &args )
{
	uint16_t count = 0, accum = 0; 
	uint8_t numArgs = args.size();
	OBJ_TYPE subType = OBJ_TYPE::TYPE_COUNTER_UP;

	if ( numArgs > 3 )
	{
		if ( args[3] == typeTagCTU )
			subType = OBJ_TYPE::TYPE_COUNTER_UP;
		else if ( args[3] == typeTagCTD )
			subType = OBJ_TYPE::TYPE_COUNTER_DOWN;
		else
			sendError(ERR_DATA::ERR_UNKNOWN_ARGS, args[0] + args[3]);
	}
	if ( numArgs > 2 )
		accum = args[2].toInt(); //some output verification tests here?
		
	if ( numArgs > 1 )
	{
		count = args[1].toInt();
		shared_ptr<CounterOBJ> newObj(new CounterOBJ(id, count, accum, subType));
		ladderObjects.emplace_back(newObj);
		#ifdef DEBUG
		Serial.println(PSTR("NEW COUNTER"));
		#endif
		return newObj;
	}
	return 0;
}

//TODO - Implement some way where a user can easily dictate which variable type to use for memory purposes, otherwise default to auto-detection (maybe always estimate high? 64-bit?).
shared_ptr<Ladder_OBJ_Logical> PLC_Main::createVariableOBJ( const String &id, const vector<String> &args )
{
	shared_ptr<Ladder_VAR> newObj = 0;

	//could use an else here for but it really doesn't matter
	if ( args.size() > 1 )
	{
		bool isDouble = false;
		String val;
		
		if(args[1] == "TRUE")
		{
			newObj = make_shared<Ladder_VAR>(true, id);
		}
		else if(args[1] == "FALSE")
		{
			newObj = make_shared<Ladder_VAR>(false, id);
		}

		if ( !newObj ) //No explicit boolean object was created
		{
			for (uint8_t x = 0; x < args[1].length(); x++ )
			{
				if (x > 18) //this is plenty of digits, more than will ever likely be needed
					break;
					
				if ( (args[1][x] >= '0' && args[1][x] <= '9') || args[1][x] == '.' || ( args[1][x] == '-' && x == 0 ) )
				{
					if ( args[1][x] == '.' )
					{
						if ( isDouble )// multiple '.' chars should be ignored
							continue;
							
						isDouble = true;
					}
					val += args[1][x];
				}
			}
			
			if ( !val.length() )
				return 0;
		}

		if (args.size() > 2 && !newObj) // in this case, we are manually specifying the type of variable that we are initializing
		{
			if (args[2] == VAR_INT32)
			{
				int64_t value = static_cast<int64_t>(atoll(val.c_str()));
				if( value < INT32_MIN || value > INT32_MAX )
				{
					sendError(ERR_DATA::ERR_OUT_OF_RANGE, args[2] + CHAR_SPACE + val );
					return 0;
				}
				newObj = make_shared<Ladder_VAR>( static_cast<int32_t>(value), id );
			}
			else if (args[2] == VAR_UINT32)
			{
				int64_t value = static_cast<int64_t>(atoll(val.c_str()));
				if(value < 0 || value > UINT32_MAX)
				{
					sendError(ERR_DATA::ERR_OUT_OF_RANGE, args[2] + CHAR_SPACE + val );
					return 0;
				}
				newObj = make_shared<Ladder_VAR>( static_cast<uint_fast32_t>(value), id );
			}
			else if (args[2] == VAR_INT64)
			{
				int64_t value = static_cast<int64_t>(atoll(val.c_str())); //variable > int64 needed?
				if( value < INT64_MIN || value > INT64_MAX ) //Are these criteria possible? -- probably not. Look into later.
				{
					sendError(ERR_DATA::ERR_OUT_OF_RANGE, args[2] + CHAR_SPACE + val );
					return 0;
				}
				newObj = make_shared<Ladder_VAR>(value, id);
			}
			else if (args[2] == VAR_UINT64)
			{
				uint64_t value = static_cast<uint64_t>(strtoull(val.c_str(), NULL, 10));
				if( value > UINT64_MAX ) //Is this possible?
				{
					sendError(ERR_DATA::ERR_OUT_OF_RANGE, args[2] + CHAR_SPACE + val );
					return 0;
				}
				newObj = make_shared<Ladder_VAR>( value, id );
			}
			else if (args[2] == VAR_DOUBLE)
			{
				newObj = make_shared<Ladder_VAR>(atof(val.c_str()), id );
			}
			else if (args[2] == VAR_BOOL || args[2] == VAR_BOOLEAN)
			{
				newObj = make_shared<Ladder_VAR>(static_cast<bool>(val.c_str()), id );
			}
			else 
			{
				sendError(ERR_DATA::ERR_INCORRECT_VAR_TYPE, args[2]);
				return 0;
			}
		}

		if ( !newObj ) //So we haven't created an object yet (for one reason or another) -- So try to create one from the parsed value string
		{
			if (isDouble)
			{
				newObj = make_shared<Ladder_VAR>( atof(val.c_str()), id );
			}
			else
			{
				newObj = make_shared<Ladder_VAR>( atoll(val.c_str()), id ); //assume a long (greater than 32 bits) This is a safe data type to use as it is a signed 64 bit int
			}
		}
	}

	if ( newObj ) //We've created a new object, so store it in the appropriate vectors.
	{
		ladderObjects.emplace_back(newObj);
		ladderVars.emplace_back(newObj);
		#ifdef DEBUG
		Serial.println( PSTR("New variable has value: ") + newObj->getValueStr() ); 
		#endif
	}

	return newObj;
}

shared_ptr<Ladder_OBJ_Logical> PLC_Main::createOneshotOBJ()
{
	shared_ptr<OneshotOBJ> newObj(new OneshotOBJ());
	return newObj;
}

shared_ptr<Ladder_OBJ_Logical> PLC_Main::createMathOBJ( const String &id, const vector<String> & args)
{
	shared_ptr<MathBlockOBJ> newObj = 0;
	uint8_t argSize = args.size(); //get number of arguments that were passed in

	if(argSize >= 3)
	{
		String function = args[1]; //this is the type of function that is being created
		shared_ptr<Ladder_VAR> var1ptr = findLadderVarByID(args[2]);
		shared_ptr<Ladder_VAR> var2ptr = 0;
		shared_ptr<Ladder_VAR> var3ptr = 0;
		
		if ( !var1ptr ) //must always have at least one valid source to continue
		{
			//error here could not find argument
			return 0;
		}

		//Arguments are arg[0] = math, arg[1] = function, arg[2] = first variable, arg[3] = second variable, arg[4] = dest
		if(argSize >= 4) //must have at least 4 args to access string at args[3]
		{
			var2ptr = findLadderVarByID(args[3]);
		}
		if(argSize == 5) //must have at least 5 args
		{
			var3ptr = findLadderVarByID(args[4]);
		}
		else if ( argSize > 5 )
		{
			sendError(ERR_DATA::ERR_MATH_TOO_MANY_ARGS);
		}
		
		if(function == typeTagMTAN)
		{
			if(argSize == 3)
			{
				newObj = make_shared<MathBlockOBJ>(id, OBJ_TYPE::TYPE_MATH_TAN, var1ptr);
			}
			else if(argSize == 4)
			{
				newObj = make_shared<MathBlockOBJ>(id, OBJ_TYPE::TYPE_MATH_TAN, var1ptr, shared_ptr<Ladder_VAR>(0), var2ptr);
			}
			else
			{
				sendError(ERR_DATA::ERR_MATH_TOO_MANY_ARGS);
			}
		}
		else if(function == typeTagMSIN)
		{
			if(argSize == 3)
			{
				newObj = make_shared<MathBlockOBJ>(id, OBJ_TYPE::TYPE_MATH_SIN, var1ptr);
			}
			else if(argSize == 4)
			{
				newObj = make_shared<MathBlockOBJ>(id, OBJ_TYPE::TYPE_MATH_SIN, var1ptr, shared_ptr<Ladder_VAR>(0), var2ptr);
			}
			else
			{
				sendError(ERR_DATA::ERR_MATH_TOO_MANY_ARGS);
			}
		}
		else if(function == typeTagMCOS)
		{
			if(argSize == 3)
			{
				newObj = make_shared<MathBlockOBJ>(id, OBJ_TYPE::TYPE_MATH_COS, var1ptr);
			}
			else if(argSize == 4)
			{
				newObj = make_shared<MathBlockOBJ>(id, OBJ_TYPE::TYPE_MATH_COS, var1ptr, shared_ptr<Ladder_VAR>(0), var2ptr);
			}
			else
			{
				sendError(ERR_DATA::ERR_MATH_TOO_MANY_ARGS);
			}
		}
		else if(function == typeTagMATAN)
		{
			if(argSize == 3)
			{
				newObj = make_shared<MathBlockOBJ>(id, OBJ_TYPE::TYPE_MATH_ATAN, var1ptr);
			}
			else if(argSize == 4)
			{
				newObj = make_shared<MathBlockOBJ>(id, OBJ_TYPE::TYPE_MATH_ATAN, var1ptr, shared_ptr<Ladder_VAR>(0), var2ptr);
			}
			else
			{
				sendError(ERR_DATA::ERR_MATH_TOO_MANY_ARGS);
			}
		}
		else if(function == typeTagMASIN)
		{
			if(argSize == 3)
			{
				newObj = make_shared<MathBlockOBJ>(id, OBJ_TYPE::TYPE_MATH_ASIN, var1ptr);
			}
			else if(argSize == 4)
			{
				newObj = make_shared<MathBlockOBJ>(id, OBJ_TYPE::TYPE_MATH_ASIN, var1ptr, shared_ptr<Ladder_VAR>(0), var2ptr);
			}
			else
			{
				sendError(ERR_DATA::ERR_MATH_TOO_MANY_ARGS);
			}
		}
		else if(function == typeTagMACOS)
		{
			if(argSize == 3)
			{
				newObj = make_shared<MathBlockOBJ>(id, OBJ_TYPE::TYPE_MATH_ACOS, var1ptr);
			}
			else if(argSize == 4)
			{
				newObj = make_shared<MathBlockOBJ>(id, OBJ_TYPE::TYPE_MATH_ACOS, var1ptr, shared_ptr<Ladder_VAR>(0), var2ptr);
			}
			else
			{
				sendError(ERR_DATA::ERR_MATH_TOO_MANY_ARGS);
			}
		}
		else if(function == typeTagMMUL)
		{
			if(argSize < 4)
			{
				sendError(ERR_DATA::ERR_MATH_TOO_FEW_ARGS);
			}
			else if(argSize == 4)
			{
				newObj = make_shared<MathBlockOBJ>(id, OBJ_TYPE::TYPE_MATH_MUL, var1ptr, var2ptr);
			}
			else if(argSize == 5)
			{
				newObj = make_shared<MathBlockOBJ>(id, OBJ_TYPE::TYPE_MATH_MUL, var1ptr, var2ptr, var3ptr);
			}
			else
			{
				sendError(ERR_DATA::ERR_MATH_TOO_MANY_ARGS);
			}
		}
		else if(function == typeTagMDIV)
		{
			if(argSize < 4)
			{
				sendError(ERR_DATA::ERR_MATH_TOO_FEW_ARGS);
			}
			else if(var2ptr->getValue<double>() == 0)
			{
				newObj = 0;
				sendError(ERR_DATA::ERR_MATH_DIV_BY_0);
			}
			else if(argSize == 4)
			{
				newObj = make_shared<MathBlockOBJ>(id, OBJ_TYPE::TYPE_MATH_DIV, var1ptr, var2ptr);
			}
			else if(argSize == 5)
			{
				newObj = make_shared<MathBlockOBJ>(id, OBJ_TYPE::TYPE_MATH_DIV, var1ptr, var2ptr, var3ptr);
			}
			else
			{
				sendError(ERR_DATA::ERR_MATH_TOO_MANY_ARGS);
			}
		}
		else if(function == typeTagMADD)
		{
			if(argSize < 4)
			{
				sendError(ERR_DATA::ERR_MATH_TOO_FEW_ARGS);
			}
			else if(argSize == 4)
			{
				newObj = make_shared<MathBlockOBJ>(id, OBJ_TYPE::TYPE_MATH_ADD, var1ptr, var2ptr);
			}
			else if(argSize == 5)
			{
				newObj = make_shared<MathBlockOBJ>(id, OBJ_TYPE::TYPE_MATH_ADD, var1ptr, var2ptr, var3ptr);
			}
			else
			{
				sendError(ERR_DATA::ERR_MATH_TOO_MANY_ARGS);
			}
		}
		else if(function == typeTagMSUB)
		{
			if(argSize < 4)
			{
				sendError(ERR_DATA::ERR_MATH_TOO_FEW_ARGS);
			}
			else if(argSize == 4)
			{
				newObj = make_shared<MathBlockOBJ>(id, OBJ_TYPE::TYPE_MATH_SUB, var1ptr, var2ptr);
			}
			else if(argSize == 5)
			{
				newObj = make_shared<MathBlockOBJ>(id, OBJ_TYPE::TYPE_MATH_SUB, var1ptr, var2ptr, var3ptr);
			}
			else
			{
				sendError(ERR_DATA::ERR_MATH_TOO_MANY_ARGS);
			}
		}
		else if(function == typeTagMEQ)
		{
			if(argSize < 4)
			{
				sendError(ERR_DATA::ERR_MATH_TOO_FEW_ARGS);
			}
			else if(argSize == 4)
			{
				newObj = make_shared<MathBlockOBJ>(id, OBJ_TYPE::TYPE_MATH_EQ, var1ptr, var2ptr);
			}
			else if(argSize == 5)
			{
				newObj = make_shared<MathBlockOBJ>(id, OBJ_TYPE::TYPE_MATH_EQ, var1ptr, var2ptr, var3ptr);
			}
			else
			{
				sendError(ERR_DATA::ERR_MATH_TOO_MANY_ARGS);
			}
		}
		else if(function == typeTagMNEQ)
		{
			if(argSize < 4)
			{
				sendError(ERR_DATA::ERR_MATH_TOO_FEW_ARGS);
			}
			else if(argSize == 4)
			{
				newObj = make_shared<MathBlockOBJ>(id, OBJ_TYPE::TYPE_MATH_NEQ, var1ptr, var2ptr);
			}
			else if(argSize == 5)
			{
				newObj = make_shared<MathBlockOBJ>(id, OBJ_TYPE::TYPE_MATH_NEQ, var1ptr, var2ptr, var3ptr);
			}
			else
			{
				sendError(ERR_DATA::ERR_MATH_TOO_MANY_ARGS);
			}
		}
		else if(function == typeTagMGRE)
		{
			if(argSize < 4)
			{
				sendError(ERR_DATA::ERR_MATH_TOO_FEW_ARGS);
			}
			else if(argSize == 4)
			{
				newObj = make_shared<MathBlockOBJ>(id, OBJ_TYPE::TYPE_MATH_GRT, var1ptr, var2ptr);
			}
			else if(argSize == 5)
			{
				newObj = make_shared<MathBlockOBJ>(id, OBJ_TYPE::TYPE_MATH_GRT, var1ptr, var2ptr, var3ptr);
			}
			else
			{
				sendError(ERR_DATA::ERR_MATH_TOO_MANY_ARGS);
			}
		}
		else if(function == typeTagMLES)
		{
			if(argSize < 4)
			{
				sendError(ERR_DATA::ERR_MATH_TOO_FEW_ARGS);
			}
			else if(argSize == 4)
			{
				newObj = make_shared<MathBlockOBJ>(id, OBJ_TYPE::TYPE_MATH_LES, var1ptr, var2ptr);
			}
			else if(argSize == 5)
			{
				newObj = make_shared<MathBlockOBJ>(id, OBJ_TYPE::TYPE_MATH_LES, var1ptr, var2ptr, var3ptr);
			}
			else
			{
				sendError(ERR_DATA::ERR_MATH_TOO_MANY_ARGS);
			}
		}
		else if(function == typeTagMGREE)
		{
			if(argSize < 4)
			{
				sendError(ERR_DATA::ERR_MATH_TOO_FEW_ARGS);
			}
			else if(argSize == 4)
			{
				newObj = make_shared<MathBlockOBJ>(id, OBJ_TYPE::TYPE_MATH_GRQ, var1ptr, var2ptr);
			}
			else if(argSize == 5)
			{
				newObj = make_shared<MathBlockOBJ>(id, OBJ_TYPE::TYPE_MATH_GRQ, var1ptr, var2ptr, var3ptr);
			}
			else
			{
				sendError(ERR_DATA::ERR_MATH_TOO_MANY_ARGS);
			}
		}
		else if(function == typeTagMLESE)
		{
			if(argSize < 4)
			{
				sendError(ERR_DATA::ERR_MATH_TOO_FEW_ARGS);
			}
			else if(argSize == 4)
			{
				newObj = make_shared<MathBlockOBJ>(id, OBJ_TYPE::TYPE_MATH_LEQ, var1ptr, var2ptr);
			}
			else if(argSize == 5)
			{
				newObj = make_shared<MathBlockOBJ>(id, OBJ_TYPE::TYPE_MATH_LEQ, var1ptr, var2ptr, var3ptr);
			}
			else
			{
				sendError(ERR_DATA::ERR_MATH_TOO_MANY_ARGS);
			}
		}
		else if(function == typeTagMINC)
		{
			if(argSize == 3)
			{
				newObj = make_shared<MathBlockOBJ>(id, OBJ_TYPE::TYPE_MATH_INC, var1ptr);
			}
			else
			{
				sendError(ERR_DATA::ERR_MATH_TOO_MANY_ARGS);
			}
		}
		else if(function == typeTagMDEC)
		{
			if(argSize == 3)
			{
				newObj = make_shared<MathBlockOBJ>(id, OBJ_TYPE::TYPE_MATH_DEC, var1ptr);
			}
			else
			{
				sendError(ERR_DATA::ERR_MATH_TOO_MANY_ARGS);
			}
		}
		else if(function == typeTagMMOV)
		{
			if(argSize < 4)
			{
				sendError(ERR_DATA::ERR_MATH_TOO_FEW_ARGS);
			}
			else if(argSize == 4)
			{
				newObj = make_shared<MathBlockOBJ>(id, OBJ_TYPE::TYPE_MATH_MOV, var1ptr, shared_ptr<Ladder_VAR>(0), var2ptr);
			}
			else
			{
				sendError(ERR_DATA::ERR_MATH_TOO_MANY_ARGS);
			}
		}
		else
		{
			sendError(ERR_DATA::ERR_INVALID_FUNCTION, function);
		}
	}

	if(newObj)
	{
		ladderObjects.emplace_back(newObj);
	}
	return newObj;
}

shared_ptr<Ladder_OBJ_Accessor> PLC_Main::createRemoteClient( const String &id, const vector<String> &args )
{
	//Args: IP, Port, Timeout Time
	uint32_t timeout = 2000, updfreq = 1000; //default values in ms
    uint8_t numArgs = args.size();
    if ( numArgs > 5 )
    {
        for ( uint8_t x = 5; x < numArgs; x++ )
        {
            sendError(ERR_DATA::ERR_UNKNOWN_ARGS, args[0] + args[x]);
        }
    }
    if ( numArgs > 4 )
    {
        long tempInt = args[4].toInt();
        if ( tempInt > 0 ) //maybe a minimum here? 
		    updfreq = tempInt; //update freq in ms
    }
	if ( numArgs > 3)
	{
        long tempInt = args[3].toInt();
        if ( tempInt > 0 )
		    timeout = tempInt; //timeout time in ms
	}
	if ( numArgs > 2) //must have IP and port
	{
		IPAddress serverIP;

        uint16_t port = 5000; //default port

        long tempPort = args[2].toInt();
        if ( tempPort > 0 && tempPort <= UINT16_MAX )
            port = tempPort;
		
		if ( WiFi.hostByName(args[1].c_str(), serverIP ) )
		{
			for ( uint8_t x = 0; x < getNumAccessors(); x++ )
            {
                if ( getAccessorObjects()[x]->getType() == OBJ_TYPE::TYPE_REMOTE )
                {
                    PLC_Remote_Client *currentClient = static_cast<PLC_Remote_Client *>(getAccessorObjects()[x].get());
                    if ( currentClient && currentClient->getHostAddress() == serverIP )
                    {
                        sendError( ERR_DATA::ERR_CREATION_FAILED, String(CHAR_SPACE) + PSTR("Duplicate IP"));
                        return 0; //already exists based on IP -- some error here?
                    }
                } 
            }

            if ( port == 80 ) //invalid port?
            {
                sendError( ERR_DATA::ERR_CREATION_FAILED, String(CHAR_SPACE) + PSTR("Invalid Port"));
                return 0; //some error here?
            }

            shared_ptr<PLC_Remote_Client> accessorClient = make_shared<PLC_Remote_Client>(id, serverIP, port, timeout, updfreq );
            getAccessorObjects().push_back( accessorClient );
            return accessorClient;
		}
		else
			Core.sendMessage(PSTR("Unknown host: ") + args[1]);
	}

	return 0;
}

bool PLC_Main::isValidPin( uint8_t pin, OBJ_TYPE type)
{
	std::map<uint8_t, PIN_TYPE>::iterator pinitr = pinMap.find(pin);
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

		if ( ( type == OBJ_TYPE::TYPE_INPUT_ANALOG && ( pinitr->second == PIN_TYPE::PIN_AI || pinitr->second == PIN_TYPE::PIN_AIO ) ) 
			|| ( type == OBJ_TYPE::TYPE_INPUT && ( pinitr->second == PIN_TYPE::PIN_AI || pinitr->second == PIN_TYPE::PIN_AIO || pinitr->second == PIN_TYPE::PIN_IO || pinitr->second == PIN_TYPE::PIN_I ) ) )
			{
				return true;
			}
		else if ( type == OBJ_TYPE::TYPE_OUTPUT && ( pinitr->second == PIN_TYPE::PIN_AIO || pinitr->second == PIN_TYPE::PIN_IO || pinitr->second == PIN_TYPE::PIN_O ) )
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
	std::map<uint8_t, PIN_TYPE>::iterator pinitr = pinMap.find(pin);
	if (pinitr != pinMap.end())
	{
		pinitr->second = PIN_TYPE::PIN_TAKEN;
		return true;
	}
	return false; //shouldn't happen
}

void PLC_Main::sendError(ERR_DATA err, const String &info )
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
		case ERR_DATA::ERR_INVALID_ACCESSOR:
		{

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
		case ERR_DATA::ERR_NAME_TOO_LONG:
		{
			error = err_failed_creation + CHAR_SPACE + err_name_too_long;
		}
		break;
		case ERR_DATA::ERR_INCORRECT_VAR_TYPE:
		{
			error = err_var_type_invalid;
		}
		break;
		case ERR_DATA::ERR_OUT_OF_RANGE:
		{
			error = err_var_out_of_range;
		}
		break;
		case ERR_DATA::ERR_INVALID_FUNCTION:
		{
			error = err_invalid_function;
		}
		break;
		case ERR_DATA::ERR_MATH_TOO_MANY_ARGS:
		{
			error = err_math_too_many_args;
		}
		break;
		case ERR_DATA::ERR_MATH_TOO_FEW_ARGS:
		{
			error = err_math_too_few_args;
		}
		break;
		case ERR_DATA::ERR_MATH_DIV_BY_0:
		{
			error = err_math_division_by_zero;
		}
		break;
	}

	if ( info.length() )
		Core.sendMessage(error + CHAR_SPACE + "\"" + info + "\"", PRIORITY_HIGH);
	else
		Core.sendMessage(error, PRIORITY_HIGH);

	resetAll(); //Since we have hit an error, just purge all objects from the PLC script. Since it can't be used anyway.
}

bool PLC_Main::createRemoteServer( uint16_t port )
{
	if ( getRemoteServer() && getRemoteServer()->getPort() == port )
		return false; //already initialized on that port. Do nothing 

	getRemoteServer() = unique_ptr<PLC_Remote_Server>( new PLC_Remote_Server(port) );
	return true;
}

vector<IPAddress> PLC_Main::scanForRemoteNodes( uint16_t port, uint8_t low, uint8_t high, uint16_t timeout )
{
    vector<IPAddress> ipAddrs;
    if ( low >= high )
        return ipAddrs; //nothing to do

    for ( uint8_t x = low; x < high; x++ )
    {
        IPAddress addr(192,168,0,x); //could use some tweaking
        shared_ptr<WiFiClient> newClient = make_shared<WiFiClient>();
        if ( newClient->connect( addr, port, timeout ) ) //were we able to connect to the inputted IP address on the given port?
        {
            Core.sendMessage(PSTR("Found node at: ") + addr.toString() );
            ipAddrs.push_back( newClient->remoteIP() );
        }
    }

    return ipAddrs;
}
