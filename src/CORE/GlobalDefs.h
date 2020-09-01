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
#include <vector>
#include <HardwareSerial.h>
#include <stdlib_noniso.h>
#include <map>

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
					&statusDir PROGMEM,
					&alertsDir PROGMEM,
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
			 		&html_form_Begin PROGMEM,
			 		&html_form_Middle PROGMEM,
			 		&html_form_End PROGMEM,
					&table_title_messages PROGMEM,
					&html_paragraph_begin PROGMEM,
					&html_paragraph_end PROGMEM,
					&field_title_alerts PROGMEM;

//

//PLC related error messages
extern const String &err_failed_creation PROGMEM,
			 		&err_unknown_args PROGMEM,
			 		&err_insufficient_args PROGMEM,
			 		&err_unknown_type PROGMEM,
					&err_pin_invalid PROGMEM,
					&err_pin_taken PROGMEM,
					&err_unknown_obj PROGMEM,
					&err_invalid_bit PROGMEM,
					&err_parser_failed PROGMEM;

//

//End storage related constants

enum OBJ_TYPE
{
	TYPE_INPUT = 0,		//physical input (digital read)
	TYPE_INPUT_ANALOG,  //physical input (analog read)
	TYPE_OUTPUT,		//physical output
	TYPE_VIRTUAL,		//internal coil (variable)
	TYPE_CLOCK,			//clock object type 
	TYPE_TON,			//timed on
	TYPE_TOF,			//timed off
	TYPE_TRET,			//retentive timer
	TYPE_BIT,			//represents the value of a bit stored by another object
	TYPE_CTU,			//count up timer
	TYPE_CTD,			//count down timer
	TYPE_ONS,			//one shot objects. Pulses high briefly, then goes low. Will not pulse until low-> high transition occurs. 
	TYPE_MATH_EQ,		//equals
	TYPE_MATH_GRT,		//greater than
	TYPE_MATH_LES,		//less than
	TYPE_MATH_GRQ,		//greater or equal to
	TYPE_MATH_LEQ,		//lesser or equal to
	TYPE_MATH_SIN,		//sine function
	TYPE_MATH_COS,		//cosine function
	TYPE_MATH_TAN,		//tangent function
	TYPE_MATH_ASIN,		//sine function
	TYPE_MATH_ACOS,		//cosine function
	TYPE_MATH_ATAN,		//tangent function
	TYPE_MATH_LIMIT,	//limit comparison block -- LOW_VAL <= IN <= HIGH VAL
	TYPE_MATH_INC,		//increment block - adds +1 to the inputted source variable
	TYPE_MATH_DEC,		//decrement block - subtracts 1 from the inputted source variable
	TYPE_MATH_CPT,		//compute block. Performs a math operation and sends the calculated value to the provided storage variable
	TYPE_MATH_MOV,
	TYPE_REMOTE,		//Remote object. Ued in cluster and expander operations when multiple ESP devices are interconnected via networks.

	//Variable Exclusive Types
	TYPE_VAR_UBYTE,		//variable type, used to store information (8-bit unsigned integer)
	TYPE_VAR_USHORT,	//variable type, used to store information (16-bit unsigned integer)
	TYPE_VAR_INT,		//variable type, used to store information (integers - 32bit)
	TYPE_VAR_UINT,		//variable type, uder to store information (unsigned integers - 32bit)
	TYPE_VAR_BOOL,	    //variable type, used to store information (boolean)
	TYPE_VAR_FLOAT,		//variable type, used to store information (float/double)
	TYPE_VAR_LONG,		//variable type, used to store information (long int - 64bit)
	TYPE_VAR_ULONG,		//variable type, used to store information (unsigned long - 64bit)
	TYPE_VAR_STRING,	//variable type, used to store information (String)
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

enum PIN_TYPE
{
	PIN_I = 0, //indicates that the pin is a digital input only pin
	PIN_AI, //indicates that the pin can function as an analog and digital input only
	PIN_O,	//indicates that the pin is a digital output only pin
	PIN_IO,	//Indicates that the pin is a combination digital input/output pin
	PIN_AIO, //indicates that the pin can function as an analog input or digital output
	PIN_TAKEN, //indicates that a requested pin is no longer available to the parser.
	PIN_INVALID //indicates that a requested pin id invalid, and not available for use at any time.
};

enum ERR_DATA
{
	ERR_CREATION_FAILED = 0, //This error indicates that the creation of a specified ladder object has failed for unknown reasons (generic failure)
	ERR_UNKNOWN_TYPE, //This error indicates that an object type interpreted from the parser cannot be initialized, probably because it doesn't exist.
	ERR_UNKNOWN_ARGS,	//This error indicates that the arguments given to the parser through the script cannot be interpreted, or they do not exist
	ERR_INSUFFICIENT_ARGS, //This error type indicates that a ladder logic object creation has failed due to insufficient arguments provided by the end-user.
	ERR_INVALID_OBJ,	//This error type indicates that a ladder object defined by the user does not exist or is not recognized by the parser.
	ERR_INVALID_BIT, //This error type indicates that a bit referece to a given ladder logic object is not valid or associated with the given object.
	ERR_PIN_INVALID, //This error indicates that a given pin number used for IO operations is invalid (doesn't exist on the device, or is used for special operations)
	ERR_PIN_TAKEN, //This error indicates that a given pin number used for IO operations is already occupied by another object.
	ERR_PARSER_FAILED //This error indicatews that the parser failed to exit properly.
};

const char CHAR_EQUALS = '=',
		   CHAR_P_START = '(',
		   CHAR_P_END = ')',
		   CHAR_BRACKET_START = '[',
		   CHAR_BRACKET_END = ']',
		   CHAR_SPACE = ' ',
		   CHAR_COMMA = ',',
		   CHAR_AND = '*',
		   CHAR_OR = '+',
		   CHAR_VAR_OPERATOR = '.', //EX: Timer1.DN '.' indicates that we are accessing an object variable to get a state.
		   CHAR_NOT_OPERATOR = '/', //EX: /IN1 returns opposite state of IN1 logic state
		   CHAR_NEWLINE = '\n',
		   CHAR_CARRIAGE = '\r',
		   CHAR_NONE = 0;


extern const String &bitTagDN PROGMEM,
			 		&bitTagEN PROGMEM,
			 		&bitTagTT PROGMEM,
			 		&bitTagACC PROGMEM,
			 		&bitTagPRE PROGMEM,
					&bitTagDEST PROGMEM,

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
					&typeTagMEQ PROGMEM,
			 		&typeTagMNEQ PROGMEM,
			 		&typeTagMDEC PROGMEM,
			 		&typeTagMINC PROGMEM,
			 		&typeTagMSIN PROGMEM,
			 		&typeTagMCOS PROGMEM,
			 		&typeTagMTAN PROGMEM,
					&typeTagAnalog PROGMEM,
					&typeTagDigital PROGMEM,

			 		&timerTag1 PROGMEM,
			 		&timerTag2 PROGMEM,
			 		&inputTag1 PROGMEM,
			 		&inputTag2 PROGMEM,
					&counterTag1 PROGMEM,
			 		&counterTag2 PROGMEM,
			 		&variableTag1 PROGMEM,
			 		&variableTag2 PROGMEM,
			 		&outputTag1 PROGMEM,
			 		&outputTag2 PROGMEM;


//Generic Function Declarations 

//This function parses 64 bit integer lengths
int64_t parseInt( const String & );
float parseFloat( const String & );
//This overloaded function allows for the breaking of strings based on multiple characters in a single operation.
vector<String> splitString( const String &, const vector<char> &, const vector<char> &, const vector<char> &, bool = true);
//This overloaded function allows for the breaking of strings based on multiple characters in a single operation.
vector<String> splitString( const String &, const vector<char> &, bool = true, const char = 0, const char = 0);
//This function breaks strings apart based on a single inputted char and optional delimiter.
vector<String> splitString(const String &, const char, bool = true, const char = 0, const char = 0);
//Converts a 64bit unsigned integer into a string.
String uLongToStr(uint64_t, uint8_t = 10);
//Converts a signed 64-bit integer to a string.
String longToStr(int64_t, uint8_t = 10);
//Returns a vector containing tiers of text that are nested within specific inputted characters
multimap<int8_t, String> textWithin(const String &, char, char, int8_t = -1);
//Performs a check to see if a given set of characters are present in the inputted string.
bool strContains( const String &, const vector<char> & );
//Performs a check to see if a specific character is present in the inputted string.
bool strContains( const String &, const char );

bool strBeginsWith( const String &str, const vector<char> &c );
bool strBeginsWith( const String &str, const char c );
bool strEndsWith( const String &str, const vector<char> &c );
bool strEndsWith( const String &str, const char c );
#endif /* GLOBALDEFS_H_ */