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


void UICore::setup()
{
	WiFi.mode(WIFI_AP_STA); 
	WiFi.enableAP(false); //off by default.
	WiFi.persistent(false);

	p_UDP = make_shared<WiFiUDP>( WiFiUDP() ); //MUST BE INITIALIZED ONLY AFTER WIFI IS INITIALIZED, LEST YE FACE UNFORSEEN TRIFLES
	p_UDP->begin(123); //Open port 123 for NTP packet
	//Init our objects/configs here.
	setupServer(); //Set up the web hosting directories.
	
	if ( !SPIFFS.begin(true) ) //Format on fail = true.
		sendMessage(PSTR("Failed to initialize SPIFFS storage system."),PRIORITY_HIGH);
	else
	{
		b_FSOpen = true; //set true if begin works
		applySettings( b_FSOpen ); //apply settings that are loaded from the flash storage - should be done before creating the data fields for web UI
		*s_StyleSheet = loadWebStylesheet();
	}
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

		if ( String((char*)conf.ap.ssid) != getWiFiAPSSID() || String((char *)conf.ap.password) != getWiFiAPPWD() || loadFromFile ) //only update if ssid or pwd is different
		{
			WiFi.softAPdisconnect();// close the existing AP if it is already open
			b_enableAP = setupAccessPoint(getWiFiAPSSID(), getWiFiAPPWD()); //verify proper setup of AP
		}
	}
	else
	{
		wifi_config_t conf;
    	esp_wifi_get_config(WIFI_IF_STA, &conf);

		if ( String((const char *)conf.sta.ssid) != getWiFiSSID() || String((const char *)conf.sta.password) != getWiFiPWD() || loadFromFile ) //only update is ssid is different
		{
			if ( WiFi.isConnected() ) //only works for station connections? Hmm
				closeConnection(); //force the connection to close if open already.. somehow?

			if (!beginConnection(getWiFiSSID(), getWiFiPWD()) )
			{
				if ( getWiFiAPSSID().length() )
					b_enableAP = setupAccessPoint(getWiFiAPSSID(), getWiFiAPPWD()); //default to this if connection fails.
				else
					b_enableAP = setupAccessPoint(PSTR("ESPLC"), getWiFiAPPWD());
			}
				
		}
	}
	
	if ( b_enableDNS && getDNSHostname().length() && ( WiFi.isConnected() || b_enableAP ) ) //must have a valid hostname
	{
		b_enableDNS = MDNS.begin( getDNSHostname().c_str() );
		if ( b_enableDNS ) //verify
			MDNS.addService("http", "tcp", 80);
	}
	else
		MDNS.end(); //Close just to make sure (free resources).
}

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
			if ( password.length() )
				sendMessage( PSTR("Opening access point with SSID: ") + ssid + PSTR(" using password: ") + password );
			else
				sendMessage( PSTR("Opening access point with SSID: ") + ssid );
				
			IPAddress myIP = WiFi.softAPIP();
			sendMessage( PSTR("IP address: ") + myIP.toString() );
			sendMessage( PSTR("Hostname: ") + String(WiFi.softAPgetHostname()) );
			getWebServer().begin();//Start up page server.
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
	WiFi.disconnect( true ); //Disconnect the wifi
	//WiFi.mode(WIFI_OFF);
}

void UICore::scanNetworks()
{
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
}

bool UICore::beginConnection( const String &ssid, const String &password )
{
	if ( WiFi.isConnected() )
	{
		sendMessage( PSTR("Connected to '") + WiFi.SSID() + PSTR("'. Disconnect before attempting a new connection."), PRIORITY_HIGH );
		return false;
	}
	
	sendMessage( PSTR("Attempting connection to: ") + ssid );

	uint8_t i_retries = 0;
	while ( i_retries <= i_connectionRetries )
	{
		if ( WiFi.begin( ssid.c_str(), password.c_str() ) == WL_CONNECT_FAILED )
		{
			sendMessage( PSTR("Failed to begin Wifi connection (Invalid password or SSID?)") , true );
			closeConnection();//Just in case
			return false;
		}
		else 
		{
			uint8_t i_seconds = 0; //elapsed seconds
			uint32_t nextMillis = millis() + 1000;
			while ( !WiFi.isConnected() && i_seconds < i_timeoutLimit )
			{
				while ( nextMillis > millis()){}
				nextMillis = millis() + 1000; //delay 1 second at a time
				i_seconds++;
			}

			if ( WiFi.isConnected() )
			{
				WiFi.setHostname(String(getWiFiHostname() + String(DATA_SPLIT) + getUniqueID()).c_str());
				sendMessage( PSTR("Connected to ") + ssid + PSTR(" with local IP: ") + WiFi.localIP().toString() + PSTR(" Hostname: ") + WiFi.getHostname() );
				WiFi.setAutoReconnect(b_autoRetryConnection);
				getWebServer().begin(); //start the server
				return true; //end the function here
			}
		}

		if ( i_retries < i_connectionRetries )
			sendMessage( PSTR("Retrying connection to: ") + ssid );
		
		closeConnection( false ); //reset before we try again
		i_retries++;
	}

	sendMessage( PSTR("Connection to ") + ssid + PSTR(" timed out."), PRIORITY_HIGH ); 
	closeConnection( false );
	return false;
}

void UICore::sendMessage( const String &str, uint8_t priority )
{
	if ( priority <= i_verboseMode ) 
	{
		uint16_t vectorSize = str.length(); //start with the size of the incoming string, since it will be added to the vector.
		for ( uint8_t x = 0; x < alerts.size(); x++ )
				vectorSize += alerts[x].length();

		while( vectorSize > MAX_MESSAGE_HISTORY_SIZE ) //trim the vector if we start using too much memory
		{
			vectorSize -= alerts.front().length();
			alerts.erase(alerts.cbegin()); //delete the first element (oldest) in the vector.
		}

		alerts.push_back( getSystemTimeObj()->GetTimeStr() + CHAR_SPACE + str ); //store in history for web UI clients
		Serial.println( getSystemTimeObj()->GetTimeStr() + CHAR_SPACE + str ); //send it to the serial interface, regardless of whether anyone sees it or not
	}
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
	//sendMessage( PSTR("Port: ") + String(MDNS.port()) );
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
		
		if ( p_currentTime->IsBehind( p_nextNISTUpdateTime.get() ) && !force ) //too soon for an update?
			return false;
		
		sendMessage( PSTR("Updating time." ) );
		
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
				String line = NISTclient.readStringUntil(CHAR_CARRIAGE); //DAYTIME protocol - meh, it works.
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
		
		p_nextNISTUpdateTime->SetTime( p_currentTime.get() ); //Replace with current time
		p_nextNISTUpdateTime->IncrementTime( i_NISTupdateFreq, i_NISTUpdateUnit );//Then increment -- need to rework this a bit
		
		return true; //End here, we'll let the system clock carry on on the next second.
	}
	return false; //default path
}



