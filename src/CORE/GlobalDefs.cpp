/*
 * GlobalDefs.cpp
 *
 * Created: 1/2/2020 11:50:05 AM
 *  Author: Andrew Ward
 *  This file exists mainly to initialize const references for PROGMEM strings.
 */ 

#include "GlobalDefs.h"
#include <stdlib.h>

//directories for individual web pages
const String &styleDir PROGMEM = PSTR("/style"),
             &adminDir PROGMEM = PSTR("/admin"),
			 &statusDir PROGMEM = PSTR("/status"),
			 &alertsDir PROGMEM = PSTR("/alerts"),
			 &filesDir PROGMEM = PSTR("/files"),
			 &editDir PROGMEM = PSTR("/edit"),
			 &updateDir PROGMEM = PSTR("/update"),
			 &firmwareDir PROGMEM = PSTR("/firmware"),
             &scriptDir PROGMEM = PSTR("/script");
//

//PLC RELATED TAGS
const String &bitTagDN PROGMEM = PSTR("DN"), //Done
			 &bitTagEN PROGMEM = PSTR("EN"), //Enable
			 &bitTagTT PROGMEM = PSTR("TT"), //Timer Timing
			 &bitTagACC PROGMEM = PSTR("ACC"), //Accumulated value
			 &bitTagPRE PROGMEM = PSTR("PRE"), //Preset Value
			 &bitTagSRCA PROGMEM = PSTR("A"), //Source A variable input
			 &bitTagSRCB PROGMEM = PSTR("B"), //Source B variabel input
			 &bitTagDEST PROGMEM = PSTR("DEST"), //Destination
			 &bitTagVAL PROGMEM = PSTR("VAL"), //Value bit - generic
			 &bitTagSP PROGMEM = PSTR("SP"), //Setpoint component
			 &bitTagKP PROGMEM = PSTR("KP"), //Proportional Component
			 &bitTagKI PROGMEM = PSTR("KI"), //Integral component
			 &bitTagKD PROGMEM = PSTR("KD"), //Derivative component
			 &bitTagPV PROGMEM = PSTR("PV"), //Process Variable
			 &bitTagCV PROGMEM = PSTR("CV"), //Control Variable

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
			 &typeTagMASIN PROGMEM = PSTR("ASIN"), //TYPE: MATH - ArcSine function
			 &typeTagMACOS PROGMEM = PSTR("ACOS"), //TYPE: MATH - ArcCosine function
			 &typeTagMATAN PROGMEM = PSTR("ATAN"), //TYPE: MATH - ArcTangent function
			 &typeTagMMUL PROGMEM = PSTR("MUL"), //TYPE: MATH - Multiply function
			 &typeTagMDIV PROGMEM = PSTR("DIV"), //TYPE: MATH - Division function
			 &typeTagMADD PROGMEM = PSTR("ADD"), //TYPE: MATH - Addition function
			 &typeTagMSUB PROGMEM = PSTR("SUB"), //TYPE: MATH - Subtraction function
			 &typeTagMMOV PROGMEM = PSTR("MOV"), //MOV blocks are responsible for transferring (copying) data between two variable objects.
			 &typeTagAnalog PROGMEM = PSTR("ANALOG"), //input (and possibly output) identifier - for analog signals
			 &typeTagDigital PROGMEM = PSTR("DIGITAL"), //input (and possibly output) identifier - for digital signals
			 &typeTagPWM PROGMEM = PSTR("PWM"), //Pulse width modulation 

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
			 &mathTag PROGMEM = PSTR("MATH"), //Math object alias
			 &remoteTag PROGMEM = PSTR("REMOTE"), //remote object
			 &CANTag PROGMEM = PSTR("CAN"), //CAN object
			 &CANFrameTag PROGMEM = PSTR("CANFRAME"), //Frame data for a CAN interface
			 &oneshotTag PROGMEM = PSTR("ONS"); //oneshot object
//END PLC TAGS

//Storage related constants
const String &file_Stylesheet PROGMEM = PSTR("/style.css"),
			 &file_Configuration PROGMEM = PSTR("/config.cfg"),
			 &file_Script PROGMEM = PSTR("/PLC_SCRIPT.txt");
//

//Web UI Constants
const String &transmission_HTML PROGMEM = PSTR("text/html"),
			 &transmission_Plain PROGMEM = PSTR("text/plain"),
			 &html_form_Begin PROGMEM = PSTR("<FORM action=\"."),
			 &html_form_Middle PROGMEM = PSTR("\" method=\"post\" id=\"form\">"),
			 &html_form_Middle_Upload PROGMEM = PSTR("\" method=\"post\" enctype=\"multipart/form-data\" id=\"form\">"), 
			 &html_form_End PROGMEM = PSTR("</FORM>"),
			 &table_title_messages PROGMEM = PSTR("System Messages"),
			 &html_paragraph_begin PROGMEM = PSTR("<ul>"),
			 &html_paragraph_end PROGMEM = PSTR("</ul>"),
			 &field_title_alerts PROGMEM = PSTR("System Alerts"),
			 &http_header_connection PROGMEM = PSTR("Connection"),
			 &http_header_close PROGMEM = PSTR("close"),
			 &http_file_bad_args PROGMEM = PSTR("Invalid args"),
			 &http_file_forbidden PROGMEM = PSTR("Forbidden"),
			 &http_file_not_found PROGMEM = PSTR("File Not Found"),
			 &http_file_creation_failed = PSTR("Creation Failed"),
			 &http_file_exists PROGMEM = PSTR("File Exists");
//

//PLC Error Messages
const String &err_failed_creation PROGMEM = PSTR("Failed to create object."),
			 &err_failed_accessor PROGMEM = PSTR("Failed to create accessor."),
			 &err_unknown_args PROGMEM = PSTR("Unknown argument."),
			 &err_insufficient_args PROGMEM = PSTR("Insufficient arguments."),
			 &err_unknown_type PROGMEM = PSTR("Unknown object type."),
			 &err_pin_invalid PROGMEM = PSTR("Invalid pin for IO."),
			 &err_pin_taken PROGMEM = PSTR("IO pin already taken."),
			 &err_unknown_obj PROGMEM = PSTR("Invalid Object"),
			 &err_invalid_bit PROGMEM = PSTR("Invalid Bit"),
			 &err_name_too_long PROGMEM = PSTR("Object Name Too Long"),
			 &err_parser_failed PROGMEM = PSTR("Parser operation failed."),
			 &err_var_type_invalid PROGMEM = PSTR("Invalid variable type."),
			 &err_var_out_of_range PROGMEM = PSTR("Value exceeds variable range."),
			 &err_invalid_function PROGMEM = PSTR("Function given is not supported."),
			 &err_math_too_many_args PROGMEM = PSTR("Too many arguments."),
			 &err_math_too_few_args PROGMEM = PSTR("Too few arguments."),
			 &err_math_division_by_zero PROGMEM = PSTR("Cannot divide by zero.");
//
//Variable string definitions
const String &VAR_INT32 PROGMEM = PSTR("INT32"),
					&VAR_UINT32 PROGMEM = PSTR("UINT32"),
					&VAR_INT64 PROGMEM = PSTR("INT64"),
					&VAR_UINT64 PROGMEM = PSTR("UINT64"),
					&VAR_DOUBLE PROGMEM = PSTR("DOUBLE"),
					&VAR_BOOL PROGMEM = PSTR("BOOL"),
					&VAR_BOOLEAN PROGMEM = PSTR("BOOLEAN");


vector<String> splitString( const String &str, const vector<char> &c, const vector<char> &start_limiters, const vector<char> &end_limiters, bool removeChar )
{
    vector<String> pVector;
    String temp;
	int8_t limited = 0;
    for (uint16_t x = 0; x < str.length(); x++)
    {
		for (uint8_t y = 0; y < start_limiters.size(); y++ ) //search through our list of query limiter chars
        {
            if (str[x] == start_limiters[y])
				limited++;
        }
		for (uint8_t y = 0; y < end_limiters.size(); y++ ) //search through our list of query limiter chars
        {
            if (str[x] == end_limiters[y])
				limited--;
        }

		bool end = false;
		if ( limited == 0 )
		{
			for (uint8_t y = 0; y < c.size(); y++ ) //searching through our list of terminating chars
			{
				if (str[x] == c[y] )
				{
					if (!removeChar)
					{
						if ( temp.length() )
						{
							end = true; //only end if we have some chars stored in the string.
							temp += str[x];
						}
					}
					else
						end = true; //one of the chars matched
					break; //just end the loop here
				}
			}
		}

		if(x >= (str.length() - 1) && !end) //must also append anything at the end of the string (not including terminating char)
		{
			end = true; 
			temp += str[x]; //append
		}

		if (end) //must have some length before being added to the vector. 
		{
			if ( temp.length() )
			{
				pVector.emplace_back(temp);
				temp.clear(); //empty
			}
		}
		else
        	temp += str[x]; //append
    }

    return pVector;
}

vector<String> splitString( const String &str, const vector<char> &splitChar, bool removeChar, const char lim_begin, const char lim_end )
{ 
	return splitString(str, splitChar, vector<char>{lim_begin}, vector<char>{lim_end}, removeChar ); 
}
vector<String> splitString( const String &str, const char splitChar, bool removeChar, const char lim_begin, const char lim_end )
{ 
	return splitString (str, vector<char>{splitChar}, removeChar, lim_begin, lim_end ); 
}

multimap<int8_t, String> textWithin(const String &str, char begin, char end, int8_t depth ) //must be a multimap because we can have multiple subtiers in the same nest tier
{
	uint8_t maxTiers = 0;
	if ( depth < 0)
	{
		for ( uint16_t x = 0; x < str.length(); x++ )
		{
			if ( str[x] == begin )
				maxTiers++;
		}
	}

	multimap<int8_t, String> pMap;
	int8_t iOperatorTier = 0; //default tier
	String temp;
	for ( int8_t iTier = 0; iTier <= maxTiers; iTier++ ) 
	{
		for (uint16_t x = 0; x < str.length(); x++)
		{
			if (str[x] == begin) //have we reached one of our terminating chars?
			{
				iOperatorTier++; //jump up one tier
				if ( (depth < 0 && iTier == iOperatorTier) )
				{
					if ( iTier && temp.length() )
					{
						pMap.emplace(iTier,temp);
						temp.clear();
					}
					continue;
				}
				else if ( depth && depth == iOperatorTier )
					continue;
			}
			else if (str[x] == end)
			{
				iOperatorTier--; //shift down a tier
				if ( ( depth < 0 && iTier == iOperatorTier ) )
				{
					if ( iTier && temp.length() )
					{
						pMap.emplace(iTier,temp);
						temp.clear();
					}
					continue;
				}
			}

			if ( ( depth < 0 && iOperatorTier == iTier ) || ( depth > 0 && iOperatorTier >= depth ) )
				temp += str[x];
		}
		if ( temp.length() )
		{
			if ( depth < 0)
				pMap.emplace(iTier,temp);
			else
				pMap.emplace(0,temp);

			temp.clear();
		}
		else break; //looks like we're done, just end here
	}
    
    return pMap;
}

int64_t parseInt( const String &str )
{
	String tempstr;
	for ( uint8_t x = 0; x < str.length(); x++ )
	{
		char tempChar = str.charAt(x);
		
		if ( (tempChar > 47 && tempChar < 58) || tempChar == 45 )//only add number chars to the string, also the negative sign
		tempstr.concat( tempChar );
	}
	
    return atoll(tempstr.c_str());
	//return tempstr.toInt(); //Will return 0 if buffer does not contain data. (safe)
}

float parseFloat( const String &str )
{
	String tempstr;
	for ( uint_fast32_t x = 0; x < str.length(); x++ )
	{
		char tempChar = str.charAt(x);
		
		if ( (tempChar > 47 && tempChar < 58) || tempChar == 45 || tempChar == 46 )//only add number chars to the string, also the negative sign and period
			tempstr.concat( tempChar );
	}
	
	return tempstr.toFloat(); //Will return 0 if buffer does not contain data. (safe)
}

//taken from 'stdlib_noniso.c'
void reverse(char* begin, char* end) {
    char *is = begin;
    char *ie = end - 1;
    while(is < ie) {
        char tmp = *ie;
        *ie = *is;
        *is = tmp;
        ++is;
        --ie;
    }
}

DATA_TYPE strDataType( const String &str )
{
	if ( str.length() > 18 )
		return DATA_TYPE::TYPE_STRING; //can't have this many numbers in a valid numeric value 

	uint8_t numDots = 0;
	for (uint16_t x = 0; x < str.length(); x++ )
	{
		if ( str[x] == '.')
			numDots++;
		if ( str[x] > 57 || ( (str[x] < 48 ) && (str[x] != '.') && (str[x] != '-' && x == 0) ) || numDots > 1 )
			return DATA_TYPE::TYPE_STRING; //string type
	}

	if ( numDots )
		return DATA_TYPE::TYPE_DOUBLE; //double type
	
	return DATA_TYPE::TYPE_INT; //integer type
}

bool strContains( const String &str, const vector<char> &c )
{
	for ( uint16_t x = 0; x < str.length(); x++ )
	{
		for (uint8_t y = 0; y < c.size(); y++ )
		{
			if ( str[x] == c[y] )
				return true;
		}
	}
	return false;
}
bool strContains( const String &str, const char c ){ return strContains(str, vector<char>{c}); }

bool strBeginsWith( const String &str, const vector<String> &s)
{
    for ( uint8_t y = 0; y < s.size(); y++ )
    {
        if ( (bool)str.startsWith(s[y]) )
            return true;
    }

    return false;
}
bool strBeginsWith( const String &str, const String &s ){ return strBeginsWith( str, vector<String>{ s } ); }

bool strBeginsWith( const String &str, const vector<char> &c )
{
	for (uint8_t y = 0; y < c.size(); y++ )
	{
		if ( *str.begin() == c[y] )
			return true;
	}
	return false;
}
bool strBeginsWith( const String &str, const char c ){ return strBeginsWith(str, vector<char>{c}); }

bool strEndsWith( const String &str, const vector<String> &s)
{
    for ( uint8_t y = 0; y < s.size(); y++ )
    {
        if ( (bool)str.endsWith(s[y]) )
            return true;
    }

    return false;
}
bool strEndsWith( const String &str, const String &s ){ return strEndsWith( str, vector<String>{ s } ); }

bool strEndsWith( const String &str, const vector<char> &c )
{
	for (uint8_t y = 0; y < c.size(); y++ )
	{
		if ( *str.end() == c[y] )
			return true;
	}
	return false;
}
bool strEndsWith( const String &str, const char c ){ return strEndsWith(str, vector<char>{c}); }

String removeFromStr( const String &str, const vector<char> &c )
{
	String output;
	bool skipChar = false;

	for ( uint16_t x = 0; x < str.length(); x++ )
	{
		skipChar = false;
		for ( uint8_t y = 0; y < c.size(); y++ )
		{
			if ( c[y] == str[x] )
			{
				skipChar = true;
				break; //no need to look further this cycle.
			}
		}

		if ( !skipChar )
			output += str[x]; //append the char
	}

	return output;
}

String removeFromStr( const String &str, const char c ){ return removeFromStr( str, vector<char>{c} ); }

OBJ_TYPE findMathObjectType( const String &str )
{
	if( str == typeTagMACOS ) return OBJ_TYPE::TYPE_MATH_ACOS;
	else if( str == typeTagMADD ) return OBJ_TYPE::TYPE_MATH_ADD;
	else if( str == typeTagMASIN ) return OBJ_TYPE::TYPE_MATH_ASIN;
	else if( str == typeTagMATAN ) return OBJ_TYPE::TYPE_MATH_ATAN;
	else if ( str == typeTagMCOS ) return OBJ_TYPE::TYPE_MATH_COS;
	else if ( str == typeTagMDEC ) return OBJ_TYPE::TYPE_MATH_DEC;
	else if ( str == typeTagMDIV ) return OBJ_TYPE::TYPE_MATH_DIV;
	else if ( str == typeTagMEQ ) return OBJ_TYPE::TYPE_MATH_EQ;
	else if ( str == typeTagMGRE ) return OBJ_TYPE::TYPE_MATH_GRT;
	else if ( str == typeTagMGREE ) return OBJ_TYPE::TYPE_MATH_GRQ;
	else if ( str == typeTagMINC ) return OBJ_TYPE::TYPE_MATH_INC;
	else if ( str == typeTagMLES ) return OBJ_TYPE::TYPE_MATH_LES;
	else if ( str == typeTagMLESE ) return OBJ_TYPE::TYPE_MATH_LEQ;
	else if ( str == typeTagMMOV ) return OBJ_TYPE::TYPE_MATH_MOV;
	else if ( str == typeTagMMUL ) return OBJ_TYPE::TYPE_MATH_MUL;
	else if ( str == typeTagMSUB ) return OBJ_TYPE::TYPE_MATH_SUB;
	else if ( str == typeTagMSIN ) return OBJ_TYPE::TYPE_MATH_SIN;
	else if ( str == typeTagMNEQ ) return OBJ_TYPE::TYPE_MATH_NEQ;

	return OBJ_TYPE::TYPE_INVALID; //could not find the object
}

uint64_t generateBitMask(const uint8_t numBits, const uint8_t offset)
{
	if (numBits > 63)
		return 0;

	uint64_t mask = 1;

	for (uint8_t x = 1; x < numBits; x++)
		mask |= (1 << x); 

	mask = (mask << offset);

	return mask;
}

uint32_t unpackData(const uint8_t numBits, const uint8_t offset, const uint64_t *packedBits)
{
	uint64_t data = (generateBitMask(numBits, offset) & *packedBits);
	data = (data >> offset);

	return (uint32_t)data;
}

uint32_t unpackData(const uint64_t mask, const uint8_t offset, const uint64_t *packedBits)
{
	uint64_t data = (mask & *packedBits);
	data = (data >> offset);

	return (uint32_t)data;
}
