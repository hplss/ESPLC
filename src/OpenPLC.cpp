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
#include "./PLC/OBJECTS/MATH/math_basic.h"

PLC_Main PLCObj; //PLC ladder logic processing object. 
UICore Core; //UI object init -- for web and serial interfaces, as well as settings storage, etc.

//
//const String &script = PSTR("TIMER1=TIMER(1000)\n SW4=INPUT(13)\n SW3=INPUT(25)\n SW2=INPUT(33)\n SW1=INPUT(32)\n POTENTIOMETER=INPUT(26)\n WHITE=OUTPUT(16)\n YELLOW=OUTPUT(17)\n BLUE=OUTPUT(18)\n GREEN=OUTPUT(19)\n RED=OUTPUT(21)\n SW1+SW2+SW3+SW4+POTENTIOMETER=WHITE=TIMER1\n SW2*TIMER1.DN=YELLOW\n SW3=BLUE\n SW4=GREEN\n SW1*SW3*POTENTIOMETER=RED\n");
//Main device setup function, called once at device power-up
void setup()
{
	millis(); //HACK HACK - calling this here seems to prevent millis() from crashing the device when a timer is used (weird bug). - DO NOT REMOVE
	Serial.begin(9600); //open the serial port 
	Core.setup(); //Initialize all core UI stuff. Should always be before the PLC_Main object is initialized (script is parsed), because certain settings in the FS should be loaded first.
	Core.loadPLCScript(PLCObj.getScript()); //load the script from the flash file system, 
	PLCObj.parseScript(PLCObj.getScript()); //Parses the PLC logic script given above (testing).
	Core.setupAccessPoint("ESPLC-DEV",""); //Create an access point by default to facilitate easier testing. (Not Secured)
}



//Main device program loop - typically running on core 1 by default
void loop()
{
	Core.Process();
	PLCObj.processLogic(); //call loop for PLC
	//other input stuff here
}
