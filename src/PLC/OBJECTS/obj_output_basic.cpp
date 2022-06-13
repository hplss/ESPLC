#include "obj_output_basic.h"


//////////////////////////////////////////////////////////////////////////
// OUTPUT OBJECT BEGIN
//////////////////////////////////////////////////////////////////////////
void OutputOBJ::updateObject() //Logic used to update the coil
{
	bool lineState = (getLineState() != iLogic) ? LOW : HIGH;

	if ( iType == OBJ_TYPE::TYPE_OUTPUT )
	{
		if ( iOutputValue != lineState )
			iOutputValue = lineState; //only update if changed.

		digitalWrite(iPin, lineState);
	}
	else if ( iType == OBJ_TYPE::TYPE_OUTPUT_PWM )
	{
		if ( !lineState )
			iOutputValue = lineState;
		else
			iOutputValue = iDutyCycle; //set as the stored duty cycle

		ledcWrite(iPWMChannel, iOutputValue);
	}
	Ladder_OBJ_Logical::updateObject();
}