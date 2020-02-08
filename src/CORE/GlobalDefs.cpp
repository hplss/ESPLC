/*
 * GlobalDefs.cpp
 *
 * Created: 1/2/2020 11:50:05 AM
 *  Author: Andrew Ward
 *  This file exists mainly to initialize const references for PROGMEM strings.
 */ 

#include "GlobalDefs.h"

//directories for individual web pages
const String &styleDir PROGMEM = PSTR("/style"),
             &adminDir PROGMEM = PSTR("/admin"),
             &scriptDir PROGMEM = PSTR("/script");
//

//PLC RELATED TAGS
const String &bitTagDN PROGMEM = PSTR("DN"),
			 &bitTagEN PROGMEM = PSTR("EN"),
			 &bitTagTT PROGMEM = PSTR("TT"),
			 &bitTagACC PROGMEM = PSTR("ACC"),
			 &bitTagPRE PROGMEM = PSTR("PRE"),

             &logicTagNO PROGMEM = PSTR("NO"),
			 &logicTagNC PROGMEM = PSTR("NC"),

             &typeTagTOF PROGMEM = PSTR("TOF"),
			 &typeTagTON PROGMEM = PSTR("TON"),
			 &typeTagCTD PROGMEM = PSTR("CTD"),
			 &typeTagCTU PROGMEM = PSTR("CTU"),
			 &typeTagMGRE PROGMEM = PSTR("GRE"),
			 &typeTagMLES PROGMEM = PSTR("LES"),
			 &typeTagMGREE PROGMEM = PSTR("GRQ"),
			 &typeTagMLESE PROGMEM = PSTR("LEQ"),

             &timerTag1 PROGMEM = PSTR("TIMER"),
			 &timerTag2 PROGMEM = PSTR("TMR"),
			 &inputTag1 PROGMEM = PSTR("INPUT"),
			 &inputTag2 PROGMEM = PSTR("IN"),
			 &counterTag1 PROGMEM = PSTR("COUNTER"),
			 &counterTag2 PROGMEM = PSTR("CNTR"),
			 &virtualTag1 PROGMEM = PSTR("VIRTUAL"),
			 &virtualTag2 PROGMEM = PSTR("VIRT"),
			 &outputTag1 PROGMEM = PSTR("OUTPUT"),
			 &outputTag2 PROGMEM = PSTR("OUT"),
			 &variableTag PROGMEM = PSTR("VAR"),
			 &mathBasicTag PROGMEM = PSTR("MATH");
//END PLC TAGS

//Storage related constants
const String &file_Stylesheet PROGMEM = PSTR("/style.css"),
			 &file_Configuration PROGMEM = PSTR("/config.cfg"),
			 &file_Script PROGMEM = PSTR("/PLC_SCRIPT.txt");
//

//Web UI Constants
const String &transmission_HTML PROGMEM = PSTR("text/html"),
			 &form_Begin PROGMEM = PSTR("<FORM action=\"."),
			 &form_Middle PROGMEM = PSTR("\" method=\"post\" id=\"form\">"),
			 &form_End PROGMEM = PSTR("</FORM>");
//

//PLC Error Messages
const String &err_failed_creation PROGMEM = PSTR("Failed to create object."),
			 &err_unknown_args = PSTR("Unknown argument."),
			 &err_insufficient_args = PSTR("Insufficient arguments."),
			 &err_unknown_type PROGMEM = PSTR("Unknown type."),
			 &err_unknown_obj = PSTR("Object not defined."),
			 &err_invalid_bit = PSTR("Invalid Bit");
//
