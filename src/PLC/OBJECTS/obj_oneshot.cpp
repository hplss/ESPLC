#include "obj_oneshot.h"


//////////////////////////////////////////////////////////////////////////
// ONESHOT OBJECT BEGIN
//////////////////////////////////////////////////////////////////////////
void OneshotOBJ::updateObject()
{
    Ladder_OBJ::updateObject();
}

void OneshotOBJ::setLineState( bool &state, bool bNot)
{
    if(state)//must have a HIGH state coming into the object
    {
        if( getType() == TYPE_ONS )
        {
            if(lAccum>0)//has the object already pulsed during this iteration of the linestate?
            {
                state = false;//set state LOW since object has already pulsed this iteration
            }
            else
            {
                lAccum++;//increment the accumulator to show the object has pulsed on this iteration
                state = true;//set the outgoing state to HIGH
            }
        }
    }

    else//incoming state is LOW
    {
        state = false;//outgoing state always LOW if incoming is LOW
        lAccum = 0;//clear accumulator so that the object will pulse on the next HIGH linestate
    }

    Ladder_OBJ::setLineState( state, bNot );
}

