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
struct Ladder_OBJ_Wrapper; 
class Ladder_VAR;
//

//This is the base class for all PLC ladder logic objects. Individual object types derive from this class.
class Ladder_OBJ
{
public:
	Ladder_OBJ( uint16_t &id, uint8_t type ){ ObjID = id; iType = type; bLineState = false; }//start off
	virtual ~Ladder_OBJ(){ nextObj.clear(); }
	//Sets the unique ID number for the object, possibly for future reference by the program if needed. ARGS: <ID>
	void setObjID(uint16_t id) { ObjID = id; }
	bool setState(uint8_t state) { objState = state; return true; }
	//Sets the PLC Ladder Logic object type. This tells us whether the object is an INPUT/OUTPUT,TIMER, etc.
	void setType(uint8_t type) { iType = type; }
	virtual void setLineState(uint16_t rung, bool state){ if(state) bLineState = state; getNextObj( rung ); } //latch state high if applied, then find the next connected object in the rung.
	void setLogic(uint8_t logic) { objLogic = logic; }
	//Add the potential objects that are next in the chain, based on Rung number. ARGS: <rung number>, <Object>
	bool addNextObject( uint16_t, shared_ptr<Ladder_OBJ_Wrapper> ); 
	bool addNextObject( uint16_t, vector<shared_ptr<Ladder_OBJ_Wrapper>> &);
	
	//This function gets the next ladder logic object and checks to see if it passes the logic check.
	void getNextObj( uint16_t ); 
	uint8_t getState(){ return objState; } //Returns enabled/disabled
	uint8_t getLogic() { return objLogic; }
	//Returns the object type (OUTPUT/INPUT/TIMER,etc.)
	uint8_t getType(){ return iType; }
	//Returns the current state of the "line" (or ladder rung), which is computed based on the logic operations of the preceding object. This serves as a foundation for the current object's logic operations.
	bool getLineState(){ return bLineState; }
	//Returns the unique object ID
	uint16_t getID(){ return ObjID; }
	//Set the line state back to false for the next scan This should only be called by the rung manager (which applies the logic after processing)
	virtual void updateObject(){ bLineState = false; } 
	//Returns an object's bit (Ladder_VAR pointer) based on an inputted bit ID string
	virtual shared_ptr<Ladder_VAR> getObjectVAR( const String & );
	virtual shared_ptr<Ladder_VAR> addObjectVAR( const String & );
	
private:
	//vector<Ladder_OBJ *>nextObj;
	multimap<uint16_t, shared_ptr<Ladder_OBJ_Wrapper>> nextObj;
	typedef multimap<uint16_t, shared_ptr<Ladder_OBJ_Wrapper>> :: iterator itr;
	uint8_t iType; //Identifies the type of this object. 0 = input, 1 = Physical output, 2 = Virtual Output, 3 = timer, etc.	
	uint8_t objState; //Enabled or disabled? needed?
	uint8_t objLogic;
	uint16_t ObjID; //The unique ID for this object (globally)
	bool bLineState; //This is the logic state at this point in the line for this pass. If it is set to true at any point, it remains true till the end of the scan.
	//Bit shifting to save memory? Look into later
};

//This object serves as a means of storing logic script specific flags that pertain to a single ladder object. This allows us to perform multiple varying logic operations without the need to create multiple copies of the same object.
struct Ladder_OBJ_Wrapper 
{
	//Bits 8 = EN, 7 = TT, 6 = DN, 5 = ACC, 4 = ?, 3 = ?, 2 = ?, 1 = ?
	Ladder_OBJ_Wrapper(shared_ptr<Ladder_OBJ> obj, bool not_flag = false)
	{
		bNot = not_flag; //Exclusively for NOT logic
		ladderOBJ = obj; 
	}
	~Ladder_OBJ_Wrapper(){ }

	//Adds the inputted object to the current object's list.
	bool addNextObject( uint16_t rung, shared_ptr<Ladder_OBJ_Wrapper> pObj )
	{
		return getObject()->addNextObject(rung, pObj);
	}
	
	//Returns the pointer to the ladder object stored by this object.
	shared_ptr<Ladder_OBJ> getObject(){ return ladderOBJ; }
	//Tells us if the object is being interpreted using NOT logic
	bool getNot(){ return bNot; }
		
	private:
	bool bNot; //if the object is using not logic (per instance in rungs)
	shared_ptr<Ladder_OBJ> ladderOBJ; 
};
//Ladder_VARs can serve as both local variables to specific ladder objects (such as timers,counters,etc.), as well as independent values stored in memory, to be shared by multiple objects.
class Ladder_VAR : public Ladder_OBJ
{
	public:
	//These constructors are for pointers to existing variables
	Ladder_VAR( int_fast32_t *value, uint16_t id = 0 ) : Ladder_OBJ( id, TYPE_VAR_INT ){ values.i.val_ptr = value; b_usesPtr = true; }
	Ladder_VAR( uint_fast32_t *value, uint16_t id = 0 ) : Ladder_OBJ( id, TYPE_VAR_UINT ){ values.ui.val_ptr = value; b_usesPtr = true; }
	Ladder_VAR( bool *value, uint16_t id = 0 ) : Ladder_OBJ( id, TYPE_VAR_BOOL ){ values.b.val_ptr = value; b_usesPtr = true; }
	Ladder_VAR( float *value, uint16_t id = 0 ) : Ladder_OBJ( id, TYPE_VAR_FLOAT ){ values.f.val_ptr = value; b_usesPtr = true; }
	Ladder_VAR( uint64_t *value, uint16_t id = 0 ) : Ladder_OBJ( id, TYPE_VAR_ULONG ){ values.ul.val_ptr = value; b_usesPtr = true; }
	Ladder_VAR( int64_t *value, uint16_t id = 0 ) : Ladder_OBJ( id, TYPE_VAR_LONG ){ values.l.val_ptr = value; b_usesPtr = true; }
	//
	//These constructors are for locally stored values
	Ladder_VAR( int_fast32_t value, uint16_t id = 0 ) : Ladder_OBJ( id, TYPE_VAR_INT ){ values.i.val = value; b_usesPtr = false; }
	Ladder_VAR( uint_fast32_t value, uint16_t id = 0 ) : Ladder_OBJ( id, TYPE_VAR_UINT ){ values.ui.val = value; b_usesPtr = false; }
	Ladder_VAR( bool value, uint16_t id = 0 ) : Ladder_OBJ( id, TYPE_VAR_BOOL ){ values.b.val = value; b_usesPtr = false; }
	Ladder_VAR( float value, uint16_t id = 0 ) : Ladder_OBJ( id, TYPE_VAR_FLOAT ){ values.f.val = value; b_usesPtr = false; }
	Ladder_VAR( uint64_t value, uint16_t id = 0 ) : Ladder_OBJ( id, TYPE_VAR_ULONG ){ values.ul.val = value; b_usesPtr = false; }
	Ladder_VAR( int64_t value, uint16_t id = 0 ) : Ladder_OBJ( id, TYPE_VAR_LONG ){ values.l.val = value; b_usesPtr = false; }
	//
	virtual void updateObject()
	{ 
		Ladder_OBJ::updateObject(); 
	}
	bool getBoolValue(){ if (b_usesPtr && values.b.val_ptr) return *values.b.val_ptr; else return 0; }
	int64_t getLongValue(){ if (b_usesPtr && values.l.val_ptr) return *values.l.val_ptr; else return 0; }
	uint64_t getULongValue(){ if (b_usesPtr && values.ul.val_ptr) return *values.ul.val_ptr; else return 0; }
	float getFloatValue(){ if (b_usesPtr && values.f.val_ptr) return *values.f.val_ptr; else return 0; }
	int_fast32_t getIntValue(){ if (b_usesPtr && values.i.val_ptr) return *values.i.val_ptr; else return 0; }
	uint_fast32_t getUIntValue(){ if (b_usesPtr && values.ui.val_ptr) return *values.ui.val_ptr; else return 0; }

	template <class T>
	T getValue()
	{
		switch(getType())
		{
			case OBJ_TYPE::TYPE_VAR_BOOL:
			{
				if ( b_usesPtr )
					return static_cast<T>(*values.b.val_ptr);
			}
			break;
			case OBJ_TYPE::TYPE_VAR_FLOAT:
			{
				if ( b_usesPtr )
					return static_cast<T>(*values.f.val_ptr);
			}
			break;
			case OBJ_TYPE::TYPE_VAR_INT:
			{
				if ( b_usesPtr )
					return static_cast<T>(*values.i.val_ptr);
			}
			break;
			case OBJ_TYPE::TYPE_VAR_LONG:
			{
				if ( b_usesPtr )
					return static_cast<T>(*values.l.val_ptr);
			}
			break;
			case OBJ_TYPE::TYPE_VAR_ULONG:
			{
				if ( b_usesPtr )
					return static_cast<T>(*values.ul.val_ptr);
			}
			break;
			case OBJ_TYPE::TYPE_VAR_UINT: 
			{
				if ( b_usesPtr )
					return static_cast<T>(*values.ui.val_ptr);
			}
			break;
		}
		return static_cast<T>(0);
	}

	template <typename T>
	void setValue( const T val )
	{
		switch(getType())
		{
			case OBJ_TYPE::TYPE_VAR_BOOL:
			{
				if ( b_usesPtr )
					*values.b.val_ptr = static_cast<bool>(val);
			}
			break;
			case OBJ_TYPE::TYPE_VAR_FLOAT:
			{
				if ( b_usesPtr )
					*values.f.val_ptr = static_cast<float>(val);
			}
			break;
			case OBJ_TYPE::TYPE_VAR_INT:
			{
				if ( b_usesPtr )
					*values.i.val_ptr = static_cast<int_fast32_t>(val);
			}
			break;
			case OBJ_TYPE::TYPE_VAR_LONG:
			{
				if ( b_usesPtr )
					*values.l.val_ptr = static_cast<int64_t>(val);
			}
			break;
			case OBJ_TYPE::TYPE_VAR_ULONG:
			{
				if ( b_usesPtr )
					*values.ul.val_ptr = static_cast<uint64_t>(val);
			}
			break;
			case OBJ_TYPE::TYPE_VAR_UINT:
			{
				if ( b_usesPtr )
					*values.ui.val_ptr = static_cast<uint_fast32_t>(val);
			}
			break;
		}
	}

	virtual void setLineState(uint16_t rung, bool state)
	{ 
		if ( state ) //active up till this point
		{
			switch(getType())
			{
				case OBJ_TYPE::TYPE_VAR_BOOL:
					state = getBoolValue();
					break;
				case OBJ_TYPE::TYPE_VAR_FLOAT:
					state = getFloatValue() > 1 ? true : false;
					break;
				case OBJ_TYPE::TYPE_VAR_INT:
					state = getIntValue() > 1 ? true : false;
					break;
				case OBJ_TYPE::TYPE_VAR_LONG:
					state = getLongValue() > 1 ? true : false;
					break;
				case OBJ_TYPE::TYPE_VAR_ULONG:
					state = getULongValue() > 1 ? true : false;
					break;
				case OBJ_TYPE::TYPE_VAR_UINT: 
					state = getUIntValue() > 1 ? true : false;
					break;
				default:
					state = false;
					break;
			}
		}
		Ladder_OBJ::setLineState(rung, state); 
	}
	private:

	template <typename T>
	union group
	{
		T *val_ptr;
		T val;
	};
	union
	{
		group<int64_t> l;
		group<uint64_t> ul;
		group<int_fast32_t> i;
		group<uint_fast32_t> ui;
		group<float> f;
		group<bool> b;
	} values;

	bool b_usesPtr; //tells us if we're using a pointer to an object of the same type, or if we're using a locally stored value.
};

//The Remote_Ladder_OBJ represents an object that is initialized on an external "ESPLC" device.
class Remote_Ladder_OBJ : public Ladder_OBJ
{
	public:
	Remote_Ladder_OBJ( uint16_t id, uint8_t type, uint16_t remoteID ) : Ladder_OBJ( id, TYPE_REMOTE )
	{
		iRemoteType = type;
		iRemoteID = remoteID;
	}
	
	virtual void setLineState(uint16_t rung, bool state)
	{ 
		Ladder_OBJ::setLineState(rung, state); 
	}
	virtual void updateObject()
	{
		//Execute output operations, etc. post scan -- based on remote type.
		Ladder_OBJ::updateObject(); //parent class - must be called last
	}

	uint16_t getRemoteID(){ return iRemoteID; }
	uint8_t getremoteType() { return iRemoteType; }

	private:
	uint8_t iRemoteType; //This is the object type as it pertains to the remote controller. 
	uint16_t iRemoteID; //This is the object ID as it pertains to the remote controller
};

//An output object typically represents a physical pin or boolean, and represents the final logic state on a rung after all other logic operations have been performed. 
class OutputOBJ: public Ladder_OBJ
{
	public:
	OutputOBJ( uint16_t id, uint8_t pin, uint8_t logic = LOGIC_NO ) : Ladder_OBJ(id, TYPE_OUTPUT){ iPin = pin; pinMode(pin, OUTPUT); setLogic(logic); }
	~OutputOBJ()
	{
		 #ifdef DEBUG 
		 Serial.println(PSTR("Output Destructor")); 
		 #endif 
		 digitalWrite(iPin, LOW);
	}
	virtual void updateObject();
	uint8_t getOutputPin(){ return iPin; }
	virtual void setLineState(uint16_t rung, bool state){ Ladder_OBJ::setLineState(rung, state); }
	
	private:
	uint8_t iPin;
};

//Inputs objects check the state of a physical pin and perform logic opertions based on the state of that pin. This may entail setting the rung state to high or low depending on the logic script.
class InputOBJ : public Ladder_OBJ
{
	public:
	InputOBJ( uint16_t id, uint8_t pin, uint8_t type = TYPE_INPUT, uint8_t logic = LOGIC_NO ) : Ladder_OBJ(id, type)
	{ 
		iPin = pin; 

		inputValue =  make_shared<Ladder_VAR>(&iValue);

		uint64_t gpioBitMask = 1ULL<<pin;
		gpio_mode_t gpioMode = GPIO_MODE_INPUT;
		gpio_config_t io_conf;
		io_conf.intr_type = GPIO_INTR_DISABLE; //disable interrupts
		io_conf.mode = gpioMode;
		io_conf.pin_bit_mask = gpioBitMask;
		io_conf.pull_down_en = GPIO_PULLDOWN_ENABLE; //always pull low
		io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
		gpio_config(&io_conf);

		setLogic(logic); 
	}
	virtual ~InputOBJ()
	{ 
		#ifdef DEBUG
		Serial.println(PSTR("Input Destructor")); 
		#endif
	}
	uint_fast32_t getInput()
	{ 
		if ( getType() == TYPE_INPUT_ANALOG )
			return analogRead(iPin);
		
		return digitalRead(iPin);
	} //Return the value of the input from the assigned pin.
	uint8_t getInputPin(){ return iPin; }
	virtual void updateObject();
	virtual void setLineState(uint16_t, bool);
	virtual shared_ptr<Ladder_VAR> getObjectVAR( const String &id )
	{
		return inputValue; //There's only one Ladder_VAR for this type of object.
	}
	
	private:
	uint8_t iPin;
	uint_fast32_t iValue; 
	shared_ptr<Ladder_VAR> inputValue;
};

//Timer objects use the system clock to perform a logic operation based on a given action delay.
//Bits that are accessible from a timer: TT (Timer Timing), EN (Enabled), DN (Done), ACC (Accumulator), PRE (Preset)
class TimerOBJ : public Ladder_OBJ
{
	public:
	TimerOBJ(uint16_t id, uint_fast32_t delay, uint_fast32_t accum = 0, uint8_t type = TYPE_TON) : Ladder_OBJ(id, type)
	{ 
		//Defaults
		ttBit = false;
		enableBit = false;
		doneBit = false;
		lDelay = delay;  
		lAccum = accum; 
	}
	~TimerOBJ()
	{ 
		#ifdef DEBUG
		Serial.println(PSTR("Timer Destructor")); 
		#endif
	}
	virtual void updateObject();
	virtual void setLineState(uint16_t rung, bool state){ Ladder_OBJ::setLineState(rung, state); }
	virtual shared_ptr<Ladder_VAR> addObjectVAR( const String &id )
	{ 
		if ( !getObjectVAR(id) ) //proceed if it doesn't already exist
		{
			shared_ptr<Ladder_VAR> var = 0;
			if ( id == bitTagEN )
				var = make_shared<Ladder_VAR>(&enableBit);
			else if ( id == bitTagDN )
				var = make_shared<Ladder_VAR>(&doneBit);
			else if ( id == bitTagPRE )
				var = make_shared<Ladder_VAR>(&lDelay);
			else if ( id == bitTagTT )
				var = make_shared<Ladder_VAR>(&ttBit);
			else if ( id == bitTagACC)
				var = make_shared<Ladder_VAR>(&lAccum);

			if ( var )
			{
				bitMap.emplace(id,var); // store away for later
				#ifdef DEBUG 
		 		Serial.println(PSTR("Created new Timer Object Tag: ") + id ); 
				#endif
				return var;
			}
		}
		#ifdef DEBUG 
		Serial.println(PSTR("Failed: Object Tag: ") + id ); 
		#endif
		return 0; //failed to add
	}
	virtual shared_ptr<Ladder_VAR> getObjectVAR( const String &id )
	{
		std::map<const String, shared_ptr<Ladder_VAR>>::iterator bititr = bitMap.find(id);
		if (bititr != bitMap.end())
    	{
			return bititr->second; //return the shared pointer to the var
    	}
		else
		{
			return Ladder_OBJ::getObjectVAR(id); //default case.
		}
	}
	bool getTTBitVal(){ return ttBit; }
	bool getENBitVal(){ return enableBit; }
	bool getDNbitVal(){ return doneBit; }
	uint_fast32_t getAccumVal(){ return lAccum; }
	
	private:
	bool doneBit,
		enableBit, 
		ttBit;	
	uint_fast32_t timeStart, timeEnd, //Local variables only (used for calculations)
				  lDelay, lAccum;
	std::map<const String, shared_ptr<Ladder_VAR>> bitMap; //used for DN, EN, TT, ACC, PRE, if they are accessed by the parser.
};

//Counter objects serve as a means of incrementing or decrementing from a given value and performing an operation once the target value has been reached.
class CounterOBJ : public Ladder_OBJ
{
	public:
	CounterOBJ(uint16_t id, uint_fast32_t count = 0, uint_fast32_t accum = 0, uint8_t type = TYPE_CTU) : Ladder_OBJ(id, type)
	{ 
		iCount = count;
		iAccum = accum;
		doneBit = false;
		enableBit = false;
	}
	~CounterOBJ()
	{ 
		#ifdef DEBUG
		Serial.println(PSTR("Counter Destructor"));
		#endif
	}
	virtual void setLineState(uint16_t rung, bool state){ Ladder_OBJ::setLineState(rung, state); }
	virtual void updateObject();
	//returns the current value of the counter's enable bit 
	void setENBitVal(bool val){ enableBit = val; }
	//returns the current value of the counter's done bit
	void setDNBitVal(bool val){ doneBit = val; }
	//returns the current value of the counter's accumulator value
	void setAccumVal(uint_fast32_t val){ iAccum = val; }
	//returns the current value of the counter's "count-to" value
	void setCountVal(uint_fast32_t val) {iCount = val; }
	virtual shared_ptr<Ladder_VAR> addObjectVAR( const String &id )
	{ 
		if ( !getObjectVAR(id) ) //already exists?
		{
			shared_ptr<Ladder_VAR> var = 0;
			if ( id == bitTagEN )
				var = make_shared<Ladder_VAR>(&enableBit);
			else if ( id == bitTagDN )
				var = make_shared<Ladder_VAR>(&doneBit);
			else if ( id == bitTagPRE )
				var = make_shared<Ladder_VAR>(&iCount);
			else if ( id == bitTagACC)
				var = make_shared<Ladder_VAR>(&iAccum);

			if ( var )
			{
				#ifdef DEBUG 
		 		Serial.println(PSTR("Created new Counter Object Tag: ") + id ); 
				#endif
				bitMap.emplace(id,var);
				return var;
			}
		}
		#ifdef DEBUG 
		Serial.println(PSTR("Failed: Object Tag: ") + id ); 
		#endif
		return 0;
	}
	virtual shared_ptr<Ladder_VAR> getObjectVAR( const String &id )
	{
		std::map<const String, shared_ptr<Ladder_VAR>>::iterator bititr = bitMap.find(id);
		if (bititr != bitMap.end())
    	{
			return bititr->second; //return the shared pointer to the var
    	}
		else
		{
			return Ladder_OBJ::getObjectVAR(id); //default case. -- probably an error
		}
	}

	bool getENBitVal(){ return enableBit; }
	bool getDNbitVal(){ return doneBit; }
	uint_fast32_t getAccumVal(){ return iAccum; }
	uint_fast32_t getCountVal(){ return iCount; }
	void reset(){iCount = 0; iAccum = 0;}
		
	private:
	uint_fast32_t iCount, iAccum;
	bool doneBit, enableBit;
	std::map<const String, shared_ptr<Ladder_VAR>> bitMap; //used for DN, EN, ACC, PRE, if they are accessed by the parser.
};

//The clock object is a more advanced object that allows operations to be performed at a specific time.
class ClockOBJ : public Ladder_OBJ
{
	public:
	ClockOBJ(uint16_t id, shared_ptr<Time> sys, uint8_t yr, uint8_t mo, uint8_t da, uint8_t hr, uint8_t min, uint8_t sec, uint8_t type = TYPE_CLOCK) : Ladder_OBJ(id, type)
	{
		pSysTime = sys;
		doneBit = false;
		enableBit = false;
		pPresetTime = make_shared<Time>(yr, mo, da, hr, min, sec); //should remain static (not updated unless explicitly told to do so)
	}
	~ClockOBJ(){  }
	virtual void setLineState(uint16_t rung, bool state){ Ladder_OBJ::setLineState(rung, state); }
	virtual void updateObject(); //this handles the updating of the clock
	
	bool getENBit(){ return enableBit; }
	bool getDNbit(){ return doneBit; }
		
	private:
	bool doneBit, enableBit;
	shared_ptr<Time> pPresetTime;
	shared_ptr<Time> pSysTime;
};

#endif /* PLC_IO_H_ */