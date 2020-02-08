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
#include <PLC/PLC_Rung.h>
#include <PLC/PLC_VAR.h>
#include <CORE/UICore.h>
#include <esp_timer.h>
#include <HardwareSerial.h>
#include "Arduino.h"


PLC_Main PLCObj;
UICore Core; //Device object init
//
const String &script = PSTR("IN1=INPUT(22)\n IN2=INPUT(2)\n IN3=INPUT(15)\n IN4=INPUT(13)\n IN5=INPUT(25)\n IN6=INPUT(33)\n IN7=INPUT(32)\n IN8=INPUT(26)\n IN9=INPUT(14)\n IN10=INPUT(27)\n OUT1=OUTPUT(16)\n OUT2=OUTPUT(17)\n OUT3=OUTPUT(18)\n OUT4=OUTPUT(19)\n OUT5=OUTPUT(21)\n IN1*IN2*IN3*IN4*IN5*IN6*IN7=OUT1\n IN8=OUT2\n IN9=OUT3\n IN10=OUT4\n IN1+IN3+IN9=OUT5\n");
void setup()
{
	millis(); //HACK HACK - calling this here seems to prevent millis() from crashing the device when a timer is used.
	Serial.begin(9600); //open the serial port //250000
	Core.setup(); //Initialize all core UI.
	PLCObj.parseScript(script); //Parses the PLC logic script.
	//Map IO Pins and setup here.
}

void loop()
{
	Core.Process(); //handle all core UI inputs, etc.
	PLCObj.processLogic(); //call loop for PLC
	//other input stuff here

}
