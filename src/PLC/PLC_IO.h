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
class Ladder_OBJ_Logical;
class Ladder_OBJ_Accessor;
class Ladder_OBJ_Wrapper;
class NestContainer;
//

using OBJ_LOGIC_PTR = shared_ptr<Ladder_OBJ_Logical>;
using OBJ_ACC_PTR = shared_ptr<Ladder_OBJ_Accessor>;
using OBJ_WRAPPER_PTR = shared_ptr<Ladder_OBJ_Wrapper>;
using NEST_PTR = shared_ptr<NestContainer>;
using VAR_PTR = shared_ptr<Ladder_VAR>;




//This is the base class for all PLC ladder logic objects. Individual object types derive from this class.
class Ladder_OBJ
{
public:
	Ladder_OBJ( const String &id, OBJ_TYPE type ) : sObjID(id), iType(type)
	{  
		iState = 0; 
	}
	virtual ~Ladder_OBJ(){ }

	//Returns an object's bit (Ladder_VAR pointer) based on an inputted bit ID string
	virtual VAR_PTR getObjectVAR( const String & );
	//This function returns a reference to the object's local variable storage container (this may or may not be used, depending on the object's type).
	vector<VAR_PTR> &getObjectVARs(){ return localVars; }

	const String sObjID; //The unique ID for this object (globally)
	const OBJ_TYPE iType; //Identifies the type of this object. 0 = input, 1 = Physical output, 2 = Virtual Output, 3 = timer, etc.	
	uint8_t iState; //Enabled or disabled?
private:
	vector<VAR_PTR> localVars; //locally stored ladder var objects (that belong to this object)
};

//Ladder_OBJ_Logical objects are a subclass of Ladder_OBJ. These are objects that are used in performing logic operations via the logic script. 
class Ladder_OBJ_Logical : public Ladder_OBJ
{
	public:
	Ladder_OBJ_Logical( const String &id, OBJ_TYPE type ) : Ladder_OBJ( id, type ) {}
	~Ladder_OBJ_Logical(){}
	virtual void setLineState(bool &state, bool bNot){ if (state) b_lineState = state; } //save the state. Possibly consider latching the state if state is HIGH (duplicate outputs?)
	//Returns the currently stored line state for the given object.
	bool getLineState() const { return b_lineState; }
	//Set the line state back to false for the next scan This should only be called by the rung manager (which applies the logic after processing)
	virtual void updateObject(){ b_lineState = false; } 
	
	private:
	bool b_lineState;
};

//Ladder_OBJ_Accessor objects are a subclass of Ladder_OBJ. These are objects that are used as a means of communicating and managing other peripherals that 
//contain other Ladder_OBJ_Logical objects. Examples of this are networked ESPLC clients via serial/wifi/etc. where a remote host is capable of sending data in a format
//that can be parsed and interpreted.
class Ladder_OBJ_Accessor : public Ladder_OBJ 
{
	public:
	Ladder_OBJ_Accessor( const String &id, OBJ_TYPE type ) : Ladder_OBJ( id, type ){}
	~Ladder_OBJ_Accessor()
	{
		accessorVars.clear();
	}

	virtual void updateObject(){}

	void handleUpdates( const vector<String> &);
	void handleUpdates( const String & );

	//This function handles the initialization of an object(s) that does not already exist in the accessor, essentially parsing the reply from a server that is sent after an init request.
	shared_ptr<Ladder_OBJ_Logical> handleInit( const String &);

	bool addObject(OBJ_LOGIC_PTR obj) { accessorVars.push_back(obj); return true;}

	const vector<OBJ_LOGIC_PTR> &getAccessorVars(){ return accessorVars; }

	//Returns the locally stored Ladder_OBJ copy as it pertains to the remote client.
	virtual OBJ_LOGIC_PTR findAccessorVarByID( const String &id )
	{
		for ( uint16_t x = 0; x < getNumObjects(); x++ )
		{
			if ( getAccessorVars()[x]->sObjID == id )
				return getAccessorVars()[x];
		}
		
		return 0;
	}
	//Returns the number of locally stored remote objects for a given client.
	const uint16_t getNumObjects() { return getAccessorVars().size(); }

	private:
	vector<OBJ_LOGIC_PTR> accessorVars; //Storage for any initialized ladder objects on the remote client.
};

//This object serves as a means of storing logic script specific flags that pertain to a single ladder object. 
//This allows us to perform multiple varying logic operations without the need to create multiple copies of the same object.
struct Ladder_OBJ_Wrapper 
{
	Ladder_OBJ_Wrapper(OBJ_LOGIC_PTR obj, uint16_t rung, bool not_flag = false) : bNot(not_flag), pObj(obj)
	{
		//i_rungNum = rung; //store this here for now
	}
	~Ladder_OBJ_Wrapper(){ }

	//Adds the inputted object to the current object's list.
	bool addNextObject( const OBJ_WRAPPER_PTR obj )
	{
		nextObjects.push_back(obj);
		return true; 
	}
	bool addNextObject( const vector<OBJ_WRAPPER_PTR> &obj )
	{
		for ( uint8_t x = 0; x < obj.size(); x++ )
			nextObjects.push_back(obj[x]);

		return true; 
	}

	void setLineState(bool state)
	{
		pObj->setLineState(state, bNot); //line state needs to be set per wrapper, not per ladder object
		
		for(uint8_t x = 0; x < nextObjects.size(); x++ )
			nextObjects[x]->setLineState(state);
	}
		
	const OBJ_LOGIC_PTR pObj = 0; //Container for the actual Ladder_Obj object
	const bool bNot = false; //if the object is using not logic (per instance in rungs)

	private:
	//uint16_t i_rungNum;
	vector<OBJ_WRAPPER_PTR> nextObjects;
};



#endif /* PLC_IO_H_ */