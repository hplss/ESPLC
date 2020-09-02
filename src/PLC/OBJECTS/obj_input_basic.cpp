#include "obj_input_basic.h"


//////////////////////////////////////////////////////////////////////////
// INPUT OBJECT BEGIN
//////////////////////////////////////////////////////////////////////////
void InputOBJ::updateObject()
{
	Ladder_OBJ::updateObject(); //parent class - must be called last
}

void InputOBJ::setLineState( bool &state )
{
	iValue = getInput();
	if ( getType() == TYPE_INPUT ) //digital only
	{
		if(!iValue && getLogic() == LOGIC_NO) //input is low (button not pressed) and logic is normally open (default position of button is off)
			state = false; //input not activated
		else if (iValue && getLogic() == LOGIC_NC) //input is high (button is pressed), but logic is normally closed (0)
			state = false; //input not activated, only active if input is 0 in this case
	}
	
	Ladder_OBJ::setLineState( state ); //let the parent handle it from here.
}