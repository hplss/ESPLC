#ifndef PLC_IO_OBJ_REMOTE
#define PLC_IO_OBJ_REMOTE

#include "../PLC_IO.h"

//The Remote_Ladder_OBJ represents an object that is initialized on an external "ESPLC" device.
class Remote_Ladder_OBJ : public Ladder_OBJ
{
	public:
	Remote_Ladder_OBJ( const String &id, uint8_t type, uint16_t remoteID ) : Ladder_OBJ( id, TYPE_REMOTE )
	{
		iRemoteType = type;
		iRemoteID = remoteID;
	}
	
	virtual void setLineState(bool &state)
	{ 
		Ladder_OBJ::setLineState(state); 
	}
	virtual void updateObject()
	{
		//Execute output operations, etc. post scan -- based on remote type.
		Ladder_OBJ::updateObject(); //parent class - must be called last
	}

	uint16_t getRemoteID(){ return iRemoteID; }
	uint8_t getremoteType() { return iRemoteType; }

	private:
	uint8_t iRemoteType; //This is the object type as it pertains to the remote controller. 
	uint16_t iRemoteID; //This is the object ID as it pertains to the remote controller
};

#endif