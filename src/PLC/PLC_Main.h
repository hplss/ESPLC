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
#include "PLC_Parser.h"
#include <HardwareSerial.h>
#include "../CORE/UICore.h"
#include "./OBJECTS/obj_remote.h"

using namespace std;

extern UICore Core;

/*Remote controlling of other "ESPLC" devices:
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
The script (maybe if a list of valid devices hasn't been built?) 
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
	uint16_t findRemoteObjectIndexByID( const String &id )
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


//The PLC_Main object handles the parsing of a user-inputtd logic script and functions as the central manager for all created ladder logic objects. 
class PLC_Main
{
	public:
	PLC_Main()
	{
		currentScript = make_shared<String>(); //initialize the smart pointer
		nodeMode = 0; //default to off
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
	//Used to parse the appropriate logic tags from the logic script and return the associated byte.
	uint8_t parseLogic( const String & );
	//Used to send specific errors to both the web interface, as well as the serial.
	void sendError(uint8_t, const String & = ""); 
	//This function is responsible for destructing all rungs and objects before reconstructing the ladder logic program.
	void resetAll(); 
	//This function adds the inputted ladder rung into the ladder rung vector.
	bool addLadderRung(shared_ptr<Ladder_Rung>);
	//Creates a new ladder object based in inputted TYPE argument (parsed from the logic script), once the arguments for each ne wobject have been parsed, the appropriate 
	//object is created, paired with its name for later reference by the parser. 
	shared_ptr<Ladder_OBJ> createNewObject( const String &, const vector<String> &);
	//Creates a new OUTPUT type object, based on the inputted arguments. Script args: [1] = output pin, [2] = NO/NC
	shared_ptr<Ladder_OBJ> createOutputOBJ( const String &, const vector<String> &);
	//Creates an input object and associates it with a name. Script args: [1] = input pin, [2] = type (analog/digital), [3] = logic
	shared_ptr<Ladder_OBJ> createInputOBJ( const String &, const vector<String> &);
	//Creates a counter object and associates it with a name. Script args: [1] = count value, [2] = accum, [3] = subtype(CTU/CTD)
	shared_ptr<Ladder_OBJ> createCounterOBJ( const String &, const vector<String> &);
	//Creates a new timer object and associates it with a name. Script args: [1] = delay(ms), [2]= accum default(ms), [3] = subtype(TON/TOF)
	shared_ptr<Ladder_OBJ> createTimerOBJ( const String &, const vector<String> &);
	//Creates a new basic math object, which is capable of performing a series of simple calculations based on inputted arguments.
	shared_ptr<Ladder_OBJ> createMathOBJ( const String &, const vector<String> &);
	//Creates a oneshot object, which pulses HIGH for one cycle, then LOW until reset. Declared inline.
	shared_ptr<Ladder_OBJ> createOneshotOBJ( const String &, const vector<String> &);
	//Creates a new virtual type object, which represents a stored value in memory, to be accessed by other objects such as counters or timers or comparison blocks, etc.
	shared_ptr<Ladder_OBJ> createVariableOBJ( const String &, const vector<String> &);
	//performs a lookup to make sure the inputted pin number corresponds to a pin that is valid for the device. Used for some basic error checking in the parser. 
	//Also lets us know if a given pin is already claimed by another object. 
	//ARGS: <Pin>, <Device Type>
	bool isValidPin( uint8_t, uint8_t );
	//attempts to claim the inputted pin as "taken", thereby preventing other objects from using the pin as they are initialized.
	bool setClaimedPin( uint8_t );

	//Returns the number of current rungs stored in the rung vector.
	uint16_t getNumRungs(){ return ladderRungs.size(); }
	//Parses and returns the name of the ladder object, after removing any operators that be precede the name.
	String getObjName( const String & );
	//Returns the String object stored in the shared pointer for the logic script (stored in RAM).
	String &getScript(){ return *currentScript.get(); }
	//Returns the shared pointer object for the logic script. 
	shared_ptr<String> &getLogicScript(){ return currentScript; }
	//Returns the created ladder object that corresponds to it's unique ID
	//Args: Ladder Object Vector, Unique ID
	shared_ptr<Ladder_OBJ> findLadderObjByID( const String & );
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
	uint8_t nodeMode; //Networking mode - 0 = disabled. 1 = dependent, 2 = cluster

	std::map<uint8_t, uint8_t> pinMap;
	//std::map<const String &, function<shared_ptr<ladderOBJdata>(const vector<String> &)>> creationMap; //not used yet
};

//Generic functions here
char toUpper( char x );
String &toUpper(String &strn);
extern PLC_Main PLCObj;

#endif /* PLC_MAIN_H_ */