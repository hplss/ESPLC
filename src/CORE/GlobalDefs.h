/*
 * GlobalDefs.h
 *
 * Created: 10/11/2019 9:24:44 PM
 *  Author: Andrew
 This file contains information that is shared globally throughout the entire program.
 */ 


#ifndef GLOBALDEFS_H_
#define GLOBALDEFS_H_
#include <WString.h>
#include <HardwareSerial.h>

#define DEBUG //comment out to remove debugging code.

using namespace std;

const unsigned char NULL_CHAR = 0;//'/0';
#define MAX_BUFFERSIZE 128

#define VERBOSE_MAX 2

#define PRIORITY_LOW 2
#define PRIORITY_HIGH 1

/*enum TIMEZONES 
{
	UTC = 1;
};*/

//directories for individual web pages
extern const String &styleDir PROGMEM,
			 		&adminDir PROGMEM,
			 		&scriptDir PROGMEM;
//

const char CMD_PREFIX = '/', // "\"
		   DATA_SPLIT = ':'; //Char used to split multiple strings of data in a serial commamnd stream

const char CMD_DISCONNECT = 'd',
		   CMD_CONNECT = 'c', //"SSID":"Password"
		   CMD_NETWORKS = 's', //scan
		   CMD_NETINFO = 'i', //request current network info <if connected>
		   CMD_AP = 'a', //"ssid":"password"
		   CMD_VERBOSE = 'v', //<mode> can be 0 or any non-zero value, as well as 'on' or 'off'
		   CMD_PROGRAM = 'p', //stores specified values to eeprom so that they will load automatically in the future
		   CMD_TIME = 't'; //Used to set system time. No args returns time, args set time.


//Storage related constants
extern const String &file_Stylesheet PROGMEM,
			        &file_Configuration PROGMEM,
			        &file_Script PROGMEM;
//

//Web UI constants
extern const String &transmission_HTML PROGMEM,
			 		&form_Begin PROGMEM,
			 		&form_Middle PROGMEM,
			 		&form_End PROGMEM;

//

//PLC related error messages
extern const String &err_failed_creation PROGMEM,
			 		&err_unknown_args,
			 		&err_insufficient_args,
			 		&err_unknown_type PROGMEM;

//

//End storage related constants

enum LADDER_OBJ //Lists the types of objects that are used by the parser.
{
	LADDER_OBJECT,
	LADDER_VARIABLE	
};

enum OBJ_TYPE
{
	TYPE_INPUT = 0,		//physical input
	TYPE_OUTPUT,		//physical output
	TYPE_VIRTUAL,		//internal coil (variable)
	TYPE_COF,
	TYPE_CON,
	TYPE_TON,			//timed on
	TYPE_TOF,			//timed off
	TYPE_TRET,			//retentive timer
	TYPE_BIT,			//represents the value of a bit stored by another object
	TYPE_CTU,			//count up timer
	TYPE_CTD,			//count down timer
	TYPE_ONS,			//one shot objects. Pulses high briefly, then goes low. Will not pulse until low-> high transition occurs. 
	TYPE_MATH_GRT,		//greater than
	TYPE_MATH_LES,		//less than
	TYPE_MATH_GRQ,		//greater or equal to
	TYPE_MATH_LEQ,		//lesser or equal to
	TYPE_MATH_LIMIT,	//limit comparison block -- LOW_VAL <= IN <= HIGH VAL
	TYPE_MATH_CMP,		//comparison block with basic statement
	TYPE_MATH_CPT,		//compute block. Performs a math operation and sends the calculated value to the provided storage variable.

	//Variable Exclusive Types
	TYPE_VAR_INT,		//variable type, used to store information (integers - 32bit)
	TYPE_VAR_UINT,		//variable type, uder to store information (unsigned integers - 32bit)
	TYPE_VAR_BOOL,	    //variable type, used to store information (boolean)
	TYPE_VAR_FLOAT,		//variable type, used to store information (float/double)
	TYPE_VAR_LONG,		//variable type, used to store information (long int - 64bit)
	TYPE_VAR_ULONG,
};

enum OBJ_LOGIC
{
	LOGIC_NC = 0,	//Normally Closed
	LOGIC_NO		//Normally Open
};

enum OBJ_STATE
{
	STATE_DISABLED = 0,  //line state to this object is high(true)
	STATE_ENABLED, //line state to this object is low(false)
	STATE_LATCHED, //worry about this later
	STATE_UNLATCHED
};

const char CHAR_EQUALS = '=',
		   CHAR_P_START = '(',
		   CHAR_P_END = ')',
		   CHAR_SPACE = ' ',
		   CHAR_COMMA = ',',
		   CHAR_AND = '*',
		   CHAR_OR = '+',
		   CHAR_VAR_OPERATOR = '.', //EX: Timer1.DN '.' indicates that we are accessing an object variable to get a state.
		   CHAR_NOT_OPERATOR = '/', //EX: /IN1 returns opposite state of IN1 logic state
		   CHAR_NEWLINE = '\n',
		   CHAR_CARRIAGE = '\r';


extern const String &bitTagDN PROGMEM,
			 		&bitTagEN PROGMEM,
			 		&bitTagTT PROGMEM,
			 		&bitTagACC PROGMEM,
			 		&bitTagPRE PROGMEM,

			 		&logicTagNO PROGMEM,
			 		&logicTagNC PROGMEM,

			 		&typeTagTOF PROGMEM,
			 		&typeTagTON PROGMEM,
			 		&typeTagCTD PROGMEM,
			 		&typeTagCTU PROGMEM,
			 		&typeTagMGRE PROGMEM,
			 		&typeTagMLES PROGMEM,
			 		&typeTagMGREE PROGMEM,
			 		&typeTagMLESE PROGMEM,

			 		&timerTag1 PROGMEM,
			 		&timerTag2 PROGMEM,
			 		&inputTag1 PROGMEM,
			 		&inputTag2 PROGMEM,
					&counterTag1 PROGMEM,
			 		&counterTag2 PROGMEM,
			 		&virtualTag1 PROGMEM,
			 		&virtualTag2 PROGMEM,
			 		&outputTag1 PROGMEM,
			 		&outputTag2 PROGMEM,
			 		&variableTag PROGMEM,
			 		&mathBasicTag PROGMEM;
#endif /* GLOBALDEFS_H_ */