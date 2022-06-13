#include "acc_CAN.h"

void PLC_CAN_ADAPTER::updateObject()
{
    while( interface->available() ) //if?
    {
        CANMessage frame;
        interface->receive(frame);
        
        CAN_FRAME_PTR ptr = getFrameByID(frame.id);
        if ( !ptr )
            continue;

        ptr->iFrameData = frame.data64;
    }

    for ( uint16_t x = 0; x < frames.size(); x++ )
    {
        CAN_FRAME_PTR ptr = frames[x];

        if ( !frames[x]->bTX || (ptr->lastTXMillis + ptr->iTXRate) > millis() )
            continue;

        CANMessage frame;
        frame.ext = true;
        frame.id = ptr->iFrameID;
        frame.data64 = ptr->iFrameData;

        interface->tryToSend(frame); //Update TX time?
        ptr->lastTXMillis = millis();
    }

    Ladder_OBJ_Accessor::updateObject();
}

bool PLC_CAN_ADAPTER::addNewFrame(CAN_FRAME_PTR ptr)
{
    if (!ptr || getFrameByID(ptr->iFrameID))
        return false;

    frames.push_back(ptr);
    return true;
}

CAN_FRAME_PTR PLC_CAN_ADAPTER::getFrameByID(uint32_t id)
{
    for ( uint16_t x = 0; x < frames.size(); x++ )
    {
        if ( id == frames[x]->iFrameID )
            return frames[x];
    }

    return 0;
}

