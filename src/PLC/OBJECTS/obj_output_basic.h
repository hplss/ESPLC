#ifndef PLC_IO_OBJ_OUTPUT_BASIC
#define PLC_IO_OBJ_OUTPUT_BASIC

#include "../PLC_IO.h"
#include "obj_var.h"

//An output object typically represents a physical pin or boolean, and represents the final logic state on a rung after all other logic operations have been performed. 
class OutputOBJ: public Ladder_OBJ_Logical
{
	public:
	//PWM resolution formula 8*10^6/(2^resolution) = max frequency 
	OutputOBJ( const String &id, uint8_t pin, OBJ_TYPE type = OBJ_TYPE::TYPE_OUTPUT, uint8_t logic = LOGIC_NO, uint8_t pwm_channel = 0, uint16_t duty_cycle = 0, double pwm_frequency = 78000.00, uint8_t pwm_resolution = 10 ) : Ladder_OBJ_Logical(id, type)
	{ 
		iPin = pin; 
		iPWMChannel = pwm_channel;
		iDutyCycle = duty_cycle;
		iOutputValue = 0; //by default

		if ( type == OBJ_TYPE::TYPE_OUTPUT )
			pinMode(pin, OUTPUT); 
		else if ( type == OBJ_TYPE::TYPE_OUTPUT_PWM )
		{
			ledcSetup(pwm_channel, pwm_frequency, pwm_resolution); //configure the PWM parameters
			ledcAttachPin(pin, pwm_channel); //set the IO pin as a PWM output
		}

		getObjectVARs().emplace_back(make_shared<Ladder_VAR>(&iOutputValue, bitTagVAL)); //VAL variable - corresponds to the duty cycle of a PWM output, or a HIGH/LOW signal

		setLogic(logic); 
	}

	~OutputOBJ()
	{
		 #ifdef DEBUG 
		 Serial.println(PSTR("Output Destructor")); 
		 #endif 
		 digitalWrite(iPin, LOW);

		 if (getType() == OBJ_TYPE::TYPE_OUTPUT_PWM)
			 ledcDetachPin(iPin); //detatch from the PWM generator
	}

	virtual void updateObject();
	uint8_t getOutputPin(){ return iPin; }
	virtual void setLineState(bool &state, bool bNot){ Ladder_OBJ_Logical::setLineState(state, bNot); }
	virtual shared_ptr<Ladder_VAR> getObjectVAR( const String &id )
	{
		for ( uint8_t x = 0; x < getObjectVARs().size(); x++ )
		{
			if ( getObjectVARs()[x]->getID() == id )
				return getObjectVARs()[x];
		}

		return Ladder_OBJ_Logical::getObjectVAR(id);
	}
	
	private:
	uint8_t iPin,
			iPWMChannel;

	uint16_t iOutputValue, //used for both analog and digital outputs.	
			 iDutyCycle;
};

#endif