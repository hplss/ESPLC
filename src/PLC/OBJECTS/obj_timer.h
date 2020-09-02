#ifndef PLC_IO_OBJ_TIMER
#define PLC_IO_OBJ_TIMER

#include "../PLC_IO.h"
#include "obj_var.h"

//Timer objects use the system clock to perform a logic operation based on a given action delay.
//Bits that are accessible from a timer: TT (Timer Timing), EN (Enabled), DN (Done), ACC (Accumulator), PRE (Preset)
class TimerOBJ : public Ladder_OBJ
{
	public:
	TimerOBJ(const String &id, uint_fast32_t delay, uint_fast32_t accum = 0, uint8_t type = TYPE_TON) : Ladder_OBJ(id, type)
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
	virtual void setLineState(bool &state){ Ladder_OBJ::setLineState(state); }
	virtual shared_ptr<Ladder_VAR> addObjectVAR( const String &id );
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

#endif