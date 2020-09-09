#ifndef PLC_IO_OBJ_ONESHOT
#define PLC_IO_OBJ_ONESHOT

#include "../PLC_IO.h"
#include "obj_var.h"

//The oneshot object is am object that pulses high for one scan cycle, then holds low until reset or powered high again
class OneshotOBJ : public Ladder_OBJ
{
    public:
    OneshotOBJ( const String &id, uint8_t type = TYPE_ONS ) : Ladder_OBJ(id, type)
    {

    }
    virtual void updateObject();






};

#endif