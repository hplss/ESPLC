#include "UICore.h"
#include "GlobalDefs.h"
#include <PLC/PLC_Main.h>


//SPIFFS (flash file system) messages stored in program memory
const String &err_Script PROGMEM = PSTR("Failed to load PLC Script!"),
             &err_Style PROGMEM = PSTR("Failed to load web stylesheet!"),
             &err_Config PROGMEM = PSTR("Failed to load device configuration!"),
             &succ_Script PROGMEM = PSTR("PLC Script saved."),
             &succ_Style PROGMEM = PSTR("Web stylesheet saved."),
             &succ_Config PROGMEM = PSTR("Device configuration saved."),
             &succ_Script_loaded PROGMEM = PSTR("PLC script loaded."),
             &succ_Style_loaded PROGMEM = PSTR("Web stylesheet loaded."),
             &succ_Config_loaded PROGMEM = PSTR("Device configuration loaded.");

void Device_Setting::setValue( const String &str )
{
    switch(i_Type)
    {
        case OBJ_TYPE::TYPE_VAR_BOOL:
        {
            *data.b_Ptr = str.toInt() > 0 ? true : false;
        }
        break;
        case OBJ_TYPE::TYPE_VAR_STRING:
        {
            *data.s_Ptr = str;
        }
        break;
        case OBJ_TYPE::TYPE_VAR_UBYTE:
        {
            *data.ui8_Ptr = static_cast<uint8_t>(parseInt(str)); //we can only assume it won't overflow
        }
        break;
        case OBJ_TYPE::TYPE_VAR_UINT:
        {
            *data.ui_Ptr = parseInt(str); //we can only assume it won't overflow
        }
        break;
        case OBJ_TYPE::TYPE_VAR_USHORT:
        {
            *data.ui16_Ptr = static_cast<uint16_t>(parseInt(str)); //we can only assume it won't overflow
        }
        break;
        default:
        break;
    }
}

void UICore::generateSettingsMap()
{
    //Device specific settings
    settingsMap.emplace(PSTR("dev_id"), make_shared<Device_Setting>( &getUniqueID() ) ); //Unique ID of the ESPLC device
    settingsMap.emplace(PSTR("dev_serial_v"), make_shared<Device_Setting>( &i_verboseMode) ); //Serial verbosity settings, for debugging/status updates on local device

    //security related settings
    settingsMap.emplace(PSTR("bt_en"), make_shared<Device_Setting>( &b_enableBT) ); //Enable bluetooth interface
    settingsMap.emplace(PSTR("bt_pwd"), make_shared<Device_Setting>( &getBTPWD() ) ); //password for Bluetooth adaptor connectivity
    settingsMap.emplace(PSTR("ui_uname"), make_shared<Device_Setting>( &getLoginName() ) ); //username for web UI access (security)
    settingsMap.emplace(PSTR("ui_pwd"), make_shared<Device_Setting>( &getLoginPWD() ) ); //password for web UI access (security)

    //Network settings
    settingsMap.emplace(PSTR("net_ap_en"), make_shared<Device_Setting>( &b_enableAP) ); //enable access point mode
    settingsMap.emplace(PSTR("net_sta_retry"), make_shared<Device_Setting>( &b_autoRetryConnection) ); //Retry connection to internet on failure
    settingsMap.emplace(PSTR("net_sta_retry_count"), make_shared<Device_Setting>( &i_connectionRetries ) ); //number of attempts to retry a connection
    settingsMap.emplace(PSTR("net_sta_retry_timeout"), make_shared<Device_Setting>( &i_timeoutLimit) ); //seconds before timeout on connection
    settingsMap.emplace(PSTR("net_ap_ssid"), make_shared<Device_Setting>( &getWiFiAPSSID() ) );
    settingsMap.emplace(PSTR("net_sta_ssid"), make_shared<Device_Setting>( &getWiFiSSID() ) ); //SSID of Wifi connection - for autoconnection
    settingsMap.emplace(PSTR("net_ap_pwd"), make_shared<Device_Setting>( &getWiFiAPPWD() ) ); //password for WiFi auto-connection
    settingsMap.emplace(PSTR("net_sta_pwd"), make_shared<Device_Setting>( &getWiFiPWD() ) ); //password for WiFi auto-connection
    settingsMap.emplace(PSTR("net_hostname"), make_shared<Device_Setting>( &getWiFiHostname() ) ); 
    settingsMap.emplace(PSTR("dns_en"), make_shared<Device_Setting>( &b_enableDNS ) ); //enable DNS server
    settingsMap.emplace(PSTR("dns_hostname"), make_shared<Device_Setting>( &getDNSHostname() ) ); //hostname for DNS server.

    //PLC networking settings
    settingsMap.emplace(PSTR("plc_netmode"), make_shared<Device_Setting>( &i_plc_netmode )); //Switch for disabled (0), IO expander mode (1), or cluster mode (2)
    settingsMap.emplace(PSTR("plc_broadcast_port"), make_shared<Device_Setting>( &i_plc_broadcast_port )); //status broadcast port 

    //Time Settings
	settingsMap.emplace(PSTR("time_en"), make_shared<Device_Setting>( &b_enableNIST) ); //Enable automatic time fetching when connected to internet
	settingsMap.emplace(PSTR("time_server"), make_shared<Device_Setting>( &getNISTServer() ) ); //URL for time fetching
    settingsMap.emplace(PSTR("time_port"), make_shared<Device_Setting>( &i_NISTPort) ); //port for time fetching
    settingsMap.emplace(PSTR("time_upd_freq"), make_shared<Device_Setting>( &i_NISTupdateFreq) ); //update frequency for time fetching. 
}

void UICore::clearSettingsMap()
{
	settingsMap.clear(); //empty the map. Should delete the objects that were created.
}


bool UICore::loadSettings()
{
    if ( !b_FSOpen )
        return false;

    File settingsFile = SPIFFS.open(file_Configuration, FILE_READ);
    if (!settingsFile)
    {
        sendMessage(err_Config, PRIORITY_HIGH);
        return false;
    }

    generateSettingsMap();

    while(settingsFile.position() != settingsFile.size()) //Go through the entire settings file
    {
        String settingID = settingsFile.readStringUntil(CHAR_EQUALS),
               settingValue = settingsFile.readStringUntil(CHAR_NEWLINE);

        for ( settings_itr = settingsMap.begin(); settings_itr != settingsMap.end(); settings_itr++ )
        {
            if ( settings_itr->first == settingID ) //search for the specific setting string identifier
                settings_itr->second.get()->setValue(settingValue);
        }
    }

    clearSettingsMap();
    sendMessage(succ_Config_loaded);
    settingsFile.close();
    return true;
}

bool UICore::loadPLCScript( String &script )
{
    if ( !b_FSOpen )
        return false;

    File scriptFile = SPIFFS.open(file_Script, FILE_READ);
    if (!scriptFile)
    {
        sendMessage(err_Script, PRIORITY_HIGH);
        return false;
    }

    script = scriptFile.readString(); //read directly into scrpt variable
    scriptFile.close();
    sendMessage(succ_Script_loaded);
    return true;
}

String UICore::loadWebStylesheet()
{
    if ( !b_FSOpen )
        return "";

    File styleFile = SPIFFS.open(file_Stylesheet, FILE_READ);
    if (!styleFile)
    {
        sendMessage(err_Style, PRIORITY_HIGH);
        return "";
    }

    String sheetdata = styleFile.readString();

    styleFile.close();
    sendMessage(succ_Style_loaded);
    return sheetdata; //return the style sheet
}

bool UICore::savePLCScript( const String &script )
{
    if ( !b_FSOpen || !script.length() ) //must have some length and FS must be initialized
        return false;

    if ( SPIFFS.exists(file_Script) )
        SPIFFS.remove(file_Script);

    File scriptFile = SPIFFS.open(file_Script, FILE_WRITE); //create a new file
    if (!scriptFile)
    {
        sendMessage(err_Script, PRIORITY_HIGH );
        return false;
    }

    scriptFile.print(script); //just print the entire script as given.
    scriptFile.close(); //close the file;
    sendMessage(succ_Script);
    return true;
}

bool UICore::saveSettings()
{
    if ( !b_FSOpen )
        return false;

    if ( SPIFFS.exists(file_Configuration)) 
        SPIFFS.remove(file_Configuration); //remove if possible

    File settingsFile = SPIFFS.open(file_Configuration, FILE_WRITE);
    if (!settingsFile)
    {
        sendMessage(err_Config, PRIORITY_HIGH );
        return false;
    }

    generateSettingsMap(); //create our temporary list of settings 

    for ( settings_itr = settingsMap.begin(); settings_itr != settingsMap.end(); settings_itr++ )
    {
        String settingValue = settings_itr->second.get()->getValue<String>();
        settingsFile.print(settings_itr->first + CHAR_EQUALS + settingValue + CHAR_NEWLINE);
        #ifdef DEBUG
        Serial.println(settings_itr->first + CHAR_EQUALS + settingValue);
        #endif
    }

    clearSettingsMap(); //empty the map and delete our settings objects to save memory

    settingsFile.close(); //close the file
    sendMessage(succ_Config);
    return true;
}

bool UICore::saveWebStyleSheet( const String &sheet )
{
    if ( !b_FSOpen )
        return false;

    if ( SPIFFS.exists(file_Stylesheet) )
        SPIFFS.remove(file_Stylesheet);

    File styleFile = SPIFFS.open(file_Stylesheet, FILE_WRITE);
    if (!styleFile)
    {
        sendMessage(err_Style, PRIORITY_HIGH);
        return false;
    }

    styleFile.print(sheet); //Write the data to the file.
    styleFile.close();
    sendMessage(succ_Style, PRIORITY_HIGH);
    return true;
}