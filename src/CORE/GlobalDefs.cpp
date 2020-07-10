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
const String &bitTagDN PROGMEM = PSTR("DN"), //Done
			 &bitTagEN PROGMEM = PSTR("EN"), //Enable
			 &bitTagTT PROGMEM = PSTR("TT"), //Timer Timing
			 &bitTagACC PROGMEM = PSTR("ACC"), //Accumulated value
			 &bitTagPRE PROGMEM = PSTR("PRE"), //Preset Value

             &logicTagNO PROGMEM = PSTR("NO"), //Normally open contact
			 &logicTagNC PROGMEM = PSTR("NC"), //Normally closed contact

             &typeTagTOF PROGMEM = PSTR("TOF"), //TYPE: Timer-off
			 &typeTagTON PROGMEM = PSTR("TON"), //TYPE: Timer-on
			 &typeTagCTD PROGMEM = PSTR("CTD"), //TYPE: Counter-down
			 &typeTagCTU PROGMEM = PSTR("CTU"), //TYPE: Counter-up
			 &typeTagMGRE PROGMEM = PSTR("GRE"), //TYPE: MATH - Greater than
			 &typeTagMLES PROGMEM = PSTR("LES"), //TYPE: MATH - Less than
			 &typeTagMGREE PROGMEM = PSTR("GRQ"), //TYPE: MATH - Greater or Equal to
			 &typeTagMLESE PROGMEM = PSTR("LEQ"), //TYPE: MATH - Lesser or equal to
			 &typeTagMEQ PROGMEM = PSTR("EQ"), //TYPE: MATH - Equal to
			 &typeTagMNEQ PROGMEM = PSTR("NEQ"), //TYPE: MATH - Not Equal to

             &timerTag1 PROGMEM = PSTR("TIMER"), //Timer object
			 &timerTag2 PROGMEM = PSTR("TMR"), //Timer object alias
			 &inputTag1 PROGMEM = PSTR("INPUT"), //Input object
			 &inputTag2 PROGMEM = PSTR("IN"), //Input object alias
			 &counterTag1 PROGMEM = PSTR("COUNTER"), //Counter object
			 &counterTag2 PROGMEM = PSTR("CNTR"), //Counter object alias
			 &virtualTag1 PROGMEM = PSTR("VIRTUAL"), //Virtuals serve as boolean storage for outputs (instead of physical pins).
			 &virtualTag2 PROGMEM = PSTR("VIRT"), //Virtual object alias
			 &outputTag1 PROGMEM = PSTR("OUTPUT"), //Output object
			 &outputTag2 PROGMEM = PSTR("OUT"), //Output object alias
			 &variableTag PROGMEM = PSTR("VAR"), //Variables serve as memory to be allocated for math comparisons of any kind. It is possible to have multiple data types as a VAR
			 &mathBasicTag PROGMEM = PSTR("MATH"), //Math blocks perform simple arithmatic calculations and comparisons
			 &movTag PROGMEM = PSTR("MOV"); //MOV blocks are responsible for transferring (copying) data between two variable objects.
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
