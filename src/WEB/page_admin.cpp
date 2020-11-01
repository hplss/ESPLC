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
	shared_ptr<DataTable> alertsTable( new DataTable( table_title_messages ) );
	shared_ptr<DataTable> deviceTable( new DataTable( PSTR("Device Specific Settings") ) );
	shared_ptr<DataTable> securityTable( new DataTable( PSTR("System Security") ) );
	shared_ptr<DataTable> networkTable( new DataTable( PSTR("Wifi Network Settings") ) );
	shared_ptr<DataTable> remotePLCTable( new DataTable( PSTR("Remote ESPLC Control Settings") ) );
	shared_ptr<DataTable> timeTable( new DataTable( PSTR("System Time Settings") ) );
	shared_ptr<DataTable> saveTable( new DataTable() );

	uint8_t index = 2;
	/*
	Future JSON format instead of HTML generation? Just an idea for now.
	label :
	id : <For post methods?>
	val : <Multiple args separated by commas?>
	type : <For checkboxes, text, etc>
	*/

	alertsTable->AddElement( make_shared<Hyperlink_Datafield>( index++, PSTR("Back to Index"), "/" ) );
	alertsTable->AddElement( make_shared<DataField>( make_shared<String>(""), 1, FIELD_TYPE::TEXTAREA, field_title_alerts, vector<String>{}, ALERTS_FIELD_COLS, ALERTS_FIELD_ROWS) );
	
	//Device specific setings
	deviceTable->AddElement( make_shared<VAR_Datafield>( s_uniqueID, index++, FIELD_TYPE::TEXT, PSTR("Device Unique ID") ) ); 
	deviceTable->AddElement( make_shared<Select_Datafield>( &i_verboseMode, index++, PSTR("Alert Verbosity Setting"), vector<String>{PSTR("Disabled"), PSTR("High Priority Only"), PSTR("All Priorities") } ) );
	//
	/*wifi_config_t conf;
    esp_wifi_get_config(WIFI_IF_STA, &conf); */

	//Security related settings
	securityTable->AddElement( make_shared<VAR_Datafield>( s_authenName, index++, FIELD_TYPE::TEXT, PSTR("Admin Username") ) );
	securityTable->AddElement( make_shared<VAR_Datafield>( s_authenPWD, index++, FIELD_TYPE::PASSWORD, PSTR("Admin Password") ) );
	securityTable->AddElement( make_shared<VAR_Datafield>( &b_enableBT, index++, FIELD_TYPE::CHECKBOX, PSTR("Enable Bluetooth Interface") ) );
	securityTable->AddElement( make_shared<VAR_Datafield>( s_BTPWD, index++, FIELD_TYPE::PASSWORD, PSTR("Bluetooth Password") ) );
	//

	//Network Table Stuff
	networkTable->AddElement( make_shared<VAR_Datafield>( &b_enableAP, index++, FIELD_TYPE::CHECKBOX, PSTR("Enable Access Point Mode") ) ); //HACKHACK - Must be before SSID fields
	networkTable->AddElement( make_shared<VAR_Datafield>( s_WiFiAPSSID, index++, FIELD_TYPE::TEXT, PSTR("Access Point SSID") ) );
	networkTable->AddElement( make_shared<VAR_Datafield>( s_WiFiAPPWD, index++, FIELD_TYPE::PASSWORD, PSTR("Access Point Password") ) );
	networkTable->AddElement( make_shared<SSID_Datafield>( s_WiFiSSID, index++, PSTR("Wi-Fi Network SSID") ) );
	networkTable->AddElement( make_shared<VAR_Datafield>( s_WiFiPWD, index++, FIELD_TYPE::PASSWORD, PSTR("Wi-Fi Network Password") ) );
	networkTable->AddElement( make_shared<VAR_Datafield>( s_WiFiHostname, index++, FIELD_TYPE::TEXT, PSTR("Network Hostname") ) );
	networkTable->AddElement( make_shared<VAR_Datafield>( &i_connectionRetries, index++, FIELD_TYPE::NUMBER, PSTR("Station Connection Retry Count"), vector<String>{}, 1 ) );
	networkTable->AddElement( make_shared<VAR_Datafield>( &i_timeoutLimit, index++, FIELD_TYPE::NUMBER, PSTR("Station Connection Attempt Timeout (seconds)"), vector<String>{}, 2 ) );
	networkTable->AddElement( make_shared<VAR_Datafield>( &b_autoRetryConnection, index++, FIELD_TYPE::CHECKBOX, PSTR("Auto Retry On Disconnect (Station)") ) );
	networkTable->AddElement( make_shared<VAR_Datafield>( &b_enableDNS, index++, FIELD_TYPE::CHECKBOX, PSTR("Enable DNS Server") ) );
	networkTable->AddElement( make_shared<VAR_Datafield>( s_DNSHostname, index++, FIELD_TYPE::TEXT, PSTR("DNS Hostname" ) ) );
	//

	//Remote control settings for external ESPLC devices
	remotePLCTable->AddElement( make_shared<Select_Datafield>( &i_plc_netmode, index++, PSTR("PLC Net Modes"), vector<String>{ PSTR("Disabled"), PSTR("IO Expander"), PSTR("Cluster") } ) );
	remotePLCTable->AddElement( make_shared<VAR_Datafield>( &i_plc_broadcast_port, index++, FIELD_TYPE::NUMBER, PSTR("Update Broadcast Port (Local)"), vector<String>{}, 5 ) );

	//Time table stuff
	timeTable->AddElement( make_shared<VAR_Datafield>( &b_enableNIST, index++, FIELD_TYPE::CHECKBOX, PSTR("Enable NIST Time Updating (Requires internet connection)") ) );
	timeTable->AddElement( make_shared<VAR_Datafield>( s_NISTServer, index++, FIELD_TYPE::TEXT, PSTR("NIST Time Update Server") ) );
	timeTable->AddElement( make_shared<VAR_Datafield>( &i_NISTPort, index++, FIELD_TYPE::NUMBER, PSTR("Time Server Port"), vector<String>{}, 5 ) );
	timeTable->AddElement( make_shared<VAR_Datafield>( &i_NISTupdateFreq, index++, FIELD_TYPE::NUMBER, PSTR("NIST Time Update Frequency"), vector<String>{}, 5, 1, false ) );
	timeTable->AddElement( make_shared<Select_Datafield>( &i_NISTUpdateUnit, index++, "", vector<String>{ PSTR("Second(s)"), PSTR("Minute(s)"), PSTR("Hour(s)"), PSTR("Day(s)"), PSTR("Month(s)"), PSTR("Year(s)") } ) );
	//timeTable->AddElement( make_shared<VAR_Datafield>( p_currentTime->GetTimeStr(), index++, FIELD_TYPE::TEXT, PSTR("Current System Time (YY:MM:DD:HR:MN:SE)") ) );
	//
	
	//Save Table Stuff
	saveTable->AddElement( make_shared<VAR_S_Datafield>( &UICore::applyDeviceSettings, &b_SaveConfig, index++, FIELD_TYPE::CHECKBOX, PSTR("Save As Defaults"), vector<String>{}, 12, 1, true ) );
	saveTable->AddElement( make_shared<DataField>(index++, FIELD_TYPE::SUBMIT, PSTR("Apply Settings") ) );
	//
	
	//Add to our list of config tables
	p_StaticDataTables.push_back( alertsTable );
	p_UIDataTables.push_back( deviceTable );
	p_UIDataTables.push_back( securityTable );
	p_UIDataTables.push_back( networkTable );
	p_UIDataTables.push_back( remotePLCTable );
	p_UIDataTables.push_back( timeTable );
	p_UIDataTables.push_back( saveTable );
	//
} 

void UICore::handleAdmin()
{
	if (!handleAuthorization())
		return;

	createAdminFields();//generate the HTML based on the fields listed above.

	if ( getWebServer().args() ) //Do we have some args to input? Apply settings if so (before generating the rest of the HTML)
		UpdateWebFields( p_UIDataTables );
	
	String HTML = generateHeader();
	HTML += generateTitle(PSTR("Admin Page"));
	HTML += generateAlertsScript( 1 ); //hackhack for now -- index may vary, unless explicitly assigned to '1'

	for ( uint8_t x = 0; x < p_StaticDataTables.size(); x++ ) //Generate static objects that are not included in the FORM
        HTML += p_StaticDataTables[x]->GenerateTableHTML();
	
	HTML += html_form_Begin + adminDir + html_form_Middle; //Construct the form entry
	
	for ( uint8_t x = 0; x < p_UIDataTables.size(); x++ )
		HTML += p_UIDataTables[x]->GenerateTableHTML(); //Add each datafield table to the HTML body
	
	HTML += html_form_End;
	HTML += generateFooter(); //Add the footer stuff.
	getWebServer().sendHeader(http_header_connection, http_header_close);
	getWebServer().send(200, transmission_HTML, HTML );
	resestFieldContainers();
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