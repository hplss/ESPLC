#ifndef PLC_IO_OBJ_OUTPUT_BASIC
#define PLC_IO_OBJ_OUTPUT_BASIC

#include "../PLC_IO.h"

//An output object typically represents a physical pin or boolean, and represents the final logic state on a rung after all other logic operations have been performed. 
class OutputOBJ: public Ladder_OBJ
{
	public:
	OutputOBJ( const String &id, uint8_t pin, uint8_t logic = LOGIC_NO ) : Ladder_OBJ(id, TYPE_OUTPUT){ iPin = pin; pinMode(pin, OUTPUT); setLogic(logic); }
	~OutputOBJ()
	{
		 #ifdef DEBUG 
		 Serial.println(PSTR("Output Destructor")); 
		 #endif 
		 digitalWrite(iPin, LOW);
	}
	virtual void updateObject();
	uint8_t getOutputPin(){ return iPin; }
	virtual void setLineState(bool &state, bool bNot){ Ladder_OBJ::setLineState(state, bNot); }
	
	private:
	uint8_t iPin;
};

#endif