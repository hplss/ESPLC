#include "obj_input_basic.h"


//////////////////////////////////////////////////////////////////////////
// INPUT OBJECT BEGIN
//////////////////////////////////////////////////////////////////////////
void InputOBJ::updateObject()
{
	Ladder_OBJ_Logical::updateObject(); //parent class - must be called last
}

void InputOBJ::setLineState( bool &state, bool bNot)
{
	iValue = getInput();

	if (state) //must have a HIGH state coming into the object before performing actions (indicates that the previous object had a successful pass)
	{
		if ( getType() == OBJ_TYPE::TYPE_INPUT ) //digital input only (0/1)
		{
			if(!iValue && getLogic() == LOGIC_NO) //input is low (button not pressed) and logic is normally open (default position of button is off)
				state = (bNot ? true : false); //input not activated
			else if (iValue && getLogic() == LOGIC_NC) //input is high (button is pressed), but logic is normally closed (0)
				state = (bNot ? true : false); //input not activated, only active if input is 0 in this case
			else if ( bNot )
				state = false;
		}
		else if ( getType() == OBJ_TYPE::TYPE_INPUT_ANALOG ) //we can still treat analog signals as a logic high or low, but we must have a threshhold that must be crossed.
		{ //2700 is ~ 2/3 of 4096 (12 bit analog reading), so we'll roll with that for now
			if( ( iValue < 2700 ) && getLogic() == LOGIC_NO) //input is low (button not pressed) and logic is normally open (default position of button is off)
				state = (bNot ? true : false); //input not activated
			else if ( ( iValue >= 2700 ) && getLogic() == LOGIC_NC) //input is high (button is pressed), but logic is normally closed (0)
				state = (bNot ? true : false); //input not activated, only active if input is 0 in this case
			else if ( bNot )
				state = false;
		}
	}
	
	Ladder_OBJ_Logical::setLineState( state, bNot ); //let the parent handle anything else from here.
}
