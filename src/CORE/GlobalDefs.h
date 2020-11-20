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

#define MAX_PLC_OBJ_NAME 32 //32 chars seems like a reasonably long enough name for a ladder logic object.

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
					&updateDir PROGMEM,
					&firmwareDir PROGMEM,
			 		&scriptDir PROGMEM;
//

//PLC_Remote Constants for queries (non printable chars)
const char CMD_REQUEST_UPDATE = 17, //Data prefix for requesting (by a client) an update of relevant values associated with a given object ID
		   CMD_REQUEST_INIT = 18, //Data prefix for requesting (by a client) a complete detailing of an object with a given ID.
		   CMD_REQUEST_INVALID = 16, //Reply sent in the event that a request is invalid or was incorrectly parsed by the remote host.
		   CMD_SEND_UPDATE = 19, //Data prefix (from a host) for updating specific ladder objects. After a request for an update, this prefix is expected. 
		   CMD_SEND_INIT = 21, //Data prefix (from a host) indicating that the included data is meant to initialize a new object on the client for later reference.
		   CMD_SEND_REFRESH = 20, //Data prefix (from a host) indicating that all objects on the client should be refreshed and re-initialized. Possibly after an abrupt disconnect or device reset, or ogic script cahnge.
		   CHAR_UPDATE_GROUP = 29, //This character is used to denote the separation of a set of data records (pertaining to individual ladder objects) for serial or web updates.
		   CHAR_UPDATE_RECORD = 30, //This character is used to denote the separation of a data record as it pertains to receiving updates from serial or a web interface.
		   CHAR_QUERY_END = 15, //This character is appended to the end of the update string, and denotes the end of all update info. This must be included before updates are applied.
		   CHAR_TRANSMIT_END = 14; //This char is appended to the end of a string that has been transmitted between hosts. Used to mark the end of a read cycle.
//

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
		   CHAR_ACCESSOR_OPERATOR = ':', //EX: REMOTE:OBJECT.BIT
		   CHAR_MODIFIER_START = '{',
		   CHAR_MODIFIER_END = '}',
		   CHAR_NEWLINE = '\n',
		   CHAR_CARRIAGE = '\r',
		   CHAR_NONE = 0;

const char CMD_PREFIX = '/', // "\"
		   DATA_SPLIT = ':'; //Char used to split multiple strings of data in a serial or network command stream

const char CMD_DISCONNECT = 'd',
		   CMD_CONNECT = 'c', //"SSID":"Password"
		   CMD_NETWORKS = 's', //scan
		   CMD_NETINFO = 'i', //request current system status info
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
					&html_form_Middle_Upload PROGMEM,
			 		&html_form_End PROGMEM,
					&table_title_messages PROGMEM,
					&html_paragraph_begin PROGMEM,
					&html_paragraph_end PROGMEM,
					&field_title_alerts PROGMEM,
					&http_header_connection PROGMEM,
					&http_header_close PROGMEM;

//

//PLC related error messages
extern const String &err_failed_creation PROGMEM,
					&err_failed_accessor PROGMEM,
			 		&err_unknown_args PROGMEM,
			 		&err_insufficient_args PROGMEM,
			 		&err_unknown_type PROGMEM,
					&err_pin_invalid PROGMEM,
					&err_pin_taken PROGMEM,
					&err_unknown_obj PROGMEM,
					&err_invalid_bit PROGMEM,
					&err_name_too_long PROGMEM,
					&err_parser_failed PROGMEM,
					&err_var_type_invalid PROGMEM,
					&err_var_out_of_range PROGMEM,
					&err_invalid_function PROGMEM,
					&err_math_too_many_args PROGMEM,
					&err_math_too_few_args PROGMEM,
					&err_math_division_by_zero PROGMEM;

//Variable string definitions
extern const String &VAR_INT32 PROGMEM,
					&VAR_UINT32 PROGMEM,
					&VAR_INT64 PROGMEM,
					&VAR_UINT64 PROGMEM,
					&VAR_DOUBLE PROGMEM,
					&VAR_BOOL PROGMEM,
					&VAR_BOOLEAN PROGMEM;



//End storage related constants

//The OBJ_TYPE enum contains identifiers that indicate a given LADDER_OBJ object's functionality. This is typically used by the parser and object creation functions.
enum class OBJ_TYPE : uint8_t
{
	TYPE_INPUT,		//physical input (digital read)
	TYPE_INPUT_ANALOG,  //physical input (analog read)
	TYPE_OUTPUT,		//physical output - digital (1/0)
	TYPE_OUTPUT_PWM, 	//physical output - PWM output type
	TYPE_VIRTUAL,		//internal coil (variable)
	TYPE_CLOCK,			//clock object type 
	TYPE_TIMER_ON,			//timed on
	TYPE_TIMER_OFF,			//timed off
	TYPE_TIMER_RET,			//retentive timer
	TYPE_COUNTER_UP,			//count up timer
	TYPE_COUNTER_DOWN,			//count down timer
	TYPE_ONS,			//one shot objects. Pulses high briefly, then goes low. Will not pulse until low-> high transition occurs. 
	TYPE_MATH_MUL, //Multiply
	TYPE_MATH_DIV, //Divide
	TYPE_MATH_ADD, //Addition
	TYPE_MATH_SUB, //Subtraction
	TYPE_MATH_EQ,		//equals
	TYPE_MATH_NEQ,	//Not equal
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
	TYPE_MATH_MOV,		//Move block, used for transferring data from a source to another destination (IE: From Source to Source, Dest to Source, etc.)
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

enum OBJ_LOGIC : uint8_t 
{
	LOGIC_NC,	//Normally Closed
	LOGIC_NO		//Normally Open
};

enum class PWM_STATUS : uint8_t 
{
	PWM_AVAILABLE,
	PWM_TAKEN
};

//This enum denotes the varying states that an object may have. 
enum OBJ_STATE : uint8_t 
{
	STATE_DISABLED,  //line state to this object is high(true)
	STATE_ENABLED, //line state to this object is low(false)
	STATE_LATCHED, //worry about this later
	STATE_UNLATCHED 
};

//This enum contains a list of valid pin types as it pertains to the IO capability of the ESP-32. 
//This is typically used by the logic parser to verify that a requested pin argument is valid.
enum class PIN_TYPE : uint8_t
{
	PIN_I, //indicates that the pin is a digital input only pin
	PIN_AI, //indicates that the pin can function as an analog and digital input only
	PIN_O,	//indicates that the pin is a digital output only pin
	PIN_IO,	//Indicates that the pin is a combination digital input/output pin
	PIN_AIO, //indicates that the pin can function as an analog input or digital output
	PIN_TAKEN, //indicates that a requested pin is no longer available to the parser.
	PIN_INVALID //indicates that a requested pin id invalid, and not available for use at any time.
};

//This enum contains identifiers that indicate specific error types.
enum class ERR_DATA : uint8_t 
{
	ERR_CREATION_FAILED, //This error indicates that the creation of a specified ladder object has failed for unknown reasons (generic failure)
	ERR_UNKNOWN_TYPE, //This error indicates that an object type interpreted from the parser cannot be initialized, probably because it doesn't exist.
	ERR_UNKNOWN_ARGS,	//This error indicates that the arguments given to the parser through the script cannot be interpreted, or they do not exist
	ERR_INSUFFICIENT_ARGS, //This error type indicates that a ladder logic object creation has failed due to insufficient arguments provided by the end-user.
	ERR_INVALID_OBJ,	//This error type indicates that a ladder object defined by the user does not exist or is not recognized by the parser.
	ERR_INVALID_ACCESSOR, //This error type indicates that an accessor used to access other adder objects failed to initialize or is invalid.
	ERR_INVALID_BIT, //This error type indicates that a bit referece to a given ladder logic object is not valid or associated with the given object.
	ERR_PIN_INVALID, //This error indicates that a given pin number used for IO operations is invalid (doesn't exist on the device, or is used for special operations)
	ERR_PIN_TAKEN, //This error indicates that a given pin number used for IO operations is already occupied by another object.
	ERR_PARSER_FAILED, //This error indicatews that the parser failed to exit properly.
	ERR_NAME_TOO_LONG, //This indicates that the name of an object being parsed is too long to be stored (exceeds allowed memory usage)
	ERR_INCORRECT_VAR_TYPE, //Error for an incorrect variable type when naming a variable type
	ERR_OUT_OF_RANGE, //This indicates that the value assigned exceeds the maximum value of the variable type
	ERR_INVALID_FUNCTION, //This indicates that a function was given to a math object that is not supported.
	ERR_MATH_TOO_MANY_ARGS, //This indicates that a function was given too many arguments to use.
	ERR_MATH_TOO_FEW_ARGS, //This indicates that a function was given too few arguments to use.
	ERR_MATH_DIV_BY_0 //This indicates that a division by zero was about to occur.
};


extern const String &bitTagDN PROGMEM,
			 		&bitTagEN PROGMEM,
			 		&bitTagTT PROGMEM,
			 		&bitTagACC PROGMEM,
			 		&bitTagPRE PROGMEM,
					&bitTagSRCA PROGMEM,
					&bitTagSRCB PROGMEM,
					&bitTagDEST PROGMEM,
					&bitTagVAL PROGMEM,

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
					&typeTagMASIN PROGMEM,
			 		&typeTagMACOS PROGMEM,
			 		&typeTagMATAN PROGMEM,
					&typeTagMMUL PROGMEM,
					&typeTagMDIV PROGMEM,
					&typeTagMADD PROGMEM,
					&typeTagMSUB PROGMEM,
					&typeTagMMOV PROGMEM,
					&typeTagAnalog PROGMEM,
					&typeTagDigital PROGMEM,
					&typeTagPWM PROGMEM,

			 		&timerTag1 PROGMEM,
			 		&timerTag2 PROGMEM,
			 		&inputTag1 PROGMEM,
			 		&inputTag2 PROGMEM,
					&counterTag1 PROGMEM,
			 		&counterTag2 PROGMEM,
			 		&variableTag1 PROGMEM,
			 		&variableTag2 PROGMEM,
			 		&outputTag1 PROGMEM,
			 		&outputTag2 PROGMEM,
					&mathTag PROGMEM,
					&remoteTag PROGMEM,
					&oneshotTag PROGMEM;


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

void reverse(char* begin, char* end); //taken from 'stdlib_noniso.c'

template <typename T>
char* intToASCII(T value, char* result, uint8_t base = 10) 
{
    if(base < 2 || base > 16) {
        *result = 0;
        return result;
    }

    char* out = result;
    T quotient = llabs(value);

    do {
        const T tmp = quotient / base;
        *out = "0123456789abcdef"[quotient - (tmp * base)];
        ++out;
        quotient = tmp;
    } while(quotient);

    // Apply negative sign
    if(value < 0)
        *out++ = '-';

    reverse(result, out);
    *out = 0;
    return result;
}

template <typename T>
String intToStr( const T value, uint8_t base = 10 )
{
	char buf[2 + 8 * sizeof(T)];
    intToASCII(value, buf, base);
    
	String str(buf);
    return str;
}


//Returns a vector containing tiers of text that are nested within specific inputted characters
multimap<int8_t, String> textWithin(const String &, char, char, int8_t = -1);
//Performs a check to see if a given set of characters are present in the inputted string.
bool strContains( const String &, const vector<char> & );
//Performs a check to see if a specific character is present in the inputted string.
bool strContains( const String &, const char );

bool strBeginsWith( const String &, const vector<char> & );
bool strBeginsWith( const String &, const char  );
bool strEndsWith( const String &, const vector<char> & );
bool strEndsWith( const String &, const char );
//Remove specific character(s) from a given string.
String removeFromStr( const String &, const vector<char> &);
String removeFromStr( const String &, const char);
#endif /* GLOBALDEFS_H_ */