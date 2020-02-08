/*
 * page_admin.cpp
 *
 * Created: 2/12/2018 4:04:47 PM
 *  Author: Andrew
 * The purpose of this file is to house the HTML generator for the admin page and related functions.
 */

#include <CORE/UICore.h>
 
extern UICore Core; //Accessor for the initialized UIcore object.

void UICore::createAdminFields() //Should never be called more than once
{
	shared_ptr<DataTable> deviceTable( new DataTable( PSTR("Device Specific Settings") ) );
	shared_ptr<DataTable> securityTable( new DataTable( PSTR("System Security") ) );
	shared_ptr<DataTable> networkTable( new DataTable( PSTR("Wifi Network Settings") ) );
	shared_ptr<DataTable> timeTable( new DataTable( PSTR("System Time Settings") ) );
	shared_ptr<DataTable> saveTable( new DataTable() );

	uint8_t index = 1;
	
	//DataField *resourceField = new DataField( 2, TYPE_INPUT_CHECKBOX, "Verbose Mode (Serial)", "verbose_serial", "checked" );
	//Device specific setings
	deviceTable->AddElement( make_shared<STRING_Datafield>( s_uniqueID, index++, FIELD_TYPE::TEXT, PSTR("Device Unique ID") ) ); 
	deviceTable->AddElement( make_shared<UINT8_Datafield>( &i_verboseMode, index++, FIELD_TYPE::TEXT, PSTR("Verbosity Setting (Serial)") ) );
	deviceTable->AddElement( make_shared<UINT8_Datafield>( &i_verboseMode, index++, FIELD_TYPE::TEXT, PSTR("Verbosity Setting (Web UI)") ) );
	//
	/*wifi_config_t conf;
    esp_wifi_get_config(WIFI_IF_STA, &conf); */

	//Security related settings
	securityTable->AddElement( make_shared<STRING_Datafield>( s_authenName, index++, FIELD_TYPE::TEXT, PSTR("Admin Username") ) );
	securityTable->AddElement( make_shared<STRING_Datafield>( s_authenPWD, index++, FIELD_TYPE::PASSWORD, PSTR("Admin Password") ) );
	securityTable->AddElement( make_shared<BOOL_Datafield>( &b_enableBT, index++, FIELD_TYPE::CHECKBOX, PSTR("Enable Bluetooth Interface") ) );
	securityTable->AddElement( make_shared<STRING_Datafield>( s_BTPWD, index++, FIELD_TYPE::PASSWORD, PSTR("Bluetooth Password") ) );
	//

	//Network Table Stuff
	networkTable->AddElement( make_shared<BOOL_Datafield>( &b_enableAP, index++, FIELD_TYPE::CHECKBOX, PSTR("Enable Access Point Mode") ) ); //HACKHACK - Must be before SSID fields
	networkTable->AddElement( make_shared<STRING_Datafield>( s_WiFiSSID, index++, FIELD_TYPE::TEXT, PSTR("Access Point SSID") ) );
	networkTable->AddElement( make_shared<SSID_Datafield>(index++, PSTR("Station Network SSID") ) );
	networkTable->AddElement( make_shared<STRING_Datafield>( s_WiFiPWD, index++, FIELD_TYPE::PASSWORD, PSTR("Network Password (For AP or Station)") ) );
	networkTable->AddElement( make_shared<STRING_Datafield>( s_WiFiHostname, index++, FIELD_TYPE::TEXT, PSTR("Network Hostname") ) );
	networkTable->AddElement( make_shared<UINT8_Datafield>( &i_timeoutLimit, index++, FIELD_TYPE::TEXT, PSTR("Connection Retry Limit") ) );
	networkTable->AddElement( make_shared<BOOL_Datafield>( &b_autoRetryConnection, index++, FIELD_TYPE::CHECKBOX, PSTR("Auto Retry On Disconnect (Station)") ) );
	networkTable->AddElement( make_shared<BOOL_Datafield>( &b_enableDNS, index++, FIELD_TYPE::CHECKBOX, PSTR("Enable DNS Server") ) );
	networkTable->AddElement( make_shared<STRING_Datafield>( s_DNSHostname, index++, FIELD_TYPE::TEXT, PSTR("DNS Hostname" ) ) );
	//
	
	//Time table stuff
	timeTable->AddElement( make_shared<BOOL_Datafield>( &b_enableNIST, index++, FIELD_TYPE::CHECKBOX, PSTR("Enable NIST Time Updating (Requires internet connection)") ) );
	timeTable->AddElement( make_shared<STRING_Datafield>( s_NISTServer, index++, FIELD_TYPE::TEXT, PSTR("NIST Time Update Server") ) );
	timeTable->AddElement( make_shared<UINT_Datafield>( &i_NISTPort, index++, FIELD_TYPE::TEXT, PSTR("Time Server Port") ) );
	timeTable->AddElement( make_shared<UINT_Datafield>( &i_NISTupdateFreq, index++, FIELD_TYPE::TEXT, PSTR("NIST Time Update Frequency") ) );
	//
	
	//Save Table Stuff
	saveTable->AddElement( make_shared<BOOL_S_Datafield>( &UICore::applyDeviceSettings, &b_SaveConfig, index++, FIELD_TYPE::CHECKBOX, PSTR("Save As Defaults"), false ) );
	saveTable->AddElement( make_shared<DataField>(index++, FIELD_TYPE::SUBMIT, PSTR("Apply Settings") ) );
	//
	
	//Add to our list of config tables
	p_UIDataTables.push_back( deviceTable );
	p_UIDataTables.push_back( securityTable );
	p_UIDataTables.push_back( networkTable );
	p_UIDataTables.push_back( timeTable );
	p_UIDataTables.push_back( saveTable );
	//
} 

void UICore::HandleAdmin()
{
	if (!handleAuthorization())
		return;

	createAdminFields();

	String AlertHTML;
	if ( p_server->args() ) //Do we have some args to input? Apply settings if so (before generating the rest of the HTML)
		UpdateWebFields( p_UIDataTables, AlertHTML );
	
	String HTML = generateHeader();
	HTML += generateTitle(PSTR("Admin Page"));
	HTML += AlertHTML; //Any alerts? MAY REWORK THIS LATER
	
	HTML += form_Begin + adminDir + form_Middle; //Construct the form entry
	
	for ( uint8_t x = 0; x < p_UIDataTables.size(); x++ )
		HTML += p_UIDataTables[x]->GenerateTableHTML(); //Add each datafield table to the HTML body
	
	HTML += form_End;
	HTML += generateFooter(); //Add the footer stuff.
	p_server->send(200, transmission_HTML, HTML );
	p_UIDataTables.clear(); //empty the data table
}

void UICore::applyDeviceSettings()
{
	if ( Core.b_SaveConfig )
	{
		Core.saveSettings();
		Core.b_SaveConfig = false;
	}
	Core.applySettings(); //apply the updated settings to our system
}