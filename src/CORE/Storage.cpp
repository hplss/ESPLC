#include "UICore.h"
#include "GlobalDefs.h"

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

void Device_Setting::setSettingValue( const String &str )
{
    switch(getType())
    {
        case SETTING_TYPE::TYPE_BOOL:
        {
            getBOOL() = bool(str);
        }
        break;
        case SETTING_TYPE::TYPE_STRING:
        {
            getSTRING() = str;
        }
        break;
        case SETTING_TYPE::TYPE_UINT8:
        {
            getUINT8() = parseInt(str); //we can only assume it won't overflow
        }
        break;
        case SETTING_TYPE::TYPE_UINT:
        {
            getUINT() = parseInt(str); //we can only assume it won't overflow
        }
        break;
    }
}

String Device_Setting::getSettingValue()
{
    switch(getType())
    {
        case SETTING_TYPE::TYPE_BOOL:
        {
            return String(getBOOL());
        }
        break;
        case SETTING_TYPE::TYPE_STRING:
        {
            return getSTRING();
        }
        break;
        case SETTING_TYPE::TYPE_UINT8:
        {
            return String(getUINT8());
        }
        break;
        case SETTING_TYPE::TYPE_UINT:
        {
            return String(getUINT());
        }
        break;
    }

    return "";
}

void UICore::generateSettingsMap()
{
    //Device specific settings
    settingsMap.emplace(PSTR("dev_id"), make_shared<Device_Setting>( &getUniqueID() ) );
    settingsMap.emplace(PSTR("dev_serial_v"), make_shared<Device_Setting>( &i_verboseMode) );

    //security related settings
    settingsMap.emplace(PSTR("bt_en"), make_shared<Device_Setting>( &b_enableBT) );
    settingsMap.emplace(PSTR("bt_pwd"), make_shared<Device_Setting>( &getBTPWD() ) );
    settingsMap.emplace(PSTR("ui_uname"), make_shared<Device_Setting>( &getLoginName() ) );
    settingsMap.emplace(PSTR("ui_pwd"), make_shared<Device_Setting>( &getLoginPWD() ) );

    //Network settings
    settingsMap.emplace(PSTR("net_ap_en"), make_shared<Device_Setting>( &b_enableAP) );
    settingsMap.emplace(PSTR("net_retry"), make_shared<Device_Setting>( &b_autoRetryConnection) );
    settingsMap.emplace(PSTR("net_max_retries"), make_shared<Device_Setting>( &i_timeoutLimit) );
    settingsMap.emplace(PSTR("net_ssid"), make_shared<Device_Setting>( &getWiFiSSID() ) );
    settingsMap.emplace(PSTR("net_pwd"), make_shared<Device_Setting>( &getWiFiPWD() ) );
    settingsMap.emplace(PSTR("net_hostname"), make_shared<Device_Setting>( &getWiFiHostname() ) );
    settingsMap.emplace(PSTR("dns_en"), make_shared<Device_Setting>( &b_enableDNS ) );
    settingsMap.emplace(PSTR("dns_hostname"), make_shared<Device_Setting>( &getDNSHostname() ) );

    //Time Settings
	settingsMap.emplace(PSTR("time_en"), make_shared<Device_Setting>( &b_enableNIST) );
	settingsMap.emplace(PSTR("time_server"), make_shared<Device_Setting>( &getNISTServer() ) );
    settingsMap.emplace(PSTR("time_port"), make_shared<Device_Setting>( &i_NISTPort) );
    settingsMap.emplace(PSTR("time_upd_freq"), make_shared<Device_Setting>( &i_NISTupdateFreq) );
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
                settings_itr->second.get()->setSettingValue(settingValue);
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
    if ( !b_FSOpen || script.length() ) //must have some length and FS must be initialized
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
        String settingValue = settings_itr->second.get()->getSettingValue();
        settingsFile.print(settings_itr->first + CHAR_EQUALS + settingValue + CHAR_NEWLINE);
        #ifdef DEBUG
        Serial.println(settings_itr->first + CHAR_EQUALS + settingValue);
        #endif
    }

    clearSettingsMap(); //empty the map and delete our settings objects

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