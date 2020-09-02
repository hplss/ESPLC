/*
 * PLC_IO.h
 *
 * Created: 9/22/2019 4:26:05 PM
 * Author: Andrew Ward
 * This file is dedicated to housing all of the function declarations that are responsible for handling all PLC logic IO functionality.
 */ 


#ifndef PLC_IO_H_
#define PLC_IO_H_

#include <vector>
#include <esp32-hal-gpio.h>
#include <HardwareSerial.h>
#include "../CORE/GlobalDefs.h"
#include "../CORE/Time.h"
#include <map>
#include <memory>

using namespace std;

//predefine to prevent some linker problems
class Ladder_VAR;
//
//This is the base class for all PLC ladder logic objects. Individual object types derive from this class.
class Ladder_OBJ
{
public:
	Ladder_OBJ( const String &id, uint8_t type ){ s_ObjID = id; iType = type; }//start off
	virtual ~Ladder_OBJ(){ }
	//Sets the unique ID number for the object, possibly for future reference by the program if needed. ARGS: <ID>
	bool setState(uint8_t state) { objState = state; return true; }
	//Sets the PLC Ladder Logic object type. This tells us whether the object is an INPUT/OUTPUT,TIMER, etc.
	void setType(uint8_t type) { iType = type; }
	virtual void setLineState(bool &state){ bLineState = state; } //save the state. Possibly consider latching the state if state is HIGH (duplicate outputs?)
	void setLogic(uint8_t logic) { objLogic = logic; }

	//Returns enabled/disabled
	uint8_t getState(){ return objState; } 
	uint8_t getLogic() { return objLogic; }
	//Returns the object type (OUTPUT/INPUT/TIMER,etc.)
	uint8_t getType(){ return iType; }
	bool getLineState(){ return bLineState; }
	//Returns the unique object ID
	const String &getID(){ return s_ObjID; }
	//Set the line state back to false for the next scan This should only be called by the rung manager (which applies the logic after processing)
	virtual void updateObject(){ bLineState = false;} 
	//Returns an object's bit (Ladder_VAR pointer) based on an inputted bit ID string
	virtual shared_ptr<Ladder_VAR> getObjectVAR( const String & );
	virtual shared_ptr<Ladder_VAR> addObjectVAR( const String & );
	
private:
	uint8_t iType; //Identifies the type of this object. 0 = input, 1 = Physical output, 2 = Virtual Output, 3 = timer, etc.	
	uint8_t objState; //Enabled or disabled? needed?
	uint8_t objLogic; //Is it normally closed or normally open? (for example)
	String s_ObjID; //The unique ID for this object (globally)
	bool bLineState; 
	//Bit shifting to save memory? Look into later
};

//This object serves as a means of storing logic script specific flags that pertain to a single ladder object. 
//This allows us to perform multiple varying logic operations without the need to create multiple copies of the same object.
struct Ladder_OBJ_Wrapper 
{
	Ladder_OBJ_Wrapper(shared_ptr<Ladder_OBJ> obj, uint16_t rung, bool not_flag = false)
	{
		bNot = not_flag; //Exclusively for NOT logic
		ladderOBJ = obj; 
		i_rungNum = rung; //store this here for now
	}
	~Ladder_OBJ_Wrapper(){ }

	//Adds the inputted object to the current object's list.
	bool addNextObject( shared_ptr<Ladder_OBJ_Wrapper> pObj )
	{
		nextObjects.push_back(pObj);
		return true; 
	}
	bool addNextObject( const vector<shared_ptr<Ladder_OBJ_Wrapper>> &pObj )
	{
		for ( uint8_t x = 0; x < pObj.size(); x++ )
			nextObjects.push_back(pObj[x]);

		return true; 
	}

	void setLineState(bool state)
	{
		getObject()->setLineState(state); //line state needs to be set per wrapper, not per ladder object

		for(uint8_t x = 0; x < nextObjects.size(); x++ )
			nextObjects[x]->setLineState(state);
	}
	
	//Returns the pointer to the ladder object stored by this object.
	shared_ptr<Ladder_OBJ> &getObject(){ return ladderOBJ; }
	//Tells us if the object is being interpreted using NOT logic
	bool getNot(){ return bNot; }
		
	private:
	bool bNot; //if the object is using not logic (per instance in rungs)
	uint16_t i_rungNum;
	shared_ptr<Ladder_OBJ> ladderOBJ; //Container for the actual Ladder_Obj object
	vector<shared_ptr<Ladder_OBJ_Wrapper>> nextObjects;
};



#endif /* PLC_IO_H_ */