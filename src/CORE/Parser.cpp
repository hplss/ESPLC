/*
 * parser.cpp
 *
 * Created: 2/1/2018 2:30:31 PM
 *  Author: Andrew
 * TODO: Reduce usage of String objects in favor of C-Style char arrays (memory safety).
 */ 

#include "Arduino.h" //Used for serial.
#include "UICore.h"
#include <WiFi.h>

void UICore::parseSerialData()
{	
	char buffer[MAX_BUFFERSIZE] = { NULL_CHAR };
		
	if ( Serial.available() ) //Have something in the serial buffer?
	{
		Serial.readBytesUntil( NULL_CHAR, buffer, MAX_BUFFERSIZE - 1 ); //read into the buffer.
		uint8_t length = strlen(buffer);
		for ( int pos = 0; pos < length ; pos++ )
		{
			if ( buffer[pos] == NULL_CHAR )
				break; //Assume that a null char is the end of the buffer.
				
			if ( buffer[pos] == CMD_PREFIX ) //Found our command indicator in the buffer
			{
				pos++; //increment our pos +1
				if ( pos >= length || buffer[pos] == NULL_CHAR )
					break; //Safety first. End here
					
				switch ( buffer[pos] )
				{
					case CMD_CONNECT: //For connecting to a wifi router ** Should be able to connect by SSID index as well. 
						parseConnect( parseArgs( pos, length, buffer ) );
						break;
					case CMD_PROGRAM:
						parseCfg( parseArgs( pos, length, buffer ) );
						break;
					case CMD_DISCONNECT: //Disconnect from current wifi network
						closeConnection();
						break;
					case CMD_NETWORKS: //For listing all available networks
						scanNetworks();
						break;
					case CMD_AP: //Telling the ESP to become an access point "name":"password"
						parseAccessPoint( parseArgs( pos, length, buffer ) );
						break;
					case CMD_VERBOSE: //Enable/disable verbose mode
						parseVerbose( parseArgs( pos, length, buffer ) );
						break;
					case CMD_NETINFO: //Status
						printDiag();
						break;
					case CMD_TIME:
						parseTime( parseArgs( pos, length, buffer ) );
						break;
					default:
						continue; //Nothing here? just skip it.
				}
				
			}
		}
	}
}

vector<String> UICore::parseArgs( int &pos, const uint8_t len, const char buffer[] ) //Here is where we split our buffer up into a series of strings, we'll read until the next CMD_PREFIX char.
{
	vector<String> args;
	bool quoteBegin = false;
	String tempStr;
	
	for ( pos++; pos < len; pos++ ) //init position +1
	{
		if ( buffer[pos] == CMD_PREFIX || buffer[pos] == NULL_CHAR || pos >= len || buffer[pos] == CHAR_CARRIAGE || buffer[pos] == CHAR_NEWLINE ) //Start of a new command or end of the buffer.
		{
			if ( buffer[pos] == CMD_PREFIX )
				pos--; //Jump back a step so that other command can be interpreted in parseSerialData

			if ( tempStr.length() )
				args.push_back( tempStr ); 
			return args; //Exit the function, and push our arguments.
		}
				
		if ( buffer[pos] == '"' ) //Check for the beginning of quotes to prohibit further filtering. 
		{ 
			quoteBegin = !quoteBegin; //Toggle
			continue; //Skip this char and move on
		}
		
		if ( !quoteBegin ) //Assuming we're not in quotes here.
		{
			if ( ( buffer[pos] <= 32 || buffer[pos] > 126 ) ) //Filter all non-printable ascii chars and whitespace if we're not between the quotes
				continue; //Skip
						
			if ( buffer[pos] == DATA_SPLIT  ) //DATA_SPLIT can be a valid char if in quotes.
			{
				args.push_back( tempStr ); //We've reached the splitter char for multiple data streams. 
				tempStr.clear(); //Clear the string.
				continue; //Skip this char
			}	
		}

		tempStr += buffer[pos]; //Add the char to our string if we've come this far.
	}
	
	return args;
}

bool UICore::parseAccessPoint( const vector<String> &args )
{
	//<SSID>:<Password>
	if ( args.size() >= 2 ) //Make sure they exist, prevent crashing
	{
		Serial.println(args[1]);
		return setupAccessPoint( args[0], args[1] ); //Any other args will be discarded (not used)
	}
	else
		sendMessage( PSTR("Not enough arguments to initialize access point."), PRIORITY_HIGH );
		return false;
}

void UICore::parseConnect( const vector<String> &args )
{
	//<SSID>:<Password>:<special mode>
	if ( args.size() >= 2 ) //Minimum args.
	{
		if ( args.size() >= 3 ) //More than 2 args?
		{
			if ( args[2] == "#" )//Connect by index option. This only works if a scan has already been performed.
			{
				unsigned int ssidIndex = parseInt(args[0]);
				int8_t indexes = WiFi.scanNetworks();
				if ( !ssidIndex || indexes <= 0 || ssidIndex > indexes )
				{
					sendMessage( PSTR("Invalid SSID index: ") + String(ssidIndex), PRIORITY_HIGH );
					return;
				}
				beginConnection( WiFi.SSID(ssidIndex), args[1] );
				return;
			}
			else if ( args[2] == "*" ) //Wild-card connection option.
			{
				//WiFi.scanNetworks(); //Build a list of all available networks first.
				int8_t numResults = WiFi.scanNetworks(); //We need to build an index for all available nearby networks first
				if ( numResults > 0 ) //It's possible to have negative numbers here (error codes)
				{
					for ( uint8_t i = 0; i < numResults; i++ )
					{
						if ( !strncmp( args[0].c_str(), WiFi.SSID(i).c_str(), args[0].length() ) ) //Returns 0 if strings are equal
						{
							beginConnection( WiFi.SSID(i), args[1] );
							return;
						}
					}
					sendMessage( F("No valid SSID match found using wild-card."), PRIORITY_HIGH );
					return;
				}
				else
					sendMessage( F("Scan returned no available networks, cannot connect via wild-card."), PRIORITY_HIGH  );
				
				return; //if we've made it this far, we're probably going nowhere. just end.
			}	
		}
		 
		beginConnection( args[0], args[1] ); //Normal connection method
	}
	else
		sendMessage( F("Not enough arguments to connect to network."), PRIORITY_HIGH );
}

void UICore::parseVerbose( const vector<String> &args ) //deprecated?
{
	if ( args.size() ) //Make sure we've got some data.
	{ 
		uint8_t value = parseInt( args[0] ); //See if we have a numeric value 
		if ( value )
		{
			if ( value > VERBOSE_MAX )
				value = VERBOSE_MAX; //cap
				
			i_verboseMode = value;
		}
		else if ( args[0] == "on" )// specified "on" - just default to show all messages
			i_verboseMode = PRIORITY_LOW;
		else if ( !value || args[0] == "off" )
			i_verboseMode = 0;
	}
}

long parseInt( const String &str )
{
	String tempstr;
	for ( uint_fast32_t x = 0; x < str.length(); x++ )
	{
		char tempChar = str.charAt(x);
		
		if ( (tempChar > 47 && tempChar < 58) || tempChar == 45 )//only add number chars to the string, also the negative sign
		tempstr.concat( tempChar );
	}
	
	return tempstr.toInt(); //Will return 0 if buffer does not contain data. (safe)
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

vector<String> splitString( const String &str, const char split )
{
	vector<String> args;
	String temp;
	for ( unsigned int x = 0; x < str.length(); x++ )
	{
		if ( str[x] == split )
		{
			args.push_back(temp);
			temp.clear();
		}
		else
			temp += str[x];
	}

	args.push_back(temp); //push back anything left
	return args;
}

void UICore::parseCfg( const vector<String> &args )
{
	generateSettingsMap();
	if ( args.size() <= 0 )
	{
		for ( settings_itr = settingsMap.begin(); settings_itr != settingsMap.end(); settings_itr++ )
			sendMessage( settings_itr->first + CHAR_EQUALS + settings_itr->second.get()->getSettingValue(), PRIORITY_HIGH);
	}
	else
	{
		for ( uint8_t x = 0; x < args.size(); x++ )
		{
			vector<String> splitArgs = splitString(args[x], CHAR_EQUALS);
			if (splitArgs.size() == 2) //can only have 2 args
			{
				for ( settings_itr = settingsMap.begin(); settings_itr != settingsMap.end(); settings_itr++ )
				{
					if ( settings_itr->first == splitArgs[0] ) //search for the specific setting string identifier
					{
						settings_itr->second.get()->setSettingValue(splitArgs[1]);
						sendMessage(PSTR("Applying setting to ") + splitArgs[0], PRIORITY_HIGH);
					}
				}
			}
			else if (splitArgs[0] == PSTR("save"))
				saveSettings(); //write the current settings to the config file
			else
				sendMessage(PSTR("Wrong number of arguments for setting."), PRIORITY_HIGH);
		}
	}
	
	clearSettingsMap(); //empty the map here
}

void UICore::parseTime( const vector<String> &args )
{
	uint8_t totalArgs = args.size();
		
	if ( totalArgs )
	{
		for ( uint8_t x = 0; x < totalArgs; x++ )
		{
			if ( args[x] == "n" && totalArgs >= (x+1) ) //enable/disable
			{
				x++; //Move to next element.
				uint8_t value = parseInt( args[x] );
				if ( value )
					b_enableNIST = true;
				else
					b_enableNIST = false;
			}
			else if ( args[x] == "u" ) //forced update
			{
				if ( !UpdateNIST(true) )
					sendMessage(F("Failed to update NIST time."));
			}
			else if ( args[x] == "f" && totalArgs >= (x+1) ) //frequency
			{
				x++; //Move to next element.
				i_NISTupdateFreq = parseInt( args[x] );
			}
			else if ( args[x] == "s" && totalArgs >= (x+1) ) //nist server name/ip
			{
				x++; //Move to next element.
				getNISTServer() = args[x]; //FIX FIX
			}
			else if ( args[x] == "t" && totalArgs >= (x+6) ) //manual time entry (requires year, month, day, hour, min, and sec)
			{
				p_currentTime->SetTime(parseInt(args[x + 1]), parseInt(args[x + 2]), parseInt(args[x + 3]), parseInt(args[x + 4]), parseInt(args[x + 5]), parseInt(args[x + 6]) );
				x += 6; //advance
			}
			else if ( args[x] == "z" && totalArgs >= (x+1) ) //zone
			{
				x++;
				p_currentTime->SetTimeZone(parseInt( args[x] ));
			}
			else
				sendMessage( p_currentTime->GetTimeStr(), PRIORITY_HIGH );
		}
	}	
}

