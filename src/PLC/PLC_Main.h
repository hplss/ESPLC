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

using namespace std;

extern UICore Core;

enum ERR_DATA
{
	CREATION_FAILED = 0,
	UNKNOWN_TYPE,
	UNKNOWN_ARGS,
	INSUFFICIENT_ARGS,
	INVALID_OBJ,
	INVALID_BIT
};

/*ladderOBJdata is mostly for tying a name (String) to an object for the brief time it matters.
* This is typically only supposed to be used during the parsing of an inputted ladder logic script.
*/
struct ladderOBJdata 
{
	ladderOBJdata( const String &name, shared_ptr<Ladder_OBJ> createdObj )
	{
		objName = name;
		LadderOBJ = createdObj;
		type = LADDER_OBJECT;
	}
	ladderOBJdata( const String &name, shared_ptr<Ladder_VAR> createdObj )
	{
		objName = name;
		LadderVAR = createdObj;
		type = LADDER_VARIABLE;
	}
	~ladderOBJdata()
	{ 
		#ifdef DEBUG
		Serial.println(PSTR("OBJdata Destructor")); 
		#endif
	}

	void addNextObject( uint16_t rung, shared_ptr<ladderOBJdata> pObj )
	{
		if ( type != LADDER_OBJECT ) //only works for ladder objects, not variables
			return;
			
		LadderOBJ->addNextObject(rung, pObj->getObject());
	}
	
	uint8_t getType(){ return type; }
	shared_ptr<Ladder_OBJ> getObject(){ return LadderOBJ; }
	shared_ptr<Ladder_VAR> getVariable(){ return LadderVAR; }
	const String &getName(){ return objName; }
	
	private:
	shared_ptr<Ladder_OBJ> LadderOBJ;
	shared_ptr<Ladder_VAR> LadderVAR;
	String objName;
	uint8_t type : 2;
};

class PLC_Main
{
	public:
	PLC_Main()
	{
		currentScript = make_shared<String>(); //initialize the smart pointer
	}
	~PLC_Main()
	{
		ladderRungs.clear();
		ladderVARs.clear();
	}
	//Overloaded function that parses a logic script by string reference.
	//Returns true on success.
	bool parseScript(const String &script){ return parseScript(script.c_str()); }
	//Overloaded function that parses a logic script by string pointer.
	//Returns true on success.
	bool parseScript(String *script){ return parseScript(script->c_str()); }
	//This function parses an inputted logic script and creates the logic objects and rungs as appropriate.
	bool parseScript(const char *);
	uint8_t parseLogic( const String & );
	//Displays an error of a given type, with the option of displaying additional information.
	void sendError(uint8_t, const String & = ""); 
	shared_ptr<Ladder_VAR>parseVAR( const String &, uint16_t & );
	template <typename T>
	T &getObjectBit(String &);
	//This function is responsible for destructing all rungs and objects before reconstructing the ladder logic program.
	void resetAll(); 
	//This function adds the inputted ladder rung into the ladder rung vector.
	bool addLadderRung(shared_ptr<Ladder_Rung>);
	//Creates a new ladder object (more info needed here).
	shared_ptr<ladderOBJdata> createNewObject( const String &, const String &, uint16_t &, uint16_t &);
	//Returns the number of current rungs stored in the rung vector.
	uint16_t getNumRungs(){ return ladderRungs.size(); }
	Ladder_OJB_Wrapper *generateObjWrapper(const vector<ladderOBJdata *> &, const String &parsedStr );
	//Parses and returns the name of the ladder object, after removing any operators that be precede the name.
	String getObjName( const String & );
	//Returns the String object stored in the shared pointer for the logic script (stored in RAM).
	String &getScript(){ return *currentScript.get(); }
	//Returns the shared pointer object for the logic script. 
	shared_ptr<String> &getSharedScript(){ return currentScript; }
	//Returns the created ladder object that corresponds to it's unique ID
	//Args: Ladder Object Vector, Unique ID
	shared_ptr<ladderOBJdata> findLadderObjByID( const vector<shared_ptr<ladderOBJdata>> &, const uint16_t );
	//Returns the created ladder object that corresponds to it's name given in the logic script.
	//Args: Ladder Object Vector, Object Name
	shared_ptr<ladderOBJdata> findLadderObjByName( const vector<shared_ptr<ladderOBJdata>> &, const String & );
	//Returns the created ladder object that corresponds to it's unique ID
	//Args: Unique ID
	shared_ptr<Ladder_OBJ> findLadderObjByID( const uint16_t );
	//Returns the ladder rung based on its index in the rung vector.
	//potentially unsafe? Hmm
	//Args: Index
	shared_ptr<Ladder_Rung>getLadderRung(uint16_t rung){ return ladderRungs[rung]; } //potentially unsafe? Hmm
		
	//This is the main process loop that handles all logic operations
	void processLogic(); 
		
	private:
	vector<shared_ptr<Ladder_Rung>> ladderRungs;
	vector<shared_ptr<Ladder_VAR>> ladderVARs;
	shared_ptr<String> currentScript; //save the current script in RAM?.. Hmm..
};





#endif /* PLC_MAIN_H_ */