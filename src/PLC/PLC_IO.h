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
	uint8_t objLogic : 2;
	uint16_t ObjID; //The unique ID for this object (globally)
	bool bLineState : 1; //This is the logic state at this point in the line for this pass. If it is set to true at any point, it remains true till the end of the scan.
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
	void addNextObject( uint16_t rung, shared_ptr<Ladder_OBJ_Wrapper> pObj )
	{
		getObject()->addNextObject(rung, pObj);
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
	Ladder_VAR( int_fast32_t *value, uint16_t id = 0 ) : Ladder_OBJ( id, TYPE_VAR_INT )
	{ 
		values.iValue = value;
	}
	Ladder_VAR( uint_fast32_t *value, uint16_t id = 0 ) : Ladder_OBJ( id, TYPE_VAR_UINT )
	{ 
		values.uiValue = value;
	}
	Ladder_VAR( bool *value, uint16_t id = 0 ) : Ladder_OBJ( id, TYPE_VAR_BOOL )
	{
		values.bValue = value;
	}
	Ladder_VAR( float *value, uint16_t id = 0 ) : Ladder_OBJ( id, TYPE_VAR_FLOAT )
	{
		values.fValue = value;
	}
	Ladder_VAR( uint64_t *value, uint16_t id = 0 ) : Ladder_OBJ( id, TYPE_VAR_ULONG )
	{
		values.ulValue = value;
	}
	Ladder_VAR( int64_t *value, uint16_t id = 0 ) : Ladder_OBJ( id, TYPE_VAR_LONG )
	{
		values.lValue = value;
	}
	virtual void updateObject()
	{ 
		Ladder_OBJ::updateObject(); 
	}
	bool getBoolValue(){ if (values.bValue) return *values.bValue; else return 0; }
	int64_t getLongValue(){ if (values.lValue) return *values.lValue; else return 0; }
	uint64_t getULongValue(){ if (values.ulValue) return *values.ulValue; else return 0; }
	float getFloatValue(){ if (values.fValue) return *values.fValue; else return 0; }
	int_fast32_t getIntValue(){ if (values.iValue) return *values.iValue; else return 0; }
	uint_fast32_t getUIntValue(){ if (values.uiValue) return *values.uiValue; else return 0; }

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
	union
	{
		bool *bValue;
		float *fValue;
		int_fast32_t *iValue; //signed int
		uint_fast32_t *uiValue; //unsigned int
		int64_t *lValue;
		uint64_t *ulValue;
	} values;
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
		 enableBit = false;
		 digitalWrite(iPin, LOW);
	}
	//void setOutput(uint16_t); //Set a value to the assigned output pin
	virtual void updateObject();
	uint8_t getOutputPin(){ return iPin; }
	virtual void setLineState(uint16_t rung, bool state){ Ladder_OBJ::setLineState(rung, state); }
	
	private:
	uint8_t iPin : 6;
	bool enableBit;
};

//Inputs objects check the state of a physical pin and perform logic opertions based on the state of that pin. This may entail setting the rung state to high or low depending on the logic script.
class InputOBJ : public Ladder_OBJ
{
	public:
	InputOBJ( uint16_t id, uint8_t pin, uint8_t logic = LOGIC_NO ) : Ladder_OBJ(id, TYPE_INPUT)
	{ 
		iPin = pin; 

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
	bool getInput(){ return digitalRead(iPin); } //Return the value of the input from the assigned pin.
	uint8_t getInputPin(){ return iPin; }
	virtual void updateObject();
	virtual void setLineState(uint16_t, bool);
	
	private:
	uint8_t iPin : 6; //can be over 31
};

//Virtual objects can operate as an input or an output (it's basically acting like a variable in memory), typically they are set HIGH/LOW as outputs and used as inputs on a later rung.
class VirtualOBJ : public Ladder_OBJ 
{
	public:
	VirtualOBJ( uint16_t id, uint8_t logic = LOGIC_NO ) : Ladder_OBJ(id, TYPE_VIRTUAL)
	{ 
		setLogic(logic); 
	}
	~VirtualOBJ(){}
	virtual void updateObject(); //update logic
	virtual void setLineState(uint16_t rung, bool lineState)
	{ 
		/*bool input = bit;
		if ( lineState ) //looks like we have a high state coming in to this object from the previous on the rung, let's see if we pass the state tests for this object.
		{
			if(!input && getLogic() == LOGIC_NO) //input is low (button not pressed) and logic is normally open (default position of button is off)
				lineState = false; //input not activated
			else if (input && getLogic() == LOGIC_NC) //input is high (button is pressed), but logic is normally closed (0)
				lineState = false; //input not activated, only active if input is 0 in this case
		}*/
		Ladder_OBJ::setLineState(rung, lineState); 
	} 
	
	private:	
	shared_ptr<Ladder_VAR> var;
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

class ComparisonOBJ :public Ladder_OBJ //uses a math statement to generate logic -- WIP
{
	public:
	ComparisonOBJ(uint16_t id, uint8_t type) : Ladder_OBJ(id, type){}
	~ComparisonOBJ(){ Serial.println(PSTR("Comparison Destructor")); }
		
	private:
};

//The clock object is a more advanced object that allows operations to be performed at a specific time.
class ClockOBJ : public Ladder_OBJ
{
	public:
	ClockOBJ(uint16_t id, shared_ptr<Time> sys, uint8_t yr, uint8_t mo, uint8_t da, uint8_t hr, uint8_t min, uint8_t sec, uint8_t type = TYPE_CON) : Ladder_OBJ(id, type)
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
//Syntax: COUNTER.TAG (needs to verify that TAG exists and is valid for object type, if it is, the values needs to have all references properly established)
// This entails storing the reference into a new LADDER_VAR, and using the var to handle the mixing of different types. 
/*
class BasicMathBlockOBJ :public Ladder_OBJ //Basic math operations block (does GRE, LES, GRQ, LEQ operations)
{
	public:
	BasicMathBlockOBJ(uint16_t id, Ladder_VAR *A, Ladder_VAR *B, uint8_t type) : Ladder_OBJ(id, type){ sourceA = A; sourceB = B; }
	~BasicMathBlockOBJ(){}
	virtual void setLineState(bool state){ Ladder_OBJ:setLineState(state); }
	virtual void updateObject();
	
	private:
	Ladder_VAR* sourceA,
				sourceB;
};

class LimitOBJ :public Ladder_OBJ //Basic math comparison block (does GRE, LES, GRQ
{
	public:
	ComparisonOBJ(uint16_t id, uint_fast32_t low, uint_fast32_t high, uint8_t type) : Ladder_OBJ(id, type){ lowVal = low; highVal = high; }
	~ComparisonOBJ(){}
	virtual void setLineState(bool state){ Ladder_OBJ:setLineState(state); }
	virtual void updateObject();
	
	private:
	uint_fast32_t lowVal,
				  highVal;
};*/

#endif /* PLC_IO_H_ */