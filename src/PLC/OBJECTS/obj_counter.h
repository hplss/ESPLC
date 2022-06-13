#ifndef PLC_IO_OBJ_COUNTER
#define PLC_IO_OBJ_COUNTER

#include "../PLC_IO.h"
#include "obj_var.h"

//Counter objects serve as a means of incrementing or decrementing from a given value and performing an operation once the target value has been reached.
class CounterOBJ : public Ladder_OBJ_Logical
{
	public:
	CounterOBJ(const String &id, VAR_PTR count, VAR_PTR accum, OBJ_TYPE type = OBJ_TYPE::TYPE_COUNTER_UP) : Ladder_OBJ_Logical(id, type)
	{ 
		doneBit = false;
		enableBit = false;

		if ( !count )
		{
			count = make_shared<Ladder_VAR>( (uint32_t)0, bitTagPRE ); //just in case
		}

		if ( !accum )
		{
			accum = make_shared<Ladder_VAR>( (uint32_t)0, bitTagACC );
		}

		ptr_count = count;
		ptr_accum = accum;

		getObjectVARs().emplace_back(count);
		getObjectVARs().emplace_back(accum);
	}
	~CounterOBJ()
	{ 
		#ifdef DEBUG
		Serial.println(PSTR("Counter Destructor"));
		#endif
	}
	virtual void setLineState(bool &state, bool bNot){ Ladder_OBJ_Logical::setLineState(state, bNot); }
	virtual void updateObject(bool);

	virtual shared_ptr<Ladder_VAR> getObjectVAR( const String & );

	void reset()
	{
		ptr_accum->setValue(0);
		ptr_count->setValue(0); 
		doneBit = false;
		enableBit = false;
	}
		
	private:
	VAR_PTR ptr_accum, ptr_count;
	bool doneBit, enableBit;
};

#endif