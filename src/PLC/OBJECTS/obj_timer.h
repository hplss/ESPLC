#ifndef PLC_IO_OBJ_TIMER
#define PLC_IO_OBJ_TIMER

#include "../PLC_IO.h"
#include "obj_var.h"

//Timer objects use the system clock to perform a logic operation based on a given action delay.
//Bits that are accessible from a timer: TT (Timer Timing), EN (Enabled), DN (Done), ACC (Accumulator), PRE (Preset)
class TimerOBJ : public Ladder_OBJ_Logical
{
	public:
	TimerOBJ(const String &id, VAR_PTR delay, VAR_PTR accum, OBJ_TYPE type = OBJ_TYPE::TYPE_TIMER_ON ) : Ladder_OBJ_Logical(id, type)
	{ 
		//Defaults
		ttBit = false;
		enableBit = false;
		doneBit = false;

		if ( !delay )
			delay = make_shared<Ladder_VAR>( (uint32_t)0, bitTagPRE );

		if ( !accum )
			accum = make_shared<Ladder_VAR>( (uint32_t)0, bitTagACC );

		//Store local copies
		ptr_delay = delay;  
		ptr_accum = accum; 

		//Pust to global list of object variables
		getObjectVARs().push_back(ptr_delay);
		getObjectVARs().push_back(ptr_accum);
	}
	~TimerOBJ()
	{ 
		#ifdef DEBUG
		Serial.println(PSTR("Timer Destructor")); 
		#endif
	}
	virtual void updateObject();
	virtual void setLineState(bool &state, bool bNot){ Ladder_OBJ_Logical::setLineState(state, bNot); }
	void reset();

	virtual VAR_PTR getObjectVAR( const String &id );
	
	private:
	bool doneBit,
		enableBit, 
		ttBit;	
		
	uint32_t timeStart, timeEnd; //Local variables only (used for calculations)

	VAR_PTR ptr_delay, ptr_accum;
};

#endif