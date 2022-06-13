#ifndef PLC_IO_OBJ_INPUT_BASIC
#define PLC_IO_OBJ_INPUT_BASIC

#include "../PLC_IO.h"
#include "obj_var.h"
#include <driver/adc.h> //analog support

//Inputs objects check the state of a physical pin and perform logic opertions based on the state of that pin. This may entail setting the rung state to high or low depending on the logic script.
class InputOBJ : public Ladder_OBJ_Logical
{
	public:
	InputOBJ( const String &id, uint8_t pin, OBJ_TYPE type = OBJ_TYPE::TYPE_INPUT, uint8_t logic = LOGIC_NO ) : Ladder_OBJ_Logical(id, type), iPin(pin), iLogic(logic)
	{ 
		iValue = 0; //default

		getObjectVARs().emplace_back(make_shared<Ladder_VAR>(&iValue, bitTagVAL)); 

		uint64_t gpioBitMask = 1ULL<<pin;
		gpio_mode_t gpioMode = GPIO_MODE_INPUT;
		gpio_config_t io_conf;
		io_conf.intr_type = GPIO_INTR_DISABLE; //disable interrupts
		io_conf.mode = gpioMode;
		io_conf.pin_bit_mask = gpioBitMask;
		io_conf.pull_down_en = GPIO_PULLDOWN_ENABLE; //always pull low
		io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
		gpio_config(&io_conf);
	}
	virtual ~InputOBJ()
	{ 
		#ifdef DEBUG
		Serial.println(PSTR("Input Destructor")); 
		#endif
	}

	uint16_t getInput()
	{ 
		if ( iType == OBJ_TYPE::TYPE_INPUT_ANALOG )
			return analogRead(iPin);
		
		return digitalRead(iPin);
	} //Return the value of the input from the assigned pin.

	virtual void updateObject();
	virtual void setLineState(bool &, bool);

private:
	uint16_t iValue; //input value that was read and stored off

	const uint8_t iPin,
	              iLogic;
};

#endif