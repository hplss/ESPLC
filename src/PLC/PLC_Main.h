/*
 * PLC_Main.h
 *
 * Created: 9/28/2019 5:35:20 PM
 *  Author: Andrew Ward
  The purpose of this class is to serve as the main PLC controller object. All rungs and objects that are created by the parser are controlled by this object.
 The object will read (scan) the rungs in sequential order and report any errors along the way as needed.
  This will also serve as the interpreter for the PLC logic. Human generated logic is read from a source, then the corresponding objects/rungs are created and organized as needed.
  The parser (with the help of the PLC_Main object) will attempt to verify that there are no duplicate objects being created in memory, and will also report errors to the user interface as needed.
 */ 


#ifndef PLC_MAIN_H_
#define PLC_MAIN_H_

#include "PLC_IO.h"
#include "PLC_Rung.h"
#include <HardwareSerial.h>
#include "../CORE/UICore.h"

//other object includes
#include "OBJECTS/MATH/math_basic.h"
//


using namespace std;

extern UICore Core;

/*Remote controling of other "ESPLC" devices:
MODE 1: - The secondary device acts purely as an IO expander, where the primary device initializes ladder objects on the secondary, and sends updates to it as necessary. 
		The secondary device performs no logic processing.  
MODE 2(?): - Gather a list of available (initialized) objects already being processed on the external device. 
			This allows the external device to perform its own logic operations, and the primary device to poll the status of different ladder objects (bits,inputs,outputs,etc) 
			and use it for its own calculations. This is ideal for a decentralized "cluster" type operation. 
			
Scripting Syntax: Other devices are accessed by IP (or hostname?) and corresponding port. DEV1=EXTERNAL(<IP>) 
Ladder objects are accessed using bit operators and corresponding device ID's. 
Example(1): DEV1.<ID> -- Accessing an object directly (such as inputs and outputs)
Example(2) DEV1.<ID>.EN -- Accessing a bit of an external object.
Objects that cannot be acessed directly: MATH object types
Notes: Maybe the Web UI could perform a query across local IP ranges to find available devices automatically (only if connected to router/gateway)
It my also be possible to have another ESP device directly connect to the primary device. 
Problems: -- What triggers the seach? 
The user (explicitly via serial command or web UI). 
The script (maybe if a liost of valid devices hasn't been built?) 
If a device explicitly connects to the ESP (primary), it can easily send a signal to the primary indicating that it's an expander.

For clustered operation: multiple TCP connections can be supported for sending/receiving data.

Status broadcast port: 50000
Communication ports = 50000 + Connection #
*/



//the remoteController class represents another ESP32 device that processes its own ladder logic operations, and shares data between the current device and itself, thereby enabling tethering/cluster operations
class remoteController
{
	remoteController( shared_ptr<WiFiClient> client )
	{
		nodeClient = client;
	}
	~remoteController() //deconstructor
	{
		remoteObjects.clear();
		nodeClient->stop();
	}
	bool addRemoteObject( shared_ptr<Remote_Ladder_OBJ> obj){ remoteObjects.emplace_back(obj); return true; }
	shared_ptr<WiFiClient> getNode(){ return nodeClient; }
	//This updates an individual object
	bool updateRemoteObject()
	{
		//nodeClient->print()
		return false;
	}
	uint16_t findRemoteObjectIndexByID( uint16_t id )
	{
		for ( uint16_t x = 0; x < remoteObjects.size(); x++ )
		{
			if ( remoteObjects[x]->getID() == id )
				return x;
		}
	}
	uint16_t getNumObjects() { return remoteObjects.size(); }
	private: 
	vector<shared_ptr<Remote_Ladder_OBJ>> remoteObjects;
	shared_ptr<WiFiClient> nodeClient;
};

/* parserHelperObject serves as a container and accessor for variables that are used for establishing ladder logic object relationships in the parser. */
struct parserHelperObject
{
public:
	parserHelperObject( const String &parsed )
	{
		/*create a new ladder rung when the helper is initialized. 
		Presumably each helper represents a line being parsed, and each line represents a "rung" in the ladder logic.*/
		pRung = make_shared<Ladder_Rung>(); 
		bitOperator = false;
		bitNot = false; 
		iLastOP = CHAR_AND;
		sParsedLine = parsed;
	}

	void appendToObjName( const char c ) { sParsedObj += c; }
	void appendToBitName( const char c ){ sParsedBit += c; }
	//Resets the values typically used by the parser to their default values.
	void reset(){ sParsedObj.clear(); sParsedBit.clear(); bitNot = false; bitOperator = false; }
	//Transfers the stored objects from OR operations into the previous objects container, also clears the OR objects container.
	void orToPreviousObjects(){ pPreviousObjects = pOrObjects; pOrObjects.clear(); }
	void addPreviousObject( const shared_ptr<Ladder_OBJ_Wrapper> &ptr ){ pPreviousObjects.emplace_back(ptr); }
	void addOrObject( const shared_ptr<Ladder_OBJ_Wrapper> &ptr ){ pOrObjects.emplace_back(ptr); }
	//Attempts to create a new object wrapper using an already defined ladder object, applies necessary wrapper flags, and returns the created object.
	template <typename T>
	shared_ptr<Ladder_OBJ_Wrapper> getObjectVARWrapper(T *ptr)
	{
		shared_ptr<Ladder_VAR> pVar = ptr->getObjectVAR(sParsedBit);
		if ( !pVar ) //couldn't find it, so let's try to add it
			pVar = ptr->addObjectVAR( sParsedBit ); //try to create the VAR object
		
		if ( pVar ) //If successful, make the wrapper and return it
			return make_shared<Ladder_OBJ_Wrapper>( pVar, getNotOP() );

		return 0; //failed, return NULL
	}
	//This is responsible for generating the object wrapper by associating it with the (already initialized) object. May return a wrapper for the object itself, or an associated bit in the object.
	shared_ptr<Ladder_OBJ_Wrapper> createNewWrapper( shared_ptr<Ladder_OBJ> obj )
	{
		if ( obj ) //pointer must be valid
		{
			shared_ptr<Ladder_OBJ_Wrapper> newOBJWrapper = 0; //init
			if ( bitOperator ) //accessing a specific bit?
			{
				switch( obj->getType() ) //only valid object types that have bits need to be added here.
				{
					case OBJ_TYPE::TYPE_TOF: //Looks like we've reached a timer. Set timer bits as appropriate then move on (Timer counted as an output)
					case OBJ_TYPE::TYPE_TON:
						newOBJWrapper = getObjectVARWrapper(static_cast<TimerOBJ *>(obj.get()));
						break;
				
					case OBJ_TYPE::TYPE_CTD:
					case OBJ_TYPE::TYPE_CTU:
						newOBJWrapper = getObjectVARWrapper(static_cast<CounterOBJ *>(obj.get()));
						break;
					default:
						return 0; //not a valid object type for bit accessing
						break;
				}
			}
			else
			{
				newOBJWrapper = make_shared<Ladder_OBJ_Wrapper>( obj, getNotOP() );
			}

			if ( getRung()->addRungObject(newOBJWrapper) ) //Add to the new rung in order to perform updates on the object, post line scanning.
			{
				return newOBJWrapper; //return the new wrapper for later use.
			}
		}

		return 0; //return null
	}
	void setPreviousObject( const shared_ptr<Ladder_OBJ_Wrapper> &ptr ){ pPreviousObjects.clear(); pPreviousObjects.emplace_back(ptr); }
	void setBitOP( bool bit ){ bitOperator = bit; }
	void setNotOP( bool bit ){ bitNot = bit; }
	void setPreviousObject( const vector<shared_ptr<Ladder_OBJ_Wrapper>> &ptr )
	{
		pPreviousObjects.clear();
		for (uint8_t i = 0; i < ptr.size(); i++ )
		{
			pPreviousObjects.emplace_back(ptr[i]);
		}
	}
	void setLastOP( uint8_t op ){ iLastOP = op; }

	uint8_t getNumPreviousObjects(){ return pPreviousObjects.size(); }
	uint8_t getNumOrObjects(){ return pOrObjects.size(); }
	uint8_t getLastOP(){ return iLastOP; }
	const String &getParsedObjectStr(){ return sParsedObj; } 
	const String &getParsedBitStr(){ return sParsedBit; } 
	vector<shared_ptr<Ladder_OBJ_Wrapper>> &getPreviousObjects(){ return pPreviousObjects; }
	vector<shared_ptr<Ladder_OBJ_Wrapper>> &getOrObjects(){ return pOrObjects; }
	shared_ptr<Ladder_Rung> &getRung(){ return pRung; }
	bool getNotOP(){ return bitNot; }
	bool getBitOP(){ return bitOperator; }
	uint16_t &getLinePos() { return iLinePos; }
	uint16_t getLineLength() { return sParsedLine.length(); }
	const String &getParsedLineStr(){ return sParsedLine; }
	const char getCurrentLineChar(){ return getParsedLineStr()[getLinePos()]; }

private:
	bool bitOperator;
	bool bitNot; 
	uint8_t iLastOP; //Last operator used by the parser
	uint16_t iLinePos; //position at which the line is currently being parsed (in the string character array)
	uint16_t iLineLength; //total length of the parsed line (number of chars)
	String sParsedLine,
		   sParsedObj, //object name 
		   sParsedBit; //for bit operations on objects

	vector<shared_ptr<Ladder_OBJ_Wrapper>> pPreviousObjects, //indicate objects stored from the previous operator logic (previous object parsed and initialized)
										   pOrObjects;	//used as a container for chained LOGIC OR objects that are parsed, for the previous object to reference

	shared_ptr<Ladder_Rung> pRung; //Rung object that is being created by the parser
};
/*ladderOBJdata is mostly for tying a name (String) to an object for the brief time it matters.
* This is typically only supposed to be used during the parsing of an inputted ladder logic script, and objects of this type are deleted once the script has been parsed and object associations are established.
*/
struct ladderOBJdata 
{
	ladderOBJdata( const String &name, shared_ptr<Ladder_OBJ> createdObj )
	{
		objName = name;
		LadderOBJ = createdObj;
	}
	~ladderOBJdata()
	{ 
		#ifdef DEBUG
		Serial.println(PSTR("OBJdata Destructor")); 
		#endif
	}
	
	//Returns a pointer to the ladder object stored in this object (if applicble).
	shared_ptr<Ladder_OBJ> getObject(){ return LadderOBJ; } 
	//Returns a reference to the name (string) of the object as it has been parsed. This is only used while the parser is generating the logic functionality.
	const String &getName(){ return objName; }
	
	private:
	shared_ptr<Ladder_OBJ> LadderOBJ;
	String objName;
};

//The PLC_Main object handles the parsing of a user-inputtd logic script and functions as the central manager for all created ladder logic objects. 
class PLC_Main
{
	public:
	PLC_Main()
	{
		currentScript = make_shared<String>(); //initialize the smart pointer
		currentObjID = 1; //start at 1, becaose 0 indicates a child variable (bit operators)
		nodeMode = 0; //default to off
		generatePinMap();
	}
	~PLC_Main()
	{
		ladderRungs.clear(); //empty our vectors -- should also delete the objects once they are no longer referenced (smart pointers)
		ladderObjects.clear();
	}
	//Generates the map that stores pin data in relation to availability and capability. 
	void generatePinMap();
	//Overloaded function that parses a logic script by string reference.
	//Returns true on success.
	bool parseScript(const String &script){ return parseScript(script.c_str()); }
	//Overloaded function that parses a logic script by string pointer.
	//Returns true on success.
	bool parseScript(String *script){ return parseScript(script->c_str()); }
	//This function parses an inputted logic script and breaks it into individual lines, which ultimately create the logic objects and rungs as appropriate.
	bool parseScript(const char *);
	//Parses the individual lines for logic operations and declarations (called from parseScript())
	bool parseLine( const String & );
	//Used to parse the appropriate logic tags from the logic script and return the associated byte.
	uint8_t parseLogic( const String & );
	//Displays an error of a given type, with the option of displaying additional information.
	bool parser_ANDOP( shared_ptr<parserHelperObject> );
	bool parser_EQOP( shared_ptr<parserHelperObject> );
	bool parser_OROP( shared_ptr<parserHelperObject> );
	bool parser_Default( shared_ptr<parserHelperObject> );
	void sendError(uint8_t, const String & = ""); 
	shared_ptr<Ladder_VAR>parseVAR( const String &, uint16_t & );
	//This function is responsible for destructing all rungs and objects before reconstructing the ladder logic program.
	void resetAll(); 
	//This function adds the inputted ladder rung into the ladder rung vector.
	bool addLadderRung(shared_ptr<Ladder_Rung>);
	//Creates a new ladder object based in inputted TYPE argument (parsed from the logic script), once the arguments for each ne wobject have been parsed, the appropriate 
	//object is created, paired with its name for later reference by the parser. 
	shared_ptr<ladderOBJdata> createNewObject( shared_ptr<parserHelperObject> & );
	//Creates a new OUTPUT type object, based on the inputted arguments. Script args: [1] = output pin, [2] = NO/NC
	shared_ptr<Ladder_OBJ> createOutputOBJ( const vector<String> &);
	//Creates an input object and associates it with a name. Script args: [1] = input pin, [2] = type (analog/digital), [3] = logic
	shared_ptr<Ladder_OBJ> createInputOBJ( const vector<String> &);
	//Creates a counter object and associates it with a name. Script args: [1] = count value, [2] = accum, [3] = subtype(CTU/CTD)
	shared_ptr<Ladder_OBJ> createCounterOBJ( const vector<String> &);
	//Creates a new timer object and associates it with a name. Script args: [1] = delay(ms), [2]= accum default(ms), [3] = subtype(TON/TOF)
	shared_ptr<Ladder_OBJ> createTimerOBJ( const vector<String> &);
	//Creates a new virtual type object, which represents a stored value in memory, to be accessed by other objects such as counters or timers or comparison blocks, etc.
	shared_ptr<Ladder_OBJ> createVirtualOBJ( const vector<String> &);
	//performs a lookup to make sure the inputted pin number corresponds to a pin that is valid for the device. Used for some basic error checking in the parser. 
	//Also lets us know if a given pin is already claimed by another object. 
	//ARGS: <Pin>, <Device Type>
	bool isValidPin( uint8_t, uint8_t );
	//attempts to claim the inputted pin as "taken", thereby preventing other objects from using the pin as they are initialized.
	bool setClaimedPin( uint8_t );

	//Returns the number of current rungs stored in the rung vector.
	uint16_t getNumRungs(){ return ladderRungs.size(); }
	Ladder_OBJ_Wrapper *generateObjWrapper(const vector<ladderOBJdata *> &, const String &parsedStr );
	//Parses and returns the name of the ladder object, after removing any operators that be precede the name.
	String getObjName( const String & );
	//Returns the String object stored in the shared pointer for the logic script (stored in RAM).
	String &getScript(){ return *currentScript.get(); }
	//Returns the shared pointer object for the logic script. 
	shared_ptr<String> &getLogicScript(){ return currentScript; }
	//Returns the created ladder object that corresponds to it's unique ID
	//Args: Ladder Object Vector, Unique ID
	shared_ptr<Ladder_OBJ> findLadderObjByID( const uint16_t );
	//Returns the created ladder object that corresponds to it's name given in the logic script.
	//Args: Object Name (as define in the user inputted script)
	shared_ptr<Ladder_OBJ> findLadderObjByName( const String & );
	//Returns the created ladder object that corresponds to it's unique ID
	//Returns the ladder rung based on its index in the rung vector.
	//potentially unsafe? Hmm
	//Args: Index 
	shared_ptr<Ladder_Rung>getLadderRung(uint16_t rung){ if (rung > ladderRungs.size()) return 0; return ladderRungs[rung]; } //potentially unsafe? Hmm
	
	vector<shared_ptr<Ladder_Rung>> &getLadderRungs(){ return ladderRungs; }
	vector<shared_ptr<Ladder_OBJ>> &getLadderObjects(){ return ladderObjects; }
	//This is the main process loop that handles all logic operations
	void processLogic(); 
		
	private:
	vector<shared_ptr<Ladder_Rung>> ladderRungs; //Container for all ladder rungs present in the parsed ladder logic script.
	vector<shared_ptr<Ladder_OBJ>> ladderObjects; //Container for all ladder objects present in the parsed ladder logic script. Used for easy status query.
	shared_ptr<String> currentScript; //save the current script in RAM?.. Hmm..
	uint16_t currentObjID;
	uint8_t nodeMode; //Networking mode - 0 = disabled. 1 = dependent, 2 = cluster
	vector<shared_ptr<ladderOBJdata>> parsedLadderObjects; //container for information for all ladder objects created by the parser -- sleared after parsing has completed

	std::map<uint8_t, uint8_t> pinMap;
};





#endif /* PLC_MAIN_H_ */