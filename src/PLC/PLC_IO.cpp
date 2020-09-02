/*
 * PLC_IO.cpp
 *
 * Created: 9/22/2019 4:55:00 PM
 *  Author: Andrew Ward
 The purpose of the PLC_IO Objects is to serve as the actual logic objects in a ladder rung. These objects each have their own logic operations that they perform,
 and the state of the output line (from that object to the next) is passed along to the next object in the chain. This allows it to perform necessary actions based
 on that logic state. Some objects may have multiple objects that they point to (the case for an OR statement, for example), in this case: the rung manager will 
 note the position of these 'branches' in its own object, and will remember to each point in the logic after processing the previous pathway to the output(s).
 Objects include Timers, Counters, and standard I/O (as well as virtual 'coils').
 */ 

#include "PLC_IO.h"
#include <HardwareSerial.h>
#include <esp_timer.h>

//////////////////////////////////////////////////////////////////////////
// BASECLASS OBJECT BEGIN
//////////////////////////////////////////////////////////////////////////

shared_ptr<Ladder_VAR> Ladder_OBJ::getObjectVAR( const String &id )
{
	return 0;
}

shared_ptr<Ladder_VAR> Ladder_OBJ::addObjectVAR( const String &id )
{
	//Error here? Should only be called if the derived class doesn't support the requested bit tag. Like a counter referencing the TT bit.
	return 0;
}


