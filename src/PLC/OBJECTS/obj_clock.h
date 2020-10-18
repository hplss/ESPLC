#ifndef PLC_IO_OBJ_CLOCK
#define PLC_IO_OBJ_CLOCK

#include "../PLC_IO.h"

//The clock object is a more advanced object that allows operations to be performed at a specific time.
class ClockOBJ : public Ladder_OBJ_Logical
{
	public:
	ClockOBJ(const String &id, shared_ptr<Time> sys, uint8_t yr, uint8_t mo, uint8_t da, uint8_t hr, uint8_t min, uint8_t sec, uint8_t type = TYPE_CLOCK) : Ladder_OBJ_Logical(id, type)
	{
		pSysTime = sys;
		doneBit = false;
		enableBit = false;
		pPresetTime = make_shared<Time>(yr, mo, da, hr, min, sec); //should remain static (not updated unless explicitly told to do so)
	}
	~ClockOBJ(){  }
	virtual void setLineState(bool &state, bool bNot){ Ladder_OBJ_Logical::setLineState(state, bNot); }
	virtual void updateObject(bool); //this handles the updating of the clock
	
	bool getENBit(){ return enableBit; }
	bool getDNbit(){ return doneBit; }
		
	private:
	bool doneBit, enableBit;
	shared_ptr<Time> pPresetTime;
	shared_ptr<Time> pSysTime;
};

#endif