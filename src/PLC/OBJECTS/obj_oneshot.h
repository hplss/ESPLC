#ifndef PLC_IO_OBJ_ONESHOT
#define PLC_IO_OBJ_ONESHOT

#include "../PLC_IO.h"
#include "obj_var.h"

//The oneshot object is am object that pulses high for one scan cycle, then holds low until reset or powered high again
class OneshotOBJ : public Ladder_OBJ_Logical
{
    public:
    OneshotOBJ() : Ladder_OBJ_Logical("", OBJ_TYPE::TYPE_ONS)
    {
        accum = false;
    }
    ~OneshotOBJ()
    {
        #ifdef DEBUG
		Serial.println(PSTR("Oneshot Destructor")); 
		#endif
    }
    virtual void updateObject();
	virtual void setLineState(bool &, bool );

    private:
    bool accum;
};

#endif