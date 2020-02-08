/*
 * PLC_IO.h
 *
 * Created: 9/22/2019 4:26:05 PM
 *  Author: Andrew
 */ 


#ifndef PLC_IO_H_
#define PLC_IO_H_

#include <vector>
#include <esp32-hal-gpio.h>
#include <HardwareSerial.h>
#include "PLC_VAR.h"
#include "../CORE/GlobalDefs.h"
#include "../CORE/Time.h"
#include <map>
#include <memory>

using namespace std;

class Ladder_OBJ
{
public:
	Ladder_OBJ( uint16_t &id, uint8_t type ){ ObjID = id; iType = type; bLineState = false; }//start off
	virtual ~Ladder_OBJ(){ nextObj.clear(); }
	void setObjID(uint16_t id) { ObjID = id; }
	bool setState(uint8_t state) { objState = state; return true; }
	void setType(uint8_t type) { iType = type; }
	virtual void setLineState(uint16_t rung, bool state){ if(state) bLineState = state; getNextObj( rung ); } //latch state high if applied, then find the next connected object in the rung.
	void setLogic(uint8_t logic) { objLogic = logic; }
	bool addNextObject( uint16_t, shared_ptr<Ladder_OBJ> ); //Add the potential objects that are next in the chain, based on Rung number
	bool addNextObject( uint16_t, vector<shared_ptr<Ladder_OBJ>> &);
	
	void getNextObj( uint16_t ); //passes relevant data on to the next object in the sequence
	uint8_t getState(){ return objState; } //Returns enabled/disabled
	uint8_t getLogic() { return objLogic; }
	uint8_t getType(){ return iType; }
	bool getLineState(){ return bLineState; }
	uint16_t getID(){ return ObjID; }
	virtual void updateObject(){ bLineState = false; } //set the line state back to false for the next scan This should only be called by the rung manager (which applies the logic after processing)
	
private:
	//vector<Ladder_OBJ *>nextObj;
	multimap<uint16_t, shared_ptr<Ladder_OBJ>> nextObj;
	typedef multimap<uint16_t, shared_ptr<Ladder_OBJ>> :: iterator itr;
	uint8_t iType : 5; //Identifies the type of this object. 0 = input, 1 = Physical output, 2 = Virtual Output, 3 = timer, etc.	
	uint8_t objState : 3;
	uint8_t objLogic : 2;
	uint16_t ObjID;
	bool bLineState : 1; //This is the logic state at this point in the line for this pass. If it is set to true at any point, it remains true till the end of the scan.
	//Bit shifting to save memory? Look into later
};

struct Ladder_OJB_Wrapper
{
	//Bits 8 = EN, 7 = TT, 6 = DN, 5 = ACC, 4 = ?, 3 = ?, 2 = ?, 1 = ?
	Ladder_OJB_Wrapper(shared_ptr<Ladder_OBJ> obj, bool not_flag, uint8_t bitflag )
	{
		bNot = not_flag;
		ladderOBJ = obj;
		iBitFlag = bitflag;
	}
	~Ladder_OJB_Wrapper(){ }
		
	Ladder_OBJ &getObj(){ return *ladderOBJ.get(); }
	bool getNot(){ return bNot; }
	uint8_t &getBits(){ return iBitFlag; }
		
	private:
	bool bNot; //if the object is using not logic
	shared_ptr<Ladder_OBJ> ladderOBJ;
	uint8_t iBitFlag; //
};

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
	//void setOutput(uint16_t); //Set a value to the assigned output pin
	virtual void updateObject();
	uint8_t getOutputPin(){ return iPin; }
	virtual void setLineState(uint16_t rung, bool state){ Ladder_OBJ::setLineState(rung, state); }
	
	private:
	uint8_t iPin : 6;
};

//Bit objects serve as a way to check against the state of another object
class BitOBJ: public Ladder_OBJ
{
	public:
	BitOBJ( uint16_t id, uint8_t logic = LOGIC_NO ) : Ladder_OBJ(id, TYPE_BIT){ setLogic(logic); }
	~BitOBJ()
	{ 
		#ifdef DEBUG
		Serial.println(PSTR("Bit Destructor")); 
		#endif 
	}
	//void setOutput(uint16_t); //Set a value to the assigned output pin
	virtual void updateObject();
	virtual void setLineState(uint16_t, bool);
	
	private:
	uint8_t iPin : 5;
};

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

class VirtualOBJ : public Ladder_OBJ //This object can operate as an input or an output (it's basically acting like a boolean in memory)
{
	public:
	VirtualOBJ( uint16_t id, uint8_t logic = LOGIC_NO ) : Ladder_OBJ(id, TYPE_VIRTUAL){ setLogic(logic); }
	~VirtualOBJ(){}
	virtual void updateObject(); //update logic
	virtual void setLineState(uint16_t rung, bool state){ Ladder_OBJ::setLineState(rung, state); } //this will likely be different from the rest. Work on later
	
	private:	
};

class TimerOBJ : public Ladder_OBJ
{
	public:
	TimerOBJ(uint16_t id, uint_fast32_t delay, uint_fast32_t accum = 0, uint8_t type = TYPE_TON) : Ladder_OBJ(id, type){ enableBit = false; doneBit = false; ttBit = false; lDelay = delay;  lAccum = accum; }
	~TimerOBJ()
	{ 
		#ifdef DEBUG
		Serial.println(PSTR("Timer Destructor")); 
		#endif
	}
	virtual void updateObject();
	virtual void setLineState(uint16_t rung, bool state){ Ladder_OBJ::setLineState(rung, state); }
	bool getTTBit(){ return ttBit; }
	bool getENBit(){ return enableBit; }
	bool getDNbit(){ return doneBit; }
	uint_fast32_t getAccum(){ return lAccum; }
	
	private:
	bool doneBit : 1; //8-bit value 0 0 0 1
	bool enableBit : 1; //			0 0 1 0
	bool ttBit : 1;	//				0 1 0 0 
	uint_fast32_t timeStart, timeEnd;
	uint_fast32_t lDelay, lAccum;
};

class CounterOBJ : public Ladder_OBJ
{
	public:
	CounterOBJ(uint16_t id, uint16_t count = 0, uint16_t accum = 0, uint8_t type = TYPE_CTU) : Ladder_OBJ(id, type){ iCount = count; iAccum = accum; }
	~CounterOBJ()
	{ 
		#ifdef DEBUG
		Serial.println(PSTR("Counter Destructor"));
		#endif
	}
	virtual void setLineState(uint16_t rung, bool state){ Ladder_OBJ::setLineState(rung, state); }
	virtual void updateObject();
	bool getENBit(){ return enableBit; }
	bool getDNbit(){ return doneBit; }
	uint16_t getAccum(){ return iAccum; }
	void reset(){iCount = 0; iAccum = 0;}
		
	private:
	bool doneBit : 1; 
	bool enableBit : 1;
	uint16_t iCount,
			 iAccum;
};

class ComparisonOBJ :public Ladder_OBJ //uses a math statement to generate logic
{
	public:
	ComparisonOBJ(uint16_t id, uint8_t type) : Ladder_OBJ(id, type){}
	~ComparisonOBJ(){ Serial.println(PSTR("Comparison Destructor")); }
		
	private:
};

class ClockOBJ : public Ladder_OBJ
{
	public:
	ClockOBJ(uint16_t id, shared_ptr<Time> sys, uint8_t yr, uint8_t mo, uint8_t da, uint8_t hr, uint8_t min, uint8_t sec, uint8_t type = TYPE_CON) : Ladder_OBJ(id, type)
	{
		pSysTime = sys;
		pPresetTime = make_shared<Time>(yr, mo, da, hr, min, sec); //should remain static (not updated unless explicitly told to do so)
	}
	~ClockOBJ(){  }
	virtual void setLineState(uint16_t rung, bool state){ Ladder_OBJ::setLineState(rung, state); }
	virtual void updateObject(); //this handles the updating of the clock
	
	bool getENBit(){ return enableBit; }
	bool getDNbit(){ return doneBit; }
		
	private:
	bool doneBit : 1; 
	bool enableBit : 1; 
	shared_ptr<Time> pPresetTime;
	shared_ptr<Time> pSysTime;
};
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