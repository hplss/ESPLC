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

//The PLC_Remote object is responsible for serving as a container for the local broadcast server that status polls are sent to, as well as the initialized clients.
//ARGS: <Broadcast Port>,<Autoconnect IP's>, <Net Mode>
class PLC_Remote_Server
{
	public:
	PLC_Remote_Server( uint16_t );
	~PLC_Remote_Server();

	uint16_t getPort(){ return i_Port; }
	//When a new client attempts to conenect to this device, this function functions as a "hand-shake" to let the client know that the connection was successful.
	void processRequests();
	bool clientExists( const WiFiClient &);
	//Called first to determine what the client wants.
	String handleRequest( const String & ); 
	//This handles the compiling of a string that contains all information necessary to initialize a new object for later updating on the remote client's end.
	String handleInit( const String & );
	//This handles the compiling of a string that contains all information necessary to update objects that are already initialized on the remote client.
	String handleUpdate( const String & );

	private:
	shared_ptr<WiFiServer> localServer; //The actual WiFiServer object

	vector<IPAddress> localClients;
	uint16_t i_Port;
};


//The PLC_Main object handles the parsing of a user-inputtd logic script and functions as the central manager for all created ladder logic objects. 
class PLC_Main
{
	public:
	PLC_Main()
	{
		currentScript = make_shared<String>(); //initialize the smart pointer
	}
	~PLC_Main()
	{
		ladderRungs.clear(); //empty our vectors -- should also delete the objects once they are no longer referenced (smart pointers)
		ladderObjects.clear();
		accessorObjects.clear();
		ladderVars.clear();
	}
	//Generates the map that stores pin data in relation to availability and capability. 
	void generatePinMap();
	//Generates the map used for determining the available PWM output channels that can be used by an output (if applicable).
	void generatePWMMap();
	//This is used to reserve an available PWM channel from the PWM map. A return value of -1 indicatces a failure
	int8_t reservePWMChannel();

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
	void sendError(ERR_DATA, const String & = ""); 
	//This function is responsible for destructing all rungs and objects before reconstructing the ladder logic program.
	void resetAll(); 
	//This function adds the inputted ladder rung into the ladder rung vector.
	bool addLadderRung(shared_ptr<Ladder_Rung>);
	//Creates a new ladder object based in inputted TYPE argument (parsed from the logic script), once the arguments for each ne wobject have been parsed, the appropriate 
	//object is created, paired with its name for later reference by the parser. 
	shared_ptr<Ladder_OBJ> createNewLadderObject( const String &, const vector<String> &);
	//Creates a new OUTPUT type object, based on the inputted arguments. Script args: [1] = output pin, [2] = NO/NC
	shared_ptr<Ladder_OBJ_Logical> createOutputOBJ( const String &, const vector<String> &);
	//Creates an input object and associates it with a name. Script args: [1] = input pin, [2] = type (analog/digital), [3] = logic
	shared_ptr<Ladder_OBJ_Logical> createInputOBJ( const String &, const vector<String> &);
	//Creates a counter object and associates it with a name. Script args: [1] = count value, [2] = accum, [3] = subtype(CTU/CTD)
	shared_ptr<Ladder_OBJ_Logical> createCounterOBJ( const String &, const vector<String> &);
	//Creates a new timer object and associates it with a name. Script args: [1] = delay(ms), [2]= accum default(ms), [3] = subtype(TON/TOF)
	shared_ptr<Ladder_OBJ_Logical> createTimerOBJ( const String &, const vector<String> &);
	//Creates a new basic math object, which is capable of performing a series of simple calculations based on inputted arguments.

	//shared_ptr<Ladder_OBJ> createMathOBJ( const String &, const vector<String> &);
	//Creates a oneshot object, which pulses HIGH for one cycle, then LOW until reset. Declared inline.
	shared_ptr<Ladder_OBJ_Logical> createOneshotOBJ();
    //Creates a new basic math object, which is capable of performing a series of simple calculations based on inputted arguments.
	shared_ptr<Ladder_OBJ_Logical> createMathOBJ( const String &, const vector<String> &);
	//Creates a new virtual type object, which represents a stored value in memory, to be accessed by other objects such as counters or timers or comparison blocks, etc.
	shared_ptr<Ladder_OBJ_Logical> createVariableOBJ( const String &, const vector<String> &);
	//Creates a ladder object reference that represents the current state of an object that is initialized on another ESPLC device.
	shared_ptr<Ladder_OBJ_Accessor> createRemoteClient( const String &, const vector<String> &);
	//performs a lookup to make sure the inputted pin number corresponds to a pin that is valid for the device. Used for some basic error checking in the parser. 
	//Also lets us know if a given pin is already claimed by another object. 
	//ARGS: <Pin>, <Device Type>
	bool isValidPin( uint8_t, OBJ_TYPE );
	//Sets up the remote PLC server
	bool createRemoteServer( uint16_t );
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
	shared_ptr<Ladder_OBJ_Logical> findLadderObjByID( const String & );
	//Returns the created accessor object that corresponds to it's unique ID
	//Args: Ladder Object Vector, Unique ID
	shared_ptr<Ladder_OBJ_Accessor> findAccessorByID( const String & );
	//Returns the created variable object that corresponds to it's unique ID
	//Args: Ladder Var Vector, Unique ID
	shared_ptr<Ladder_VAR> findLadderVarByID( const String & );

	//This function scan for nodes on the given port
	vector<IPAddress> scanForRemoteNodes( uint16_t, uint8_t, uint8_t, uint16_t );

	
	//Returns a reference to the local storage container for all locally stored initialized logic rungs.
	vector<shared_ptr<Ladder_Rung>> &getLadderRungs(){ return ladderRungs; }
	//Returns a reference to the local storage container for initialized Ladder_OBJ_Logical objects.
	vector<shared_ptr<Ladder_OBJ_Logical>> &getLadderObjects(){ return ladderObjects; }
	//Returns a reference to the local storage container for initialized Ladder_VAR objects.
	vector<shared_ptr<Ladder_VAR>> &getLadderVars(){ return ladderVars; }
	//Returns the locally stored pointer to the PLC web status server.
	unique_ptr<PLC_Remote_Server> &getRemoteServer(){ return remoteServer; }
	//Returns a reference to the local storage container for initialized Ladder_OBJ_Accessor objects.
	vector<shared_ptr<Ladder_OBJ_Accessor>> &getAccessorObjects() { return accessorObjects; }
	//Returns the number of accessors in the locally stored accessor container.
	uint8_t getNumAccessors() { return accessorObjects.size(); }

	//This is the main process loop that handles all logic operations
	void processLogic(); 
		
	private:
	vector<shared_ptr<Ladder_Rung>> ladderRungs; //Container for all ladder rungs present in the parsed ladder logic script.

	vector<shared_ptr<Ladder_OBJ_Logical>> ladderObjects; //Container for all Ladder_OBJ_Logical objects present in the parsed ladder logic script. Used for easy status query.
	vector<shared_ptr<Ladder_OBJ_Accessor>> accessorObjects; //Container for all Ladder_OBJ_Accessor objects present in the larsed ladder logic script.
	vector<shared_ptr<Ladder_VAR>> ladderVars; //Container for all ladder variables present in the parsed ladder logic script. Used for easy status query.
	
	shared_ptr<String> currentScript; //save the current script in RAM?.. Hmm..

	unique_ptr<PLC_Remote_Server> remoteServer; //PLC_Remote_Server object
	
	std::map<uint8_t, PIN_TYPE> pinMap; //This map stores information about which physical pins are available on the ESP32 that IO can use.
	std::map<uint8_t, PWM_STATUS> pwmMap; //This map stores information about the available PWM channels that a newly declared output can use. 
};

//Generic functions here
char toUpper( char x );
String &toUpper(String &);
extern PLC_Main PLCObj;

#endif /* PLC_MAIN_H_ */