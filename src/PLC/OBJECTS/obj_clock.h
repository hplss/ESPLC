#ifndef PLC_IO_OBJ_CLOCK
#define PLC_IO_OBJ_CLOCK

#include "../PLC_IO.h"
#include "obj_var.h"

//The clock object is a more advanced object that allows operations to be performed at a specific time.
class ClockOBJ : public Ladder_OBJ_Logical
{
	public:
	ClockOBJ(const String &id, shared_ptr<Time> sys, VAR_PTR yr, VAR_PTR mo, VAR_PTR da, VAR_PTR hr, VAR_PTR min, VAR_PTR sec, OBJ_TYPE type = OBJ_TYPE::TYPE_CLOCK) : Ladder_OBJ_Logical(id, type)
	{
		pSysTime = sys;
		doneBit = false;
		enableBit = false;
		pPresetTime = make_shared<Time>(yr->getValue<uint8_t>(), mo->getValue<uint8_t>(), da->getValue<uint8_t>(), hr->getValue<uint8_t>(), min->getValue<uint8_t>(), sec->getValue<uint8_t>()); //should remain static (not updated unless explicitly told to do so)
	}
	~ClockOBJ(){ }

	virtual void setLineState(bool &state, bool bNot){ Ladder_OBJ_Logical::setLineState(state, bNot); }
	virtual void updateObject(); //this handles the updating of the clock
	virtual VAR_PTR getObjectVAR( const String & );
		
	private:
	bool doneBit, enableBit;
	shared_ptr<Time> pPresetTime;
	shared_ptr<Time> pSysTime;
	VAR_PTR ptr_year, ptr_month, ptr_day, ptr_hour, ptr_min, ptr_sec;
};

#endif