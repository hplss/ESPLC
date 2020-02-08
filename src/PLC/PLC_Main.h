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
	bool parseScript(const String &script){ return parseScript(script.c_str()); }
	bool parseScript(String *script){ return parseScript(script->c_str()); }
	bool parseScript(const char *);
	uint8_t parseLogic( const String & );
	void sendError(uint8_t, const String & = ""); //Displays an error of a given type, with the option of displaying additional information.
	shared_ptr<Ladder_VAR>parseVAR( const String &, uint16_t & );
	template <typename T>
	T &getObjectBit(String &);
	void resetAll(); //This function is responsible for destructing all rungs and objects before reconstructing the ladder logic program.
	bool addLadderRung(shared_ptr<Ladder_Rung>);
	shared_ptr<ladderOBJdata> createNewObject( const String &, const String &, uint16_t &, uint16_t &);
	uint16_t getNumRungs(){ return ladderRungs.size(); }
	Ladder_OJB_Wrapper *generateObjWrapper(const vector<ladderOBJdata *> &, const String &parsedStr );
	String getObjName( const String & );
	String &getScript(){ return *currentScript.get(); }
	shared_ptr<String> &getSharedScript(){ return currentScript; }
	shared_ptr<ladderOBJdata> findLadderObjByID( const vector<shared_ptr<ladderOBJdata>> &, const uint16_t ); //search for this object in rungs?
	shared_ptr<ladderOBJdata> findLadderObjByName( const vector<shared_ptr<ladderOBJdata>> &, const String & );
	shared_ptr<Ladder_OBJ> findLadderObjByID( const uint16_t );
	shared_ptr<Ladder_Rung>getLadderRung(uint16_t rung){ return ladderRungs[rung]; } //potentially unsafe? Hmm
		
	void processLogic(); //This is the main process loop that handles all logic operations
		
	private:
	vector<shared_ptr<Ladder_Rung>> ladderRungs;
	vector<shared_ptr<Ladder_VAR>> ladderVARs;
	shared_ptr<String> currentScript; //save the current script in RAM?.. Hmm..
};





#endif /* PLC_MAIN_H_ */