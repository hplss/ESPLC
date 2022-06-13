
#ifndef PLC_CAN_INTERFACE
#define PLC_CAN_INTERFACE

#include "../PLC_IO.h"
#include "../PLC_Main.h"
#include "../OBJECTS/obj_var.h"
#include <ACAN2515.h>

class CANFrameOBJ;
class CANDataOBJ;

using CAN_FRAME_PTR = shared_ptr<CANFrameOBJ>;
using CAN_DATA_PTR = shared_ptr<CANDataOBJ>;

/*static const byte MCP2515_SCK  = 26 ; // SCK input of MCP2517 
static const byte MCP2515_MOSI = 19 ; // SDI input of MCP2517  
static const byte MCP2515_MISO = 18 ; // SDO output of MCP2517 
static const byte MCP2515_CS  = 17 ; // CS input of MCP2515 (adapt to your design) */
static const uint32_t QUARTZ_FREQUENCY = 20UL * 1000UL * 1000UL ; // 20 MHz

//This object serves as an interface for the MC2515 CAN adapter for use in interpreting CAN signals for logic operations. 
class PLC_CAN_ADAPTER : public Ladder_OBJ_Accessor
{
    public:
    PLC_CAN_ADAPTER( const String &id, const uint8_t SCK, const uint8_t MOSI, const uint8_t MISO, const uint8_t CS, const uint32_t baud = 250000 ) 
    : Ladder_OBJ_Accessor(id, OBJ_TYPE::TYPE_CAN )
    {
        SPI.begin(SCK, MISO, MOSI);
        ACAN2515Settings CANSettings (QUARTZ_FREQUENCY, baud);

        interface = make_shared<ACAN2515>(CS, SPI, 255); // Last argument is 255 -> no interrupt pin
        interface->begin(CANSettings, NULL);
    }
    ~PLC_CAN_ADAPTER()
    {
        interface->end();
    }

    virtual void updateObject();
    bool addNewFrame(CAN_FRAME_PTR);
    CAN_FRAME_PTR getFrameByID( const uint32_t id);

    bool b_enabled;

    private:
    vector<CAN_FRAME_PTR> frames;
    shared_ptr<ACAN2515> interface;
};

//This object is meant to serve as a means of organizing data received by the CAN interface
class CANFrameOBJ : public Ladder_OBJ_Logical
{
    public: 
    CANFrameOBJ(const String &name, const uint32_t id, const bool tx = false, const uint16_t txRate = 10 ) 
    : Ladder_OBJ_Logical(name, OBJ_TYPE::TYPE_CAN_FRAME ), iFrameID(id), bTX(tx), iTXRate(txRate) 
    {
        
    }

    virtual void updateObject(){ Ladder_OBJ_Logical::updateObject(); }

    const bool bTX;
    const uint32_t iFrameID;
    const uint16_t iTXRate; //Used for TX only

    uint32_t lastTXMillis = 0;
    uint64_t iFrameData; 

    private:
};

//This object is responsible for taking bits of raw CAN data and converting it into useful data for logic operations.
class CANDataOBJ : public Ladder_VAR
{
    CANDataOBJ(const String &id, CAN_FRAME_PTR parentFrame, const uint64_t mask, const uint8_t bitOffset, const double ratio = 1.0, const double offset = 0 ) 
    : Ladder_VAR(double(0) ,id), pParentFrame(parentFrame), dRatio(ratio), dOffset(offset), iBitMask(mask), iBitOffset(bitOffset) //const uint8_t bytePos, const uint8_t bitPos, const uint8_t length
    {
    }
    
    virtual void updateObject()
	{ 
        Ladder_VAR::setValue((getRawCANData()/dRatio) + dOffset); //Sets the computed value in the Ladder_VAR parent class;
		Ladder_VAR::updateObject(); 
	}

    const uint32_t getRawCANData()
    {
        return unpackData(iBitMask, iBitOffset, &pParentFrame->iFrameData);
    }

    //Updates the CAN frame as wel as the stored value
    void setValue(double val)
    {
        pParentFrame->iFrameData = (uint64_t)((val - dOffset)/dRatio);
        Ladder_VAR::setValue(val);
    }

    private:
    const double dRatio; //Used for the setValue calculation.
    const double dOffset;
    const CAN_FRAME_PTR pParentFrame;
    const uint64_t iBitMask; //This is the mask that is generated for interpreting data from the parent frame.
    const uint8_t iBitOffset;
};



#endif