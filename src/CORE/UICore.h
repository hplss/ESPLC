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

enum SETTING_TYPE
{
	TYPE_BOOL = 0,
	TYPE_STRING,
	TYPE_UINT8,
	TYPE_UINT
};

class Device_Setting
{
	public:
	Device_Setting( bool *ptr  ){ i_Type = SETTING_TYPE::TYPE_BOOL; data.b_Ptr = ptr; }
	Device_Setting( uint8_t *ptr  ){ i_Type = SETTING_TYPE::TYPE_UINT8; data.ui8_Ptr = ptr; }
	Device_Setting( unsigned int *ptr  ){ i_Type = SETTING_TYPE::TYPE_UINT; data.ui_Ptr = ptr; }
	Device_Setting( String *ptr  ){ i_Type = SETTING_TYPE::TYPE_STRING; data.s_Ptr = ptr; }
	virtual ~Device_Setting(){} //destructor

	union settingVar
	{
		bool *b_Ptr;
		String *s_Ptr;
		uint8_t *ui8_Ptr;
		unsigned int *ui_Ptr;
	};

	uint8_t getType(){ return i_Type; }

	//This function converts a string to the proper value and stores it into the appropriate variable
	void setSettingValue( const String & );
	//Returns the current value of the setting stored in the object.
	String getSettingValue();

	uint8_t &getUINT8(){ return *data.ui8_Ptr; }
	unsigned int &getUINT(){ return *data.ui_Ptr; }
	bool &getBOOL(){ return *data.b_Ptr; }
	String &getSTRING(){ return *data.s_Ptr; }

	private:
	settingVar data; //union for storing pointers
	uint8_t i_Type; //stored the field type, because we can't cast
};

class UICore
{
public:
	UICore()
	{	
		s_authenName = make_shared<String>();
		s_authenPWD = make_shared<String>();
		s_BTPWD = make_shared<String>();
		s_authenName = make_shared<String>("");
		s_authenPWD = make_shared<String>("");
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
	
	void scanNetworks(); //Used to output to serial that gives detains on various available networks.
	bool setupAccessPoint( const String &, const String & ); //This function creates an access point based on inputted data

	//Parser stuff
	void parseSerialData();
	vector<String> parseArgs( int &, const uint8_t, const char buffer[] ); //Used as the first interpreter, to divide up all arguments to their respective functions.
	bool parseAccessPoint( const vector<String> & ); //Used to set the device as a wireless access point.
	void parseConnect( const vector<String> & ); //Used to parse values used to connect to existing wireless networks.
	void parseVerbose( const vector<String> & ); //Used to enable/disable verbose mode.
	void parseLogin( const vector<String> & ); //Used to log into (and load) user data stored remotely
	void parseTime( const vector<String> & ); //Used to change certain system time related settings. No args returns current time.
	void parseCfg( const vector<String> & ); //Used to program specific values into non-volatile storage (default wifi connection, so on)
	void generateSettingsMap(); //Fills the map used for interpreting settings storage/reading to/from SPIFFS
	void clearSettingsMap(); //Allows the memory used by the map to be freed after parsing/storing, etc.
	//
	
	void Process(); //This basically functions as our loop function
	void updateClock(); //Updates device timer settings
	void setup(); //Setup functions to be called when device inits.
	void setupServer(); //Links specific web server address to corresponding page generation functions.
	bool beginConnection( const String &, const String & ); //Used to connect to an existing wireless network.
	void closeConnection( bool = true ); //Used to close all connections to the ESP device.
	void sendMessage( const String &, uint8_t = PRIORITY_LOW ); //This prepares the inputted string for serial transmission, if applicable. 
	void printDiag(); //Used to display current connection information for the wifi device.
	
	//These functions handle the generation of HTML pages t be transmitted to users
	void HandleIndex(); //Index that displays the data fields.
	void HandleAdmin(); //Administration page (instructor page)
	void HandleScript(); //Page for editing the PLC logic script.
	void handleStyleSheet(); //Stylesheet page, to edit the UI graphics properties
	bool handleAuthorization();
	void sendStyleSheet(); //The actual style sheet file, for sending in chunks directly from flash to the user
	void sendJQuery(); //not used yet
	
	void createAdminFields(); //Create static data fields for admin page.
	void createIndexFields(); //Create static data fields for index page.
	void createStyleSheetFields(); //Create static data fields for style page.
	void createScriptFields(); //Create static data fields for PLC Logic page.

	void UpdateWebFields( const vector<shared_ptr<DataTable>>, String & ); //Updates data fields. Appends setting change notifications (if applicable) 
	String generateTitle(const String &data = ""); //Generates the title for each UI page.
	String generateHeader(); //Generates the header for each web pages, factors in style sheet data
	String generateFooter(); //Generates the common footer for all pages
	//

	//used to apply a user inputted PLC logic script from the web UI
	static void applyLogic(); 
	//used to apply changes to the CSS file for the web interface
	static void applyStyleSheet(); 
	//used to apply changes for device settings (admin panel)
	static void applyDeviceSettings(); 

	bool CheckUpdateNIST(); //Used to determine if a NIST server check should be performed.
	bool UpdateNIST( bool = false ); //Used to perform the NIST update.

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
	bool loadPLCScript( String & ); //Loads the default script for the PLC system
	String loadWebStylesheet(); //Loads a custom stylesheet for web based UI (optional) 
	//End storage related functions

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

	WebServer &getWebServer(){ return *p_server.get(); }
	WiFiUDP &getTimeUDP(){ return *p_UDP.get(); }

private:
	vector <shared_ptr<DataTable>> p_UIDataTables; //Acts as storage for the data tables for the WEB UI for all pages.

	shared_ptr<WebServer> p_server; //web server object
	shared_ptr<WiFiUDP> p_UDP; //UDP protocol object.
	
	uint8_t i_timeoutLimit;
	uint8_t i_verboseMode; //Used to determine what messages should be sent over the serial channel
	
	//device specific settings
	shared_ptr<String> s_uniqueID; //Unique ID string for device. 
	//
	
	//System Clock objects
	Time *p_currentTime;
	Time *p_nextNISTUpdateTime; //Used to store the time for next NIST update.
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
	shared_ptr<String> s_WiFiSSID, //SSID for AP mode broadcast or station connection.
					   s_WiFiPWD, //Password to connect to AP or station
					   s_WiFiHostname, //Hostname for the device.
					   s_DNSHostname; //Hostname for DNS server.
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

	//Settings storage/reading variables
	std::map<const String, shared_ptr<Device_Setting>> settingsMap;
	std::map<const String, shared_ptr<Device_Setting>>::iterator settings_itr;
	//
};

long parseInt( const String &str );
vector<String> splitString(const String &, const char);


#endif /* UICore_H_ */