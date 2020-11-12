#include "obj_oneshot.h"


//////////////////////////////////////////////////////////////////////////
// ONESHOT OBJECT BEGIN
//////////////////////////////////////////////////////////////////////////
void OneshotOBJ::updateObject()
{
    Ladder_OBJ_Logical::updateObject();
}

void OneshotOBJ::setLineState( bool &state, bool bNot)
{
    if(state)//must have a HIGH state coming into the object
    {
        if(accum)//has the object already pulsed during this iteration of the linestate?
        {
            state = false;//set state LOW since object has already pulsed this iteration
        }
        else
        {
            accum = true;//increment the accumulator to show the object has pulsed on this iteration
        }
    }

    else if(accum)//incoming state is LOW
    {
        accum = false;//clear accumulator so that the object will pulse on the next HIGH linestate
    }

    Ladder_OBJ_Logical::setLineState( state, bNot );
}

