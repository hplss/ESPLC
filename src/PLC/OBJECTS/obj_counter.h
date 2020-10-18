#ifndef PLC_IO_OBJ_COUNTER
#define PLC_IO_OBJ_COUNTER

#include "../PLC_IO.h"
#include "obj_var.h"

//Counter objects serve as a means of incrementing or decrementing from a given value and performing an operation once the target value has been reached.
class CounterOBJ : public Ladder_OBJ_Logical
{
	public:
	CounterOBJ(const String &id, uint_fast32_t count = 0, uint_fast32_t accum = 0, uint8_t type = TYPE_CTU) : Ladder_OBJ_Logical(id, type)
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
	virtual void setLineState(bool &state, bool bNot){ Ladder_OBJ_Logical::setLineState(state, bNot); }
	virtual void updateObject(bool);
	//returns the current value of the counter's enable bit 
	void setENBitVal(bool val){ enableBit = val; }
	//returns the current value of the counter's done bit
	void setDNBitVal(bool val){ doneBit = val; }
	//returns the current value of the counter's accumulator value
	void setAccumVal(uint_fast32_t val){ iAccum = val; }
	//returns the current value of the counter's "count-to" value
	void setCountVal(uint_fast32_t val) {iCount = val; }
	virtual shared_ptr<Ladder_VAR> addObjectVAR( const String & );
	virtual shared_ptr<Ladder_VAR> getObjectVAR( const String &id )
	{
		std::map<const String, shared_ptr<Ladder_VAR>>::iterator bititr = bitMap.find(id);
		if (bititr != bitMap.end())
    	{
			return bititr->second; //return the shared pointer to the var
    	}
		else
		{
			return Ladder_OBJ_Logical::getObjectVAR(id); //default case. -- probably an error
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

#endif