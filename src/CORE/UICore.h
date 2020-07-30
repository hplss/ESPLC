/*
 * EspComm.h
 *
 * Created: 10/12/2019 3:43:16 PM
 *  Author: Andrew
 * This header contains the base functions for the project.
 */ 

#include <vector>
#include <WString.h>
#include <WebServer.h>
#include <SPIFFS.h>
#include <esp_wifi.h>
#include <map>
#include <memory>

#include "GlobalDefs.h"
#include "../web/data_fields.h" //depends on settings.h --must come afterwards
#include "Time.h"

using namespace std;

#ifndef UICore_H_
#define UICore_H_

#define MAX_MESSAGE_HISTORY_SIZE 3072 //total number of characters allowed to be stored in WEB UI alerts history (before a client has read them). 3KB seems like enough?

class Device_Setting
{
	public:
	Device_Setting( bool *ptr  ){ i_Type = TYPE_VAR_BOOL; data.b_Ptr = ptr; }
	Device_Setting( uint8_t *ptr  ){ i_Type = TYPE_VAR_UBYTE; data.ui8_Ptr = ptr; }
	Device_Setting( uint16_t *ptr  ){ i_Type = TYPE_VAR_USHORT; data.ui16_Ptr = ptr; }
	Device_Setting( uint_fast32_t *ptr  ){ i_Type = TYPE_VAR_UINT; data.ui_Ptr = ptr; }
	Device_Setting( String *ptr  ){ i_Type = TYPE_VAR_STRING; data.s_Ptr = ptr; }
	Device_Setting( shared_ptr<String> ptr ){ i_Type = TYPE_VAR_STRING; data.s_Ptr = ptr.get(); }
	virtual ~Device_Setting(){} //destructor

	uint8_t getType(){ return i_Type; }

	//This function converts a string to the proper value and stores it into the appropriate variable
	void setSettingValue( const String & );
	//Returns the current value of the setting stored in the object.
	String getSettingValue();

	uint8_t &getUINT8(){ return *data.ui8_Ptr; }
	uint_fast32_t &getUINT(){ return *data.ui_Ptr; }
	uint16_t &getUINT16(){ return *data.ui16_Ptr; }
	bool &getBOOL(){ return *data.b_Ptr; }
	String &getSTRING(){ return *data.s_Ptr; }
	
	private:
	union settingVar
	{
		bool *b_Ptr;
		String *s_Ptr;
		uint8_t *ui8_Ptr;
		uint16_t *ui16_Ptr;
		uint_fast32_t *ui_Ptr;
	} data;
	uint8_t i_Type; //stored the field type, because we can't cast
};

class UICore
{
public:
	UICore()
	{	
		//Time Stuff
		p_currentTime = make_shared<Time>();
		p_nextNISTUpdateTime = make_shared<Time>();
		
		//
		//Default stuff for now, until EEPROM values are loaded (also initializes shared pointers -- VERY IMPORTANT)
		i_verboseMode = PRIORITY_LOW; //Do this by default for now, will probably have an eeprom setting for this later.
		i_timeoutLimit = 15; //20 second default, will probably be an eeprom config later
		b_enableNIST = true;
		b_enableAP = false;
		b_enableDNS = false;
		b_FSOpen = false; //init to false;
		b_SaveScript = false; //default to off
		b_SaveConfig = false; 
		i_NISTupdateFreq = 5; //default to 5...
		i_NISTUpdateUnit = TIME_MINUTE; //minutes (default for now)
		s_NISTServer = make_shared<String>(PSTR("time.nist.gov")); //Default for now.
		s_WiFiPWD = make_shared<String>();
		s_WiFiSSID = make_shared<String>();
		s_uniqueID = make_shared<String>(PSTR("DEFID")); //Default for now
		i_NISTPort = 13; //default
		i_nistMode = 1; //NTP
		s_StyleSheet = make_shared<String>();
		s_DNSHostname = make_shared<String>();
		s_WiFiHostname = make_shared<String>();
		s_BTPWD = make_shared<String>();
		s_authenName = make_shared<String>();
		s_authenPWD = make_shared<String>();
		s_TimeString = make_shared<String>();
		s_plc_addresses = make_shared<String>();
		s_plc_port_ranges = make_shared<String>();
		s_plc_ip_ranges = make_shared<String>();
		//
	}
	~UICore()
	{
		//p_indexDataTables.clear();
		//p_adminDataTables.clear();
		//p_styleDataTables.clear();
		//p_scriptDataTables.clear();
		p_UIDataTables.clear();
		closeConnection(); //end all wifi/webserver stuff
	}
	
	//Outputs to serial (USB) details on various available WiFi networks that are within the range of the device.
	void scanNetworks(); 
	//This function creates a wireless access point using the integrated WiFi functionality.
	//Args: SSID, PASSWORD
	bool setupAccessPoint( const String &, const String & ); 

	//this function is reponsible for reading incoming serial data (over USB), breaking the data up, and passing it into the appropriate functions.
	void parseSerialData();
	//Used as the first serial data interpreter, to divide up all arguments and pass them to their respective functions.
	vector<String> parseArgs( int &, const uint8_t, const char buffer[] ); 
	//Used by the serial parser to set the device as a wireless access point.
	bool parseAccessPoint( const vector<String> & ); 
	//Used by the serial parser to connect to existing wireless networks that are within range of the device.
	void parseConnect( const vector<String> & );
	//Used by the serial parser to modify device message verbosity settings (as it relates to serial or the web UI).
	void parseVerbose( const vector<String> & ); 
	//Used by the serial parser to change certain system time related settings. No input arguments returns current system time.
	void parseTime( const vector<String> & ); 
	//Used by the serial parser to program specific values into non-volatile storage (default wifi connection, so on).
	void parseCfg( const vector<String> & ); 
	//Fills the settings map used for interpreting settings storage/reading to/from SPIFFS (flash file system).
	void generateSettingsMap(); 
	//Allows the memory used by the map to be freed after parsing/storing, etc (optimization).
	void clearSettingsMap(); 
	
	//This basically functions as our main loop function for the Core UI.
	void Process(); 
	//Updates device timer settings
	void updateClock(); 
	//Core UI Setup function. To be called when device initializes.
	void setup(); 
	//Links specific web server address to corresponding page HTML generation functions.
	void setupServer(); 
	//Used to connect to an existing wireless network.
	//Args: SSID, PASSWORD
	bool beginConnection( const String &, const String & ); 
	//Used to close all network connections (WiFi and Web) to the device.
	void closeConnection( bool = true );
	//This prepares the inputted string for serial transmission (over USB), if applicable. 
	void sendMessage( const String &, uint8_t = PRIORITY_LOW ); 
	//Used to display current network connection information over serial (USB), as well as other device statistics and statuses.
	void printDiag(); 
	
	//This generates the index page HTML, which functions as a "main menu" for the web UI.
	void handleIndex(); 
	//Generates the HTML for the device administration page. This page is used for configuring device specific settings.
	void handleAdmin(); 
	//Generates the page HTML for editing the PLC logic script.
	void handleScript(); 
	//Generates the page HTML for the CSS editor Page. Used to edit the web UI graphics properties.
	void handleStyleSheet(); 
	//This function handles the user authorization prompt that is present on certain device configuration pages.
	bool handleAuthorization();
	//generates the page HTML for viewing initialized PLC ladder logic objects in the web UI.
	void handleStatus();
	//The actual style sheet file, for sending in chunks directly from flash to the user
	void sendStyleSheet(); 
	//Sends ystem alerts and other info over the web interface.
	void handleAlerts();

	//Creates static data fields for admin page.
	void createAdminFields(); 
	//Creates static data fields for index page.
	void createIndexFields(); 
	//Creates static data fields for style page.
	void createStyleSheetFields(); 
	//Creates static data fields for PLC Logic page.
	void createScriptFields(); 
	//Creates any necessary fields/tables for PLC ladder object status. 
	void createStatusFields();

	//Generates the JSON array for handling object status updates, etc.
	String generateStatusJSON();
	//Generates the JSON array for sending device alerts to a client that is viewing the web UI.
	String generateAlertsJSON();

	//Updates web UI data fields. Appends setting change notifications (if applicable) to HTML for user's reference.
	void UpdateWebFields( const vector<shared_ptr<DataTable>> & ); 
	//Generates the title HTML for each web UI page.
	String generateTitle(const String &data = ""); 
	//Generates the header HTML for each web UI page, and factors in style sheet data.
	String generateHeader(); 
	//Generates the common HTML footer for all web UI pages.
	String generateFooter(); 
	//Generates the common HTML for viewing device errors and other outputs.
	//The only argument is the ID for the textbox field that the messages are to be displayed in.
	String generateAlertsScript( uint8_t );
	//Generates the script that allows for status updates on the status page.
	String generateStatusScript();

	//Applies the user inputted PLC logic script from the web UI onto the device. 
	static void applyLogic(); 
	//Applies changes to the CSS file for the web interface.
	static void applyStyleSheet(); 
	//Applies changes for device settings (admin panel)
	static void applyDeviceSettings(); 

	//Determines if a NIST server check should be performed.
	bool CheckUpdateNIST(); 
	//Performs the NIST update operation.
	bool UpdateNIST( bool = false ); 

	//Saves the device settings (wifi, time, etc.)
	bool saveSettings(); 
	//Saves the logic script for the PLC system. Also applies it.
	bool savePLCScript(const String &); 
	//Saves the style sheet for the web based UI
	bool saveWebStyleSheet( const String &); 
	//Loads default configuration settings from filesystem
	bool loadSettings(); 
	//This function applies current settings to the systems that use them (IE: Updating WiFi AP with new SSID, etc.)
	//arg(s) <bool> : load from storage
	void applySettings( bool = false ); 
	//Loads the default logic script for the PLC system from the flash file system.
	bool loadPLCScript( String & ); 
	//Loads a custom stylesheet (CSS) for web the based UI from the flash file system.
	String loadWebStylesheet(); 

	//Accessors for stored pointers (Local device settings)
	String &getWiFiHostname(){ return *s_WiFiHostname.get(); }
	String &getDNSHostname(){ return *s_DNSHostname.get(); }
	String &getWiFiSSID(){ return *s_WiFiSSID.get(); }
	String &getWiFiPWD(){ return *s_WiFiPWD.get(); }
	String &getNISTServer(){ return *s_NISTServer.get(); }
	String &getUniqueID(){ return *s_uniqueID.get(); }
	String &getStyleSheet(){ return *s_StyleSheet.get(); }
	String &getLoginName(){ return *s_authenName.get(); }
	String &getLoginPWD(){ return *s_authenPWD.get(); }
	String &getBTPWD(){ return *s_BTPWD.get(); }
	//

	//Accessors for stored pointers (PLC remote settings)
	String &getPLCIPRange(){ return *s_plc_ip_ranges; }
	String &getPLCPortRange(){ return *s_plc_port_ranges; }
	String &getPLCAutoConnectIPs(){ return *s_plc_addresses; }
	//

	shared_ptr<Time> getSystemTimeObj(){ return p_currentTime; }
	WebServer &getWebServer(){ return *p_server.get(); }
	WiFiUDP &getTimeUDP(){ return *p_UDP.get(); }

private:
	//Acts as storage for the data tables for the WEB UI for all pages.
	vector <shared_ptr<DataTable>> p_UIDataTables; 

	shared_ptr<WebServer> p_server; //web server object
	shared_ptr<WiFiUDP> p_UDP; //UDP protocol object.
	
	uint8_t i_timeoutLimit;
	uint8_t i_verboseMode; //Used to determine what messages should be sent over the serial channel
	
	//device specific settings
	shared_ptr<String> s_uniqueID; //Unique ID string for device. 
	//
	
	//System Clock objects
	shared_ptr<Time> p_currentTime;
	shared_ptr<Time> p_nextNISTUpdateTime; //Used to store the time for next NIST update.
	uint8_t i_nistMode; //Daylight vs NTP protocol
	shared_ptr<String> s_NISTServer;
	unsigned int i_NISTPort;
	//
	
	//NIST time variables
	unsigned int i_NISTupdateFreq; //frequency of NIST time update
	uint8_t i_NISTUpdateUnit; //Unit of time for frequency between updates.
	bool b_enableNIST; //Enable time server update mode?
	shared_ptr<String> s_TimeString;
	//
	
	//WiFi specific settings
	bool b_enableAP; //Enable access point mode?
	bool b_enableDNS; //Enable DNS server for resolving local IP
	bool b_APFallback; //Automatically revert to AP mode upon failure to connect to station.
	bool b_autoConnect; //Automatically connect to a given SSID
	bool b_autoRetryConnection; //Automatically retry on disconnect?
	shared_ptr<String> s_WiFiSSID, //SSID for AP mode broadcast or station connection.
					   s_WiFiPWD, //Password to connect to AP or station
					   s_WiFiHostname, //Hostname for the device.
					   s_DNSHostname; //Hostname for DNS server.
	//

	//External devices settings variables
	uint8_t i_plc_netmode,//"plc_netmode"
			i_plc_ip_range;
	uint16_t i_plc_broadcast_port;
	bool b_plc_autoconnect;
	vector<IPAddress> plc_saved_addresses;
	shared_ptr<String> s_plc_addresses,
					   s_plc_port_ranges,
					   s_plc_ip_ranges;
	//

	//File system related variables
	bool b_FSOpen; //flag to let us know if we were abel to successfully open the file system
	bool b_SaveScript; //used for saving the current PLC script to the file system
	bool b_SaveConfig; //Used for saving the chosen device configuration
	bool b_SaveStyleSheet; //Used for saving the CSS file for the web based UI
	//

	//Web Style Sheet variables
	shared_ptr<String> s_StyleSheet;
	//End web Style Sheet vars

	//Security and access related settings
	shared_ptr<String> s_authenName,
					   s_authenPWD,
					   s_BTPWD;
	bool b_enableBT;
	//

	vector<String> alerts; //vestor that stores alerts that have yet to be forwarded to a web client.

	//Settings storage/reading variables
	std::map<const String, shared_ptr<Device_Setting>> settingsMap;
	std::map<const String, shared_ptr<Device_Setting>>::iterator settings_itr;
	//
};

long parseInt( const String & );
float parseFloat( const String & );
vector<String> splitString(const String &, const char);

#endif /* UICore_H_ */