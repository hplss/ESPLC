/*
 * OpenPLC.cpp
 *
 * Created: 9/22/2019 4:19:51 PM
 * Author: Andrew Ward
 *This is the "main" for the entire program. Everything branches off from here.
 */ 
//WEB RELATED INCLUDES -- DO NOT ALTER THESE UNLESS YOU KNOW WHAT YOU'RE DOING
#include <vfs_api.h>
#include <FSImpl.h>
#include <FS.h>
#include <WiFiUdp.h>
#include <WiFiType.h>
#include <IPv6Address.h>
#include <WiFiSTA.h>
#include <WiFiServer.h>
#include <WiFiScan.h>
#include <WiFiMulti.h>
#include <WiFiGeneric.h>
#include <WiFiClient.h>
#include <WiFiAP.h>
#include <WiFi.h>
#include <ETH.h>
#include <WebServer.h>
#include <HTTP_Method.h>
#include <SPIFFS.h>
#include <esp_wifi.h>
//END WEB RELATED INCLUDES

#include <vector>
#include <PLC/PLC_IO.h>
#include <PLC/PLC_Main.h>
#include <PLC/PLC_Parser.h>
#include <PLC/PLC_Rung.h>
#include <CORE/UICore.h>
#include <esp_timer.h>
#include <HardwareSerial.h>
#include "Arduino.h"

//other ladder logic object includes
#include "./PLC/OBJECTS/MATH/obj_math_basic.h"
#include "./PLC/OBJECTS/obj_var.h"
#include "./PLC/OBJECTS/obj_input_basic.h"
#include "./PLC/OBJECTS/obj_output_basic.h"
#include "./PLC/OBJECTS/obj_timer.h"
#include "./PLC/OBJECTS/obj_counter.h"
//

PLC_Main PLCObj; //PLC ladder logic processing object. 
UICore Core; //UI object init -- for web and serial interfaces, as well as settings storage, etc.

//Main device setup function, called once at device power-up
void setup()
{
	millis(); //HACK HACK - calling this here seems to prevent millis() from crashing the device when a timer is used (weird bug). - DO NOT REMOVE
	Serial.begin(115200); //open the serial port 
	Core.setup(); //Initialize all core UI stuff. Should always be before the PLC_Main object is initialized (script is parsed), because certain settings in the FS should be loaded first.
	Core.loadPLCScript(PLCObj.getScript()); //load the script from the flash file system, 
	PLCObj.parseScript(PLCObj.getScript()); //Parses the PLC logic script given above (testing).
}



//Main device program loop - typically running on core 1 by default
void loop()
{
	Core.Process();
	PLCObj.processLogic(); //call loop for PLC
	//other input stuff here
}
