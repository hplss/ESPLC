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
	Ladder_OBJ( const String &id, uint8_t type ){ s_ObjID = id; i_Type = type; i_objState = 0; }
	virtual ~Ladder_OBJ(){ }
	//Sets the state of the Ladder_OBJ
	void setState(uint8_t state) { i_objState = state; }
	//Returns enabled/disabled/etc
	uint8_t getState(){ return i_objState; } 
	//Returns the object type identifier (OUTPUT/INPUT/TIMER,etc.)
	const uint8_t getType(){ return i_Type; }
	//Returns the unique object ID
	const String &getID(){ return s_ObjID; }

private:
	uint8_t i_Type, //Identifies the type of this object. 0 = input, 1 = Physical output, 2 = Virtual Output, 3 = timer, etc.	
			i_objState; //Enabled or disabled?
	String s_ObjID; //The unique ID for this object (globally)
};

//Ladder_OBJ_Logical objects are a subclass of Ladder_OBJ. These are objects that are used in performing logic operations via the logic script. 
class Ladder_OBJ_Logical : public Ladder_OBJ
{
	public:
	Ladder_OBJ_Logical( const String &id, uint8_t type ) : Ladder_OBJ( id, type ) {}
	~Ladder_OBJ_Logical(){}
	virtual void setLineState(bool &state, bool bNot){ if (state) b_lineState = state; } //save the state. Possibly consider latching the state if state is HIGH (duplicate outputs?)
	//Returns the currently stored line state for the given object.
	bool getLineState(){ return b_lineState; }
	//Returns the logic type of the object. EX: Normally Open, Normally closed, etc.
	const uint8_t getLogic() { return i_objLogic; }

	//Sets the logic type for the given object. EX: Normally Open, Normally closed, etc.
	void setLogic(uint8_t logic) { i_objLogic = logic; }
	//Set the line state back to false for the next scan This should only be called by the rung manager (which applies the logic after processing)
	virtual void updateObject(){ b_lineState = false; } 

	//Returns an object's bit (Ladder_VAR pointer) based on an inputted bit ID string
	virtual shared_ptr<Ladder_VAR> getObjectVAR( const String & );
	//Adds a Ladder_VAR object to the logical object based on the given identifying string. Must pass the appropriate checks before it is initialized.
	virtual shared_ptr<Ladder_VAR> addObjectVAR( const String & );

	private:
	uint8_t i_objLogic;
	bool b_lineState;
};

//Ladder_OBJ_Accessor objects are a subclass of Ladder_OBJ. These are objects that are used as a means of communicating and managing other peripherals that 
//contain other Ladder_OBJ_Logical objects. Examples of this are networked ESPLC clients via serial/wifi/etc. where a remote host is capable of sending data in a format
//that can be parsed and interpreted.
class Ladder_OBJ_Accessor : public Ladder_OBJ 
{
	public:
	Ladder_OBJ_Accessor( const String &id, uint8_t type ) : Ladder_OBJ( id, type ){}
	~Ladder_OBJ_Accessor()
	{
		getObjects().clear();
	}

	virtual void updateObject(){}

	void handleUpdates( const vector<String> &);
	void handleUpdates( const String & );

	//This function handles the initialization of an object that does not already exist in the accessor.
	shared_ptr<Ladder_OBJ_Logical> handleInit( const String &);

	bool addObject(shared_ptr<Ladder_OBJ_Logical> obj) { getObjects().push_back(obj); return true;}

	vector<shared_ptr<Ladder_OBJ_Logical>> getObjects(){ return accessorObjects; }

	//Returns the locally stored Ladder_OBJ copy as it pertains to the remote client.
	virtual shared_ptr<Ladder_OBJ_Logical> findLadderObjByID( const String &id )
	{
		for ( uint16_t x = 0; x < getNumObjects(); x++ )
		{
			if ( getObjects()[x]->getID() == id )
				return getObjects()[x];
		}
		
		return 0;
	}
	//Returns the number of locally stored remote objects for a given client.
	const uint16_t getNumObjects() { return getObjects().size(); }

	private:
	vector<shared_ptr<Ladder_OBJ_Logical>> accessorObjects; //Storage for any initialized ladder objects on the remote client.
};

//This object serves as a means of storing logic script specific flags that pertain to a single ladder object. 
//This allows us to perform multiple varying logic operations without the need to create multiple copies of the same object.
struct Ladder_OBJ_Wrapper 
{
	Ladder_OBJ_Wrapper(shared_ptr<Ladder_OBJ_Logical> obj, uint16_t rung, bool not_flag = false)
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
		getObject()->setLineState(state, getNot()); //line state needs to be set per wrapper, not per ladder object
		
		for(uint8_t x = 0; x < nextObjects.size(); x++ )
			nextObjects[x]->setLineState(state);
	}
	
	//Returns the pointer to the ladder object stored by this object.
	const shared_ptr<Ladder_OBJ_Logical> getObject(){ return ladderOBJ; }
	//Tells us if the object is being interpreted using NOT logic
	bool getNot(){ return bNot; }
		
	private:
	bool bNot; //if the object is using not logic (per instance in rungs)
	uint16_t i_rungNum;
	shared_ptr<Ladder_OBJ_Logical> ladderOBJ; //Container for the actual Ladder_Obj object
	vector<shared_ptr<Ladder_OBJ_Wrapper>> nextObjects;
};



#endif /* PLC_IO_H_ */