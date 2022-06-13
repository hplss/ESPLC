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
#include "ACCESSORS/acc_CAN.h"
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

OBJ_LOGIC_PTR PLC_Main::findLadderObjByID( const String &id ) //Search through all created objects thus far. This assumes that the object was created successfully.
{
	for ( uint16_t x = 0; x < ladderObjects.size(); x++ )
	{
		OBJ_LOGIC_PTR pObj = ladderObjects[x];
		if ( pObj && pObj->sObjID == id )
			return pObj;
	}
	
	return 0; //default
}

OBJ_ACC_PTR PLC_Main::findAccessorByID( const String &id )
{
	for ( uint8_t x = 0; x < accessorObjects.size(); x++ )
	{
		OBJ_ACC_PTR pObj = accessorObjects[x];
		if ( pObj && pObj->sObjID == id )
			return pObj;
	}

	return 0;
}

VAR_PTR PLC_Main::findLadderVarByID( const String &id ) 
{
	if ( strContains(id, CHAR_ACCESSOR_OPERATOR)) //looks like we're trying to access variables that are stored in an accessor
	{
		vector<String> argVec = splitString(id, CHAR_ACCESSOR_OPERATOR); //create a vector of strings to poll with

		if ( argVec.size() > 1) //make sure number of elements is valid, must have an object ID and a variable ID
		{
			OBJ_ACC_PTR accessor = findAccessorByID( argVec[0]);
			if ( accessor )
			{
				return static_pointer_cast<Ladder_VAR>(accessor->findAccessorVarByID(argVec[1])); //use the first index of the vector to find the existing object.
			}
		}
	}
	else if ( strContains(id, CHAR_VAR_OPERATOR) ) //are we looking into a specific object that has already initialized locally? 
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
			VAR_PTR pObj = ladderVars[x];
			if ( pObj && pObj->sObjID == id )
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
			if ( !strBeginsWith(scriptLine, vector<String>{"//", "#"} ) ) //omit comments in scripts (at beginning of line)
			{
				shared_ptr<PLC_Parser> parser = make_shared<PLC_Parser>(scriptLine, getNumRungs() );
				if ( !parser->parseLine() )
				{
					sendError( ERR_DATA::ERR_PARSER_FAILED, PSTR("At Line: ") + String(iLine));
					return false; //error ocurred somewhere?
				}
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
			OBJ_TYPE objType = findMathObjectType(type);

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
			else if ( objType != OBJ_TYPE::TYPE_INVALID ) //type == mathTag 
			{
				return createMathOBJ(name, objType ,ObjArgs);
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

OBJ_LOGIC_PTR PLC_Main::createInputOBJ( const String &id, const vector<String> &args )
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

OBJ_LOGIC_PTR PLC_Main::createOutputOBJ( const String &id, const vector<String> &args )
{
	uint8_t pin = 0, logic = LOGIC_NO, numArgs = args.size();
	OBJ_TYPE type = OBJ_TYPE::TYPE_OUTPUT; //default
	uint16_t duty_cycle = 0;
	uint8_t resolution = 10;
	int8_t pwm_channel = 0; //signed because possible -1 (error) value

	double frequency = 5000;

	if ( numArgs > 6 ) //PWM resolution (bits) -- should be constant
	{
		int32_t tempInt = args[6].toInt();
		if ( tempInt > 0 && tempInt <= 16 ) //max 16 bit
			resolution = tempInt;
		else
			sendError(ERR_DATA::ERR_OUT_OF_RANGE, args[6] );
	}

	if ( numArgs > 5 ) //frequency -- should be constant?
	{
		double tempDbl = args[5].toDouble();
		if ( tempDbl > 0 && tempDbl < (CPU_CLK_FREQ/exp2(resolution)) ) //80Mhz may or may not be the right value here. Look into later.
			frequency = tempDbl;
		else
			sendError(ERR_DATA::ERR_OUT_OF_RANGE, args[5] );
	}

	if ( numArgs > 4 ) //duty cycle -- can change
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

OBJ_LOGIC_PTR PLC_Main::createTimerOBJ( const String &id, const vector<String> &args )
{
	uint8_t numArgs = args.size();
	VAR_PTR delay = 0, accum = 0;
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
	{
		accum = createVariableInstance(bitTagACC, args[2]);
	}

	if ( numArgs > 1 ) //must have at least one arg
	{
		delay = createVariableInstance(bitTagPRE, args[1]);
		shared_ptr<TimerOBJ> newObj(new TimerOBJ(id, delay, accum, subType ));
		ladderObjects.emplace_back(newObj);
		#ifdef DEBUG
		Serial.println(PSTR("NEW TIMER"));
		#endif
		return newObj;
	}

	return 0;
}

OBJ_LOGIC_PTR PLC_Main::createCounterOBJ( const String &id, const vector<String> &args )
{
	shared_ptr<Ladder_VAR> count = 0, accum = 0; 
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
	{
		accum = createVariableInstance(bitTagACC, args[2]); 
	}
		
	if ( numArgs > 1 )
	{
		count = createVariableInstance(bitTagPRE, args[1]); 

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
OBJ_LOGIC_PTR PLC_Main::createVariableOBJ( const String &id, const vector<String> &args )
{
	shared_ptr<Ladder_VAR> newObj = 0;

	//could use an else here for but it really doesn't matter
	if ( args.size() > 1 )
	{
		if(args[1] == PSTR("TRUE"))
		{
			newObj = make_shared<Ladder_VAR>(true, id);
		}
		else if(args[1] == PSTR("FALSE"))
		{
			newObj = make_shared<Ladder_VAR>(false, id);
		}

		if (args.size() > 2 && !newObj) // in this case, we are manually (explicitly) specifying the type of variable that we are initializing
		{
			if (args[2] == VAR_INT32)
			{
				int64_t value = static_cast<int64_t>(atoll(args[1].c_str()));
				if( value < INT32_MIN || value > INT32_MAX )
				{
					sendError(ERR_DATA::ERR_OUT_OF_RANGE, args[2] + CHAR_SPACE + args[1] );
					return 0;
				}
				newObj = make_shared<Ladder_VAR>( static_cast<int32_t>(value), id );
			}
			else if (args[2] == VAR_UINT32)
			{
				int64_t value = static_cast<int64_t>(atoll(args[1].c_str()));
				if(value < 0 || value > UINT32_MAX)
				{
					sendError(ERR_DATA::ERR_OUT_OF_RANGE, args[2] + CHAR_SPACE + args[1] );
					return 0;
				}
				newObj = make_shared<Ladder_VAR>( static_cast<uint_fast32_t>(value), id );
			}
			else if (args[2] == VAR_INT64)
			{
				int64_t value = static_cast<int64_t>(atoll(args[1].c_str())); //variable > int64 needed?
				if( value < INT64_MIN || value > INT64_MAX ) //Are these criteria possible? -- probably not. Look into later.
				{
					sendError(ERR_DATA::ERR_OUT_OF_RANGE, args[2] + CHAR_SPACE + args[1] );
					return 0;
				}
				newObj = make_shared<Ladder_VAR>(value, id);
			}
			else if (args[2] == VAR_UINT64)
			{
				uint64_t value = static_cast<uint64_t>(strtoull(args[1].c_str(), NULL, 10));
				if( value > UINT64_MAX ) //Is this possible?
				{
					sendError(ERR_DATA::ERR_OUT_OF_RANGE, args[2] + CHAR_SPACE + args[1] );
					return 0;
				}
				newObj = make_shared<Ladder_VAR>( value, id );
			}
			else if (args[2] == VAR_DOUBLE)
			{
				newObj = make_shared<Ladder_VAR>(atof(args[1].c_str()), id );
			}
			else if (args[2] == VAR_BOOL || args[2] == VAR_BOOLEAN)
			{
				newObj = make_shared<Ladder_VAR>(static_cast<bool>(args[1].c_str()), id );
			}
			else 
			{
				sendError(ERR_DATA::ERR_INCORRECT_VAR_TYPE, args[2]);
				return 0;
			}
		}

		if ( !newObj ) //So we haven't created an object yet (for one reason or another) -- So try to create one from the parsed value string
		{
			newObj = createVariableInstance(id, args[1]);
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

OBJ_ACC_PTR PLC_Main::createCANInterface( const String &id, const vector<String> &args )
{
	shared_ptr<PLC_CAN_ADAPTER> interface = 0;

	if ( args.size() < 5 )
		return 0;

	for ( uint8_t x = 1; x < 5; x++ )
	{
		if ( !isValidPin(args[x].toInt(), OBJ_TYPE::TYPE_OUTPUT))
		{
			#ifdef DEBUG
			Serial.println("PIN " + args[x] + " IS INVALID");
			#endif
			return 0;
		}
	}

	setClaimedPin(args[1].toInt());
	setClaimedPin(args[2].toInt());
	setClaimedPin(args[3].toInt());
	setClaimedPin(args[4].toInt());
	
	interface = make_shared<PLC_CAN_ADAPTER>(id, args[1].toInt(), args[2].toInt(), args[3].toInt(), args[4].toInt(), args.size() > 5 ? args[5].toInt() : 250000 );

	if ( interface ) //We've created a new object, so store it in the appropriate vectors.
	{
		ladderObjects.emplace_back(interface);
		#ifdef DEBUG
		Serial.println( PSTR("CREATED NEW CAN INTERFACE") ); 
		#endif
	}

	return interface;
}

//Creates a predefinition for a CAN frame associated with a particular initialized CAN interface.
OBJ_LOGIC_PTR PLC_Main::createCANFrame(const String &id, const vector<String> &args )
{
	CAN_FRAME_PTR frame = 0;

	bool tx = false;
	uint16_t txRate = 10; //10MS transmit rate by default (if it's a TX frame)

	if ( args.size() < 2 )
		return 0;

	frame = make_shared<CANFrameOBJ>(id, args[1].toInt(), args.size() > 2 ? args[3].toInt() : tx, args.size() > 3 ? args[4].toInt() : txRate );

	//CANFrameOBJ(const String &name, const uint32_t id, const bool tx = false, const uint16_t txRate = 10 ) 
	if ( frame ) //We've created a new object, so store it in the appropriate vectors.
	{
		ladderObjects.emplace_back(frame);
		#ifdef DEBUG
		Serial.println( PSTR("CREATED NEW CAN INTERFACE") ); 
		#endif
	}

	return frame;
}

//Creates a storage variable for use for writing/reading from received frame data via the CAN interface.
OBJ_LOGIC_PTR PLC_Main::createCANVariable(const String &id, const vector<String> &args )
{
	CAN_DATA_PTR newObj = 0;
	//CANDataOBJ(const String &id, CAN_FRAME_PTR parentFrame, const uint64_t mask, const uint8_t bitOffset, const double ratio = 1.0, const double offset = 0 ) 

	if ( args.size() < 4 )
		return 0;

	OBJ_LOGIC_PTR parentFrame = findLadderObjByID(args[1]);
	if ( !parentFrame || parentFrame->iType != OBJ_TYPE::TYPE_CAN_FRAME );
	{
		Serial.println("Cannot find CAN frame: " + args[1]);
		return 0;
	}

	newObj = make_shared<CANDataOBJ>(id,  );

	if ( newObj ) //We've created a new object, so store it in the appropriate vectors.
	{
		ladderObjects.emplace_back(newObj);
		ladderVars.emplace_back(newObj);
		#ifdef DEBUG
		Serial.println( PSTR("New CAN variable created.") ); 
		#endif
	}

	return newObj;
}

//this function is used to create a new variable object on the fly. Most likely for use by other objects that are being initialized.
VAR_PTR PLC_Main::createVariableInstance(const String &id, const String &arg)
{
	VAR_PTR newVar = 0;
	DATA_TYPE dataType = strDataType(arg);

	if ( dataType == DATA_TYPE::TYPE_DOUBLE) //double type
		newVar = make_shared<Ladder_VAR>( atof(arg.c_str()),id);
	else if ( dataType == DATA_TYPE::TYPE_INT)//integer type
		newVar = make_shared<Ladder_VAR>( atoll(arg.c_str()),id);
	else if ( dataType == DATA_TYPE::TYPE_STRING ) //attempt to find the variable as an existing object (whether alone or as a member of another pobject)
		newVar = findLadderVarByID(arg);

	return newVar;
}

OBJ_LOGIC_PTR PLC_Main::createOneshotOBJ()
{
	shared_ptr<OneshotOBJ> newObj(new OneshotOBJ());
	return newObj;
}

OBJ_LOGIC_PTR PLC_Main::createMathOBJ( const String &id, OBJ_TYPE type, const vector<String> & args)
{
	shared_ptr<MathBlockOBJ> newObj = 0;
	uint8_t argSize = args.size(); //get number of arguments that were passed in

	if(argSize >= 2)
	{
		VAR_PTR var1ptr = createVariableInstance(bitTagSRCA, args[1]); //set to null at first
		VAR_PTR var2ptr = 0;
		VAR_PTR var3ptr = 0;

		if ( !var1ptr ) //must always have at least one valid source and valid math type to continue
			return 0; //error here could not find argument

		//Arguments are: arg[0] = function, arg[1] = first variable, arg[2] = second variable, arg[3] = third variable
		if ( argSize > 4 )
		{
			sendError(ERR_DATA::ERR_MATH_TOO_MANY_ARGS);
		}
		
		//check for objects that need SourceA only
		if ( type == OBJ_TYPE::TYPE_MATH_INC || type == OBJ_TYPE::TYPE_MATH_DEC )
		{
			newObj = make_shared<MathBlockOBJ>(id, type, var1ptr);
		}
		// check for objects that use SourceA / DEST (Optional) only
		else if(type == OBJ_TYPE::TYPE_MATH_TAN || type == OBJ_TYPE::TYPE_MATH_SIN || type == OBJ_TYPE::TYPE_MATH_ACOS || type == OBJ_TYPE::TYPE_MATH_COS
		|| type == OBJ_TYPE::TYPE_MATH_ATAN || type == OBJ_TYPE::TYPE_MATH_ASIN || type == OBJ_TYPE::TYPE_MATH_MOV ) 
		{
			var2ptr = createVariableInstance(bitTagDEST, args[2]);

			newObj = make_shared<MathBlockOBJ>(id, type, var1ptr, VAR_PTR(0), var2ptr );
		}
		//check for objects that require SourceA, SourceB, and DEST (Optional)
		else if(type == OBJ_TYPE::TYPE_MATH_MUL || type == OBJ_TYPE::TYPE_MATH_DIV || type == OBJ_TYPE::TYPE_MATH_ADD || type == OBJ_TYPE::TYPE_MATH_SUB
		|| type == OBJ_TYPE::TYPE_MATH_EQ || type == OBJ_TYPE::TYPE_MATH_NEQ || type == OBJ_TYPE::TYPE_MATH_GRT || type == OBJ_TYPE::TYPE_MATH_GRQ
		|| type == OBJ_TYPE::TYPE_MATH_LES || type == OBJ_TYPE::TYPE_MATH_LEQ   )
		{
			if(argSize < 3)
			{
				sendError(ERR_DATA::ERR_MATH_TOO_FEW_ARGS);
			}
			else if(argSize <= 4)
			{
				var2ptr = createVariableInstance(bitTagSRCB, args[2]);
				var3ptr = createVariableInstance(bitTagDEST, args[3]);

				if ( var2ptr ) //must have valid pointers
					newObj = make_shared<MathBlockOBJ>(id, type, var1ptr, var2ptr, var3ptr);
			}
		}
	}

	if(newObj)
	{
		ladderObjects.emplace_back(newObj);
	}
	return newObj;
}

OBJ_ACC_PTR PLC_Main::createRemoteClient( const String &id, const vector<String> &args )
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
                if ( getAccessorObjects()[x]->iType == OBJ_TYPE::TYPE_REMOTE )
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
	error.reserve(384);
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
