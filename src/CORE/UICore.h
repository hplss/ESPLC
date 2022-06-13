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
	Device_Setting( bool *ptr  ){ i_Type = OBJ_TYPE::TYPE_VAR_BOOL; data.b_Ptr = ptr; }
	Device_Setting( uint8_t *ptr  ){ i_Type = OBJ_TYPE::TYPE_VAR_UBYTE; data.ui8_Ptr = ptr; }
	Device_Setting( uint16_t *ptr  ){ i_Type = OBJ_TYPE::TYPE_VAR_USHORT; data.ui16_Ptr = ptr; }
	Device_Setting( uint_fast32_t *ptr  ){ i_Type = OBJ_TYPE::TYPE_VAR_UINT; data.ui_Ptr = ptr; }
	Device_Setting( String *ptr  ){ i_Type = OBJ_TYPE::TYPE_VAR_STRING; data.s_Ptr = ptr; }
	Device_Setting( shared_ptr<String> ptr ){ i_Type = OBJ_TYPE::TYPE_VAR_STRING; data.s_Ptr = ptr.get(); } //uses raw pointers... ew
	virtual ~Device_Setting(){} //destructor

	void setValue( const String &str );
	template <typename T>
	void setValue(const T &val)
	{
		switch(i_Type) //local type
		{
			case OBJ_TYPE::TYPE_VAR_BOOL:
				*data.b_Ptr = static_cast<bool>(val);
			break;
			case OBJ_TYPE::TYPE_VAR_UBYTE:
				*data.ui8_Ptr = static_cast<uint8_t>(val);
			break;
			case OBJ_TYPE::TYPE_VAR_USHORT:
				*data.ui16_Ptr = static_cast<uint16_t>(val);
			break;
			case OBJ_TYPE::TYPE_VAR_UINT:
				*data.ui_Ptr = static_cast<uint_fast32_t>(val);
			break;
			case OBJ_TYPE::TYPE_VAR_STRING:
				*data.s_Ptr = static_cast<String>(val);
			break;
			default:
			break;
		}
	}

	//Returns the current value of the setting stored in the object.
	template <typename T>
	const T getValue()
	{
		switch(i_Type) //local type
		{
			case OBJ_TYPE::TYPE_VAR_BOOL:
				return static_cast<T>(*data.b_Ptr);
			break;
			case OBJ_TYPE::TYPE_VAR_UBYTE:
				return static_cast<T>(*data.ui8_Ptr);
			break;
			case OBJ_TYPE::TYPE_VAR_USHORT:
				return static_cast<T>(*data.ui16_Ptr);
			break;
			case OBJ_TYPE::TYPE_VAR_UINT:
				return static_cast<T>(*data.ui_Ptr);
			break;
			case OBJ_TYPE::TYPE_VAR_STRING:
				return static_cast<T>(*data.s_Ptr);
			break;
			default:
				return static_cast<T>(0);
			break;
		}
	}
	
	
	private:
	union
	{
		bool *b_Ptr;
		String *s_Ptr;
		uint8_t *ui8_Ptr;
		uint16_t *ui16_Ptr;
		uint_fast32_t *ui_Ptr;
	} data;

	OBJ_TYPE i_Type; //stored the field type, because we can't cast
};

using SETTING_PTR = shared_ptr<Device_Setting>;

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
		i_timeoutLimit = 3; //3 seconds is good enough
		i_connectionRetries = 3; //3 attempts to reconnect
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
		s_WiFiAPPWD = make_shared<String>();
		s_WiFiSSID = make_shared<String>();
		s_WiFiAPSSID = make_shared<String>(PSTR("ESPLC"));
		s_uniqueID = make_shared<String>(PSTR("DEFID")); //Default for now
		i_NISTPort = 13; //default
		i_nistMode = 1; //NTP
		s_StyleSheet = make_shared<String>();
		s_DNSHostname = make_shared<String>();
		s_WiFiHostname = make_shared<String>();
		s_BTPWD = make_shared<String>();
		s_authenName = make_shared<String>();
		s_authenPWD = make_shared<String>();

		i_plc_netmode = 0;
		i_plc_broadcast_port = 5000;
		//
	}
	~UICore()
	{
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
	//Creates a vector of IP addresses based on delimiter(s) from a given String
	vector<IPAddress> parseIPAddress( const String &, const vector<char> &  ); 
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
	//Generates the page HTML for the firmware updater page.
	void handleUpdater();
	//This function handles the user authorization prompt that is present on certain device configuration pages.
	bool handleAuthorization();
	//generates the page HTML for viewing initialized PLC ladder logic objects in the web UI.
	void handleStatus();
	//Updates the status page with the current logic object states.
	void handleUpdateStatus();
	//The actual style sheet file, for sending in chunks directly from flash to the user
	void sendStyleSheet(); 
	//Sends ystem alerts and other info over the web interface.
	void handleAlerts();

	//File Page related functions
	bool handleFileRead(const String &);
	void handleFileDisplay();
	void handleFileUpload();
	void handleFileDelete();
	void handleFileCreate();
	void handleFileList();
	//

	void resestFieldContainers();

	//Creates static data fields for admin page.
	void createAdminFields(); 
	//Creates static data fields for index page.
	void createIndexFields(); 
	//Creates static data fields for style page.
	void createStyleSheetFields(); 
	//Creates statis data fields for firmware updater page.
	void createUpdaterFields();
	//Creates static data fields for PLC Logic page.
	void createScriptFields(); 
	//Creates any necessary fields/tables for PLC ladder object status. 
	void createStatusFields();

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
	//This function handles the application of OTA (over the air) firmware updates via the Web UI
	static void applyRemoteFirmwareUpdate();

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
	String &getWiFiAPSSID(){ return *s_WiFiAPSSID.get(); }
	String &getWiFiSSID(){ return *s_WiFiSSID.get(); }
	String &getWiFiAPPWD(){ return *s_WiFiAPPWD.get(); }
	String &getWiFiPWD(){ return *s_WiFiPWD.get(); }
	String &getNISTServer(){ return *s_NISTServer.get(); }
	String &getUniqueID(){ return *s_uniqueID.get(); }
	String &getStyleSheet(){ return *s_StyleSheet.get(); }
	String &getLoginName(){ return *s_authenName.get(); }
	String &getLoginPWD(){ return *s_authenPWD.get(); }
	String &getBTPWD(){ return *s_BTPWD.get(); }
	//

	shared_ptr<Time> getSystemTimeObj(){ return p_currentTime; }
	WebServer &getWebServer(){ return *p_server.get(); }
	WiFiUDP &getTimeUDP(){ return *p_UDP.get(); }

private:
	//Acts as storage for the data tables for the WEB UI for all pages.
	vector <shared_ptr<DataTable>> p_UIDataTables; 
	vector <shared_ptr<DataTable>> p_StaticDataTables; 

	shared_ptr<WebServer> p_server; //web server object
	shared_ptr<WiFiUDP> p_UDP; //UDP protocol object.
	
	uint8_t i_timeoutLimit;
	uint8_t i_connectionRetries;
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
	//
	
	//WiFi specific settings
	bool b_enableAP; //Enable access point mode?
	bool b_enableDNS; //Enable DNS server for resolving local IP
	bool b_APFallback; //Automatically revert to AP mode upon failure to connect to station.
	bool b_autoConnect; //Automatically connect to a given SSID
	bool b_autoRetryConnection; //Automatically retry on disconnect?
	shared_ptr<String> s_WiFiSSID, //SSID for station connection.
					   s_WiFiAPSSID, //SSID for AP mode broadcast
					   s_WiFiAPPWD, //Password to connect to access point
					   s_WiFiPWD, //Password to connect to station
					   s_WiFiHostname, //Hostname for the device.
					   s_DNSHostname; //Hostname for DNS server.
	//

	//External PLC devices settings
	uint8_t i_plc_netmode;
	uint16_t i_plc_broadcast_port;
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
	std::map<String, SETTING_PTR> settingsMap;
	std::map<String, SETTING_PTR>::iterator settings_itr;
	//
};


extern UICore Core;

#endif /* UICore_H_ */