/*
 * UICore.cpp
 *
 * Created: 10/12/2019 5:58:26 PM
 *  Author: Andrew
 * This is the core of the UICore class, which handles all user inputs to the PLC device.
 */ 

#include "UICore.h"
#include <map>
#include <memory>
#include <ESPmDNS.h>

//WiFiClient client; //used for transferring data over TCP/UDP (not necessarily http)

const String &HTML_HEADER_INITIAL PROGMEM = PSTR(
"<!DOCTYPE HTML>"
"<html>"
"<head>"
"<meta name = \"viewport\" content = \"width = device-width, initial-scale = 1.0, maximum-scale = 1.0, user-scalable=0\">"
"<style>"),
	&HTML_HEADER_LAST PROGMEM = PSTR( 
"</style>"
"</head>"
"<body>"),

	&HTML_FOOTER PROGMEM = PSTR(
"</body>"
"</html>");

void UICore::setup()
{
	WiFi.mode(WIFI_AP_STA); 
	WiFi.enableAP(false); //off by default.
	WiFi.persistent(false);
	b_FSOpen = false; //init to false;
	b_SaveScript = false; //default to off
	b_SaveConfig = false; 
	if ( !SPIFFS.begin(true) ) //Format on fail = true.
		sendMessage(PSTR("Failed to initialize SPIFFS storage system."),PRIORITY_HIGH);
	else
		b_FSOpen = true; //set true if begin works

	//Time stuff
	p_UDP = make_shared<WiFiUDP>( WiFiUDP() );
	getTimeUDP().begin(123); //Open port 123 for NTP packet
	p_currentTime = new Time;
	p_nextNISTUpdateTime = new Time;

	//Init our objects/configs here.
	setupServer(); //Set up the web hosting directories.

	//Default stuff for now, until EEPROM loading is implemented (also initializes shared pointers -- VERY IMPORTANT)
	i_verboseMode = PRIORITY_LOW; //Do this by default for now, will probably have an eeprom setting for this later.
	i_timeoutLimit = 15; //20 second default, will probably be an eeprom config later
	b_enableNIST = true;
	b_enableAP = false;
	b_enableDNS = false;
	i_NISTupdateFreq = 5; //default to 5 minutes for now.
	i_NISTUpdateUnit = TIME_MINUTE; //default for now
	s_NISTServer = make_shared<String>(PSTR("time.nist.gov")); //Default for now.
	s_WiFiPWD = make_shared<String>();
	s_WiFiSSID = make_shared<String>();
	s_uniqueID = make_shared<String>(PSTR("DEFID")); //Default for now
	i_NISTPort = 13; //default
	i_nistMode = 1; //NTP
	s_StyleSheet = make_shared<String>(loadWebStylesheet());
	s_DNSHostname = make_shared<String>();
	s_WiFiHostname = make_shared<String>();
	//
	
	applySettings( true ); //apply settings that are loaded from the flash storage - should be done before creating the data fields for web UI
}

//This is the main UI process function. Responsible for handling all updates to UI objects.
void UICore::Process()
{
	parseSerialData(); //parse all incoming serial data.
	if ( WiFi.status() == WL_CONNECTED || WiFi.softAPgetStationNum() ) //Only do this stuff if we're connected to a network, or a client has connected to the AP
	{
		getWebServer().handleClient(); //Process stuff for clients that have connected.
	}
	
	updateClock(); //Update our stored system clock values;
}

bool UICore::handleAuthorization()
{
	if ( getLoginName().length() > 1 || getLoginPWD().length() > 1 )
	{
		if (!getWebServer().authenticate(getLoginName().c_str(), getLoginPWD().c_str()) )
			getWebServer().requestAuthentication(DIGEST_AUTH, String(getUniqueID() + PSTR(" Login")).c_str(), PSTR("Authentication Failed.") );
	}

	return true;
}

void UICore::applySettings( bool loadFromFile )
{
	if ( loadFromFile )
	{
		loadSettings(); //load the saved settings from flash memory and apply them to our settings variables.
	}
	if ( b_enableAP ) //set after load?
	{
		wifi_config_t conf;
    	esp_wifi_get_config(WIFI_IF_AP, &conf);
		if ( WiFi.isConnected() ) //only works for station connections? Hmm
			closeConnection(); //force the connection to close if open already.. somehow?
		
		Serial.println("should be starting the AP");
		if ( String((char*)conf.ap.ssid) != getWiFiSSID() || String((char *)conf.ap.password) != getWiFiPWD() ) //only update if ssid or pwd is different
		{
			WiFi.softAPdisconnect();// close the existing AP if it is already open
			
			b_enableAP = setupAccessPoint(getWiFiSSID(), getWiFiPWD()); //verify proper setup of AP
		}
	}
	else if ( !b_enableAP && getWiFiSSID().length() ) //connect to network instead?
	{
		wifi_config_t conf;
    	esp_wifi_get_config(WIFI_IF_STA, &conf);

		if ( String((const char *)conf.sta.ssid) != getWiFiSSID() || String((const char *)conf.sta.password) != getWiFiPWD() ) //only update is ssid is different
		{
			if ( WiFi.isConnected() ) //only works for station connections? Hmm
				closeConnection(); //force the connection to close if open already.. somehow?

			beginConnection(getWiFiSSID(), getWiFiPWD());
		}
	}
	
	if ( b_enableDNS && getDNSHostname().length() > 1 ) //must have a valid hostname
	{
		b_enableDNS = MDNS.begin( getDNSHostname().c_str() );
		if ( b_enableDNS ) //verify
			MDNS.addService("http", "tcp", 80);
	}
	else
		MDNS.end(); //Close just to make sure (free resources).
}

//This function creates the access point if a network is unavailable for us to connect to. 
bool UICore::setupAccessPoint( const String &ssid, const String &password )
{
	if ( WiFi.isConnected() )
	{
		sendMessage( PSTR("Connected to '") + WiFi.SSID() + PSTR("'. Disconnect before establishing an access point."), PRIORITY_HIGH );
		return false;
	}
	
	if (WiFi.enableAP(true)) //Allow for access point mode.
	{
		IPAddress Ip(192, 168, 1, 1); 
		IPAddress NMask(255, 255, 255, 0); 
		
		WiFi.softAPConfig(Ip, Ip, NMask);
		if ( (password.length() > 1) ? WiFi.softAP(ssid.c_str(), password.c_str()) : WiFi.softAP(ssid.c_str()) )  //Set up the access point with our password and SSID name.
		{
			WiFi.softAPsetHostname(String(getWiFiHostname() + String(DATA_SPLIT) + getUniqueID()).c_str()); //set the host name
			sendMessage( PSTR("Opening access point with SSID: ") + ssid + PSTR(" using password: ") + password );
			IPAddress myIP = WiFi.softAPIP();
			sendMessage( PSTR("IP address: ") + myIP.toString() );
			sendMessage( PSTR("Hostname: ") + String(WiFi.softAPgetHostname()) );
			p_server->begin();//Start up page server.
			return true;
		}
	}
	
	sendMessage(F("Failed to start access point."), PRIORITY_HIGH);
	WiFi.enableAP(false);
	return false;
}

void UICore::closeConnection( bool msg )
{
	if ( msg ) //Sometimes this function will be called from within the code and not explicitly by the user.
	{
		if ( WiFi.isConnected() )
			sendMessage( PSTR("Closing connection to ") + WiFi.SSID() );
		else
		{
			
		}
		
	}
	WiFi.setAutoReconnect( false ); //No reconnections
	WiFi.softAPdisconnect( true ); //Close the AP, if open.
	WiFi.enableAP(false);
	getWebServer().close(); //Stop the web server.
	WiFi.disconnect(); //Disconnect the wifi
	//WiFi.mode(WIFI_OFF);
}

void UICore::setupServer()
{
	p_server = make_shared<WebServer>(80); //Open on port 80 (http)
	//These set up our page triggers, linking them to specific functions.
	getWebServer().on(styleDir, std::bind(&UICore::handleStyleSheet, this) );
	getWebServer().on(PSTR("/"), std::bind(&UICore::HandleIndex, this) );
	getWebServer().on(adminDir, std::bind(&UICore::HandleAdmin, this) );
	getWebServer().on(scriptDir, std::bind(&UICore::HandleScript, this) );
	//
};

void UICore::scanNetworks()
{
	/*if ( WiFi.isConnected() )
	{
		sendMessage( "Connected to '" + WiFi.SSID() + "'. Disconnect before scanning for networks.", true );
		return;
	}*/
		
	//WiFi.mode(WIFI_AP_STA);
	//closeConnection( false ); //Just in case
	//delay(100);
	int8_t n = WiFi.scanNetworks(); //More than 255 networks in an area? Possible I suppose.
	if (!n)
		sendMessage( F("No networks found" ) );
	else
	{
		sendMessage( String(n) + F(" networks found.") );
		for (int i = 0; i < n; ++i)
		{
			// Print SSID and RSSI for each network found
			sendMessage( String( i ) + ": " + WiFi.SSID(i) + " (" +  WiFi.RSSI(i) + ")" + ( ( WiFi.encryptionType(i) == 7/*ENC_TYPE_NONE*/ )?" ":"*" ), true );
			delay(10);
		}
	}
	//WiFi.mode(WIFI_OFF);
}

bool UICore::beginConnection( const String &ssid, const String &password )
{
	if ( WiFi.isConnected() )
	{
		sendMessage( PSTR("Connected to '") + WiFi.SSID() + PSTR("'. Disconnect before attempting a new connection."), PRIORITY_HIGH );
		return false;
	}
	
	sendMessage( PSTR("Attempting connection to: ") + ssid );
	//WiFi.mode(WIFI_AP_STA);
	
	if ( WiFi.begin( ssid.c_str(), password.c_str() ) == WL_CONNECT_FAILED )
	{
		sendMessage( PSTR("Failed to begin Wifi connection (Invalid password or SSID?)") , true );
		closeConnection();//Just in case
		return false;
	}
	else 
	{
		uint8_t i_retries = 0;
		while ( WiFi.status() != WL_CONNECTED && i_retries < i_timeoutLimit ) //We'll give it 10 seconds to try to connect?
		{
			delay(1000); //THIS COULD CAUSE PROBLEMS FOR THE PLC LOGIC -- FIX THIS
			i_retries++;
		}
		if ( WiFi.isConnected() )
		{
			WiFi.setHostname(String(getWiFiHostname() + String(DATA_SPLIT) + getUniqueID()).c_str());
			sendMessage( PSTR("Connected to ") + ssid + PSTR(" with local IP: ") + WiFi.localIP().toString() + PSTR(" Hostname: ") + WiFi.getHostname() );
			WiFi.setAutoReconnect(b_autoRetryConnection);
			p_server->begin(); //start the server
		}
		else 
		{
			sendMessage( PSTR("Connection to ") + ssid + PSTR(" timed out."), PRIORITY_HIGH ); 
			closeConnection( false );
			return false;
		}
	}
	return true;
}

void UICore::sendMessage( const String &str, uint8_t priority )
{
	if ( i_verboseMode >= priority ) 
		Serial.println( str );
	else 
		return;
}

void UICore::printDiag()
{
	String stat; 
	switch( WiFi.status() )
	{
		case WL_CONNECTED:
			stat = F("Connected");
			break;
		case WL_DISCONNECTED:
			stat = F("Disconnected");
			break;
		case WL_CONNECTION_LOST:
			stat = F("Connection lost");
			break;
		case WL_IDLE_STATUS:
			stat = F("Idle");
			break;
		default:
			stat = String( WiFi.status() );
	}
	sendMessage( PSTR("-- Station Settings --"), PRIORITY_HIGH);
	sendMessage( PSTR("Network Status: ") + stat, PRIORITY_HIGH );
	sendMessage( PSTR("IP: ") + WiFi.localIP().toString(), PRIORITY_HIGH );
	sendMessage( PSTR("Gateway: ") + WiFi.gatewayIP().toString(), PRIORITY_HIGH );
	sendMessage( PSTR("Subnet: ") + WiFi.subnetMask().toString(), PRIORITY_HIGH );
	sendMessage( PSTR("MAC: ") + WiFi.macAddress(), PRIORITY_HIGH );
	sendMessage( PSTR("Hostname: ") + String(WiFi.getHostname()), PRIORITY_HIGH );
	sendMessage( PSTR("\n-- Access Point Settings --"), PRIORITY_HIGH);
	sendMessage( PSTR("Hostname: ") + String(WiFi.softAPgetHostname()), PRIORITY_HIGH);
	//sendMessage( PSTR("\n -- DNS Settings --"), PRIORITY_HIGH );
	//sendMessage( PSTR("Hostname: ") + MDNS.hostname() );
	//sendMessage( PSTR("Port: ") + MDNS.port() );
	//wifi_config_t conf;
    //esp_wifi_get_config(WIFI_IF_STA, &conf);
	
	sendMessage( PSTR("\n-- System Time Settings --"), PRIORITY_HIGH);
	sendMessage( PSTR("Time Server Address: ") + getNISTServer(), PRIORITY_HIGH );
	sendMessage( PSTR("Time Update Interval (mins): ") + String(i_NISTupdateFreq) );
	sendMessage( PSTR("NIST Time Mode: ") + String(b_enableNIST) );
	
	sendMessage( PSTR("Available system memory: ") + String(esp_get_free_heap_size()) + PSTR(" bytes."), PRIORITY_HIGH );
	if ( b_FSOpen )
	{
		sendMessage( PSTR("Total flash storage used: ") + String(SPIFFS.usedBytes()) + PSTR(" bytes."), PRIORITY_HIGH );
		sendMessage( PSTR("Total storage available: ") + String(SPIFFS.totalBytes()) + PSTR(" bytes."), PRIORITY_HIGH );
	}
}

void UICore::updateClock()
{
	if ( UpdateNIST() ) //if we've updated the time, advance it next round.
		return;
		
	p_currentTime->UpdateTime();
}

bool UICore::UpdateNIST( bool force ) 
{
	uint8_t retries = 0;
	
	if ( WiFi.isConnected() && b_enableNIST && getNISTServer().length() ) //Must be on a network before attempting to connect to NIST server
	{
		if ( !i_NISTupdateFreq && !force ) //must be a non-zero value
			return false;
		
		if ( p_currentTime->IsBehind( p_nextNISTUpdateTime ) && !force ) //too soon for an update?
			return false;
		
		sendMessage( F("Updating time." ) );
		
		if ( !i_nistMode )
		{
			WiFiClient NISTclient;
			while( !NISTclient.connect(getNISTServer().c_str(), i_NISTPort) )
			{
				if ( retries >= 5 )
				{
					sendMessage(PSTR("Connection to NIST server: '") + *s_NISTServer.get() + "' failed.", PRIORITY_HIGH );
					return false;
				}
				retries++;
				delay(100); //small delay to allow the server to respond
			}
		
			delay(100); //small delay to allow the server to respond
			
			while( NISTclient.available() )
			{
				String line = NISTclient.readStringUntil('\r'); //DAYTIME protocol - meh, it works.
				if ( line.length() < 24 )
					return false; //to be safe
					
				//Break the string down into its components, then save.
				if ( !p_currentTime->SetTime( line.substring(7, 9).toInt(), line.substring(10, 12).toInt(), line.substring(13, 15).toInt(),
				 line.substring(16, 18).toInt(), line.substring(19, 21).toInt(), line.substring(22, 24).toInt() ) )
					return false;
			}
		}
		else if ( i_nistMode == 1 ) //NTP mode
		{
			uint8_t NTP_PACKET_SIZE = 48;
			byte NTPBuffer[NTP_PACKET_SIZE]; // buffer to hold incoming and outgoing packets
			memset(NTPBuffer, 0, NTP_PACKET_SIZE);  // set all bytes in the buffer to 0
			// Initialize values needed to form NTP request
			NTPBuffer[0] = 0b11100011;   // LI, Version, Mode
			// send a packet requesting a timestamp:
			p_UDP->beginPacket(getNISTServer().c_str(), 123); // NTP requests are to port 123
			p_UDP->write(NTPBuffer, NTP_PACKET_SIZE); 
			p_UDP->endPacket();
			
			delay(100);
			while ( !p_UDP->parsePacket()  ) //Have we received anything?
			{
				if ( retries >= 5 )
				{
					sendMessage(PSTR("No response from NIST server: ") + getNISTServer(), PRIORITY_HIGH );
					return false;
				}
				delay(500); // wait a bit
				retries++;
			}
			
			p_UDP->read(NTPBuffer, NTP_PACKET_SIZE); // read the packet into the buffer
			uint32_t NTPTime = (NTPBuffer[40] << 24) | (NTPBuffer[41] << 16) | (NTPBuffer[42] << 8) | NTPBuffer[43]; // Combine the 4 timestamp bytes into one 32-bit number
			// Convert NTP time to a UNIX timestamp:
			p_currentTime->SetNTPTime( NTPTime - 2208988800UL );
		}
		
		p_nextNISTUpdateTime->SetTime( p_currentTime ); //Replace with current time
		p_nextNISTUpdateTime->IncrementTime( i_NISTupdateFreq, i_NISTUpdateUnit );//Then increment -- need to rework this a bit
		
		return true; //End here, we'll let the system clock carry on on the next second.
	}
	return false; //default path
}

void UICore::UpdateWebFields( const vector<shared_ptr<DataTable>>tables, String &HTML )
{
	std::map<shared_ptr<DataField>, String> functionFields;
	//This bit of code handles all of the Datafield value updating, depending on the args that were received from the POST method.
	for ( uint8_t i = 0; i < tables.size(); i++ ) //Go through each setting.
	{
		shared_ptr<DataField> tempField; //Init pointer here
		//HACKHACK - We need to set all checkboxes to "off", they'll be set to on later if applicable. This is due to a limit of the POST method
		for ( uint8_t y = 0; y < tables[i]->GetFields().size(); y++ )
		{
			tempField = tables[i]->GetFields()[y]; //Get the pointer
			
			if ( tempField->GetType() == FIELD_TYPE::CHECKBOX ) //Arg doesn't apply
				tempField->SetFieldValue("");
		}
		//
		//Basically, at this point, we need to sort it so that all data fields that call functions directly are updated last. 
		//The purpose of this is to ensure any modified variables in other fields can be used in function calls.
		//Idea: use some of the code below to build a list of objects that are being modified by args, then build a vector of all that aren't function callers,
		//Then add those to the end.
		
		for ( uint8_t x = 0; x < p_server->args(); x++ ) // For each arg...
		{
			tempField = tables[i]->GetElementByName( p_server->argName(x) );
			if ( tempField ) //Found an element with this name?
			{
				if ( tempField->GetFieldValue() == p_server->arg(x) ) //Is the arg the same as the existing setting?
					continue; //Skip if so
				
				//build a map of function data fields to update last, update all others as they come
				if ( tempField->UsesFunction() ) //All function related fields should go here
				{
					functionFields.emplace(tempField, p_server->arg(x));
				}
				else
				{
					if ( tempField->SetFieldValue( p_server->arg(x) ) )
						HTML += tempField->GetFieldLabel() + " set to: " + p_server->arg(x) + "<br>"; //Inform the admin
					else 
						HTML += PSTR("Update of '") + tempField->GetFieldLabel() + PSTR("' failed. <br>");
				}
			}
		}
	}

	for ( std::map<shared_ptr<DataField>, String>::iterator itr = functionFields.begin(); itr != functionFields.end(); itr++ )//handle each object attached to this object.
	{
		if( itr->first->SetFieldValue( itr->second ) )
			HTML += itr->first->GetFieldLabel() + " set to: " + itr->second + "<br>"; //Inform the admin
		else 
			HTML += PSTR("Update of '") +  itr->first->GetFieldLabel() + PSTR("' failed. <br>");
	}
	//Finally, update the fields as necessary, in the proper order
}

String UICore::generateTitle( const String &data )
{
	return PSTR("<title>Device: ") + getUniqueID() + " " + data + PSTR("</title>");
}

String UICore::generateHeader()
{
	return HTML_HEADER_INITIAL + getStyleSheet() + HTML_HEADER_LAST;
}

String UICore::generateFooter()
{
	return HTML_FOOTER;
}