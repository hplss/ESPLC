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
			 &statusDir PROGMEM = PSTR("/status"),
			 &alertsDir PROGMEM = PSTR("/alerts"),
             &scriptDir PROGMEM = PSTR("/script");
//

//PLC RELATED TAGS
const String &bitTagDN PROGMEM = PSTR("DN"), //Done
			 &bitTagEN PROGMEM = PSTR("EN"), //Enable
			 &bitTagTT PROGMEM = PSTR("TT"), //Timer Timing
			 &bitTagACC PROGMEM = PSTR("ACC"), //Accumulated value
			 &bitTagPRE PROGMEM = PSTR("PRE"), //Preset Value
			 &bitTagDEST PROGMEM = PSTR("DEST"),

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
			 &typeTagMDEC PROGMEM = PSTR("DEC"), //TYPE: MATH - Decrement by a single integer (1)
			 &typeTagMINC PROGMEM = PSTR("INC"), //TYPE: MATH - Increment by a single integer (1)
			 &typeTagMSIN PROGMEM = PSTR("SIN"), //TYPE: MATH - Sine function
			 &typeTagMCOS PROGMEM = PSTR("COS"), //TYPE: MATH - Cosine function
			 &typeTagMTAN PROGMEM = PSTR("TAN"), //TYPE: MATH - Tangent function
			 &typeTagAnalog PROGMEM = PSTR("ANALOG"), //input (and possibly output) identifier - for analog signals
			 &typeTagDigital PROGMEM = PSTR("DIGITAL"), //input (and possibly output) identifier - for digital signals

             &timerTag1 PROGMEM = PSTR("TIMER"), //Timer object
			 &timerTag2 PROGMEM = PSTR("TMR"), //Timer object alias
			 &inputTag1 PROGMEM = PSTR("INPUT"), //Input object
			 &inputTag2 PROGMEM = PSTR("IN"), //Input object alias
			 &counterTag1 PROGMEM = PSTR("COUNTER"), //Counter object
			 &counterTag2 PROGMEM = PSTR("CNTR"), //Counter object alias
			 &variableTag1 PROGMEM = PSTR("VARIABLE"), //Virtuals serve as boolean storage for outputs (instead of physical pins).
			 &variableTag2 PROGMEM = PSTR("VAR"), //Virtual object alias
			 &outputTag1 PROGMEM = PSTR("OUTPUT"), //Output object
			 &outputTag2 PROGMEM = PSTR("OUT"), //Output object alias
			 &movTag PROGMEM = PSTR("MOV"); //MOV blocks are responsible for transferring (copying) data between two variable objects.
//END PLC TAGS

//Storage related constants
const String &file_Stylesheet PROGMEM = PSTR("/style.css"),
			 &file_Configuration PROGMEM = PSTR("/config.cfg"),
			 &file_Script PROGMEM = PSTR("/PLC_SCRIPT.txt");
//

//Web UI Constants
const String &transmission_HTML PROGMEM = PSTR("text/html"),
			 &html_form_Begin PROGMEM = PSTR("<FORM action=\"."),
			 &html_form_Middle PROGMEM = PSTR("\" method=\"post\" id=\"form\">"),
			 &html_form_End PROGMEM = PSTR("</FORM>"),
			 &table_title_messages PROGMEM = PSTR("System Messages"),
			 &html_paragraph_begin PROGMEM = PSTR("<P>"),
			 &html_paragraph_end PROGMEM = PSTR("</P>"),
			 &field_title_alerts PROGMEM = PSTR("System Alerts");
//

//PLC Error Messages
const String &err_failed_creation PROGMEM = PSTR("Failed to create object."),
			 &err_unknown_args PROGMEM = PSTR("Unknown argument."),
			 &err_insufficient_args PROGMEM = PSTR("Insufficient arguments."),
			 &err_unknown_type PROGMEM = PSTR("Unknown object type."),
			 &err_pin_invalid PROGMEM = PSTR("Invalid pin for IO."),
			 &err_pin_taken PROGMEM = PSTR("IO pin already taken."),
			 &err_unknown_obj PROGMEM = PSTR("Invalid Object"),
			 &err_invalid_bit PROGMEM = PSTR("Invalid Bit"),
			 &err_parser_failed PROGMEM = PSTR("Parser operation failed.");
//
