#include "obj_output_basic.h"


//////////////////////////////////////////////////////////////////////////
// OUTPUT OBJECT BEGIN
//////////////////////////////////////////////////////////////////////////
void OutputOBJ::updateObject() //Logic used to update the coil
{
	bool lineState = (getLineState()!=getLogic()) ? LOW : HIGH;
	digitalWrite(iPin, lineState);
	//Serial.print("Output State: ");
	//Serial.println(state);
	Ladder_OBJ_Logical::updateObject();
}