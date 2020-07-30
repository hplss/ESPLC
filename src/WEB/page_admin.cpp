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

	uint8_t index = 1;
	/*
	Future JSON format instead of HTML generation? Just an idea for now.
	label :
	id : <For post methods?>
	val : <Multiple args separated by commas?>
	type : <For checkboxes, text, etc>
	*/

	alertsTable->AddElement( make_shared<VAR_Datafield>( make_shared<String>(""), index++, FIELD_TYPE::TEXTAREA, field_title_alerts, 50, 3) );
	
	//Device specific setings
	deviceTable->AddElement( make_shared<VAR_Datafield>( s_uniqueID, index++, FIELD_TYPE::TEXT, PSTR("Device Unique ID") ) ); 
	shared_ptr<Select_Datafield> verbosityMode( new Select_Datafield( &i_verboseMode, index++, PSTR("Alert Verbosity Setting") ) );
	verbosityMode->addOption(PSTR("Disabled"));
	verbosityMode->addOption(PSTR("High Priority Only"));
	verbosityMode->addOption(PSTR("All Priorities"));
	deviceTable->AddElement(verbosityMode);
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
	networkTable->AddElement( make_shared<VAR_Datafield>( s_WiFiSSID, index++, FIELD_TYPE::TEXT, PSTR("Access Point SSID") ) );
	networkTable->AddElement( make_shared<SSID_Datafield>(index++, PSTR("Station Network SSID") ) );
	networkTable->AddElement( make_shared<VAR_Datafield>( s_WiFiPWD, index++, FIELD_TYPE::PASSWORD, PSTR("Network Password (For AP or Station)") ) );
	networkTable->AddElement( make_shared<VAR_Datafield>( s_WiFiHostname, index++, FIELD_TYPE::TEXT, PSTR("Network Hostname") ) );
	networkTable->AddElement( make_shared<VAR_Datafield>( &i_timeoutLimit, index++, FIELD_TYPE::NUMBER, PSTR("Connection Retry Limit"), 2 ) );
	networkTable->AddElement( make_shared<VAR_Datafield>( &b_autoRetryConnection, index++, FIELD_TYPE::CHECKBOX, PSTR("Auto Retry On Disconnect (Station)") ) );
	networkTable->AddElement( make_shared<VAR_Datafield>( &b_enableDNS, index++, FIELD_TYPE::CHECKBOX, PSTR("Enable DNS Server") ) );
	networkTable->AddElement( make_shared<VAR_Datafield>( s_DNSHostname, index++, FIELD_TYPE::TEXT, PSTR("DNS Hostname" ) ) );
	//

	//Remote control settings for external ESPLC devices
	shared_ptr<Select_Datafield> remoteNetmode( new Select_Datafield( &i_plc_netmode, index++, PSTR("PLC Net Modes") ) );
	remoteNetmode->addOption(PSTR("Disabled"));
	remoteNetmode->addOption(PSTR("IO Expander"));
	remoteNetmode->addOption(PSTR("Cluster"));
	remotePLCTable->AddElement( remoteNetmode );
	remotePLCTable->AddElement( make_shared<VAR_Datafield>( &i_plc_broadcast_port, index++, FIELD_TYPE::NUMBER, PSTR("Update Broadcast Port (Local)"), 5 ) );
	remotePLCTable->AddElement( make_shared<VAR_Datafield>( s_plc_ip_ranges, index++, FIELD_TYPE::TEXT, PSTR("IP Range Limiter: LOW:HIGH (0-255)"), 5, 1, false ) );
	remotePLCTable->AddElement( make_shared<VAR_Datafield>( s_plc_port_ranges, index++, FIELD_TYPE::TEXT, PSTR("Port Range Limiter: LOW:HIGH (0-65536)"), 5 ) );
	remotePLCTable->AddElement( make_shared<VAR_Datafield>( &b_plc_autoconnect, index++, FIELD_TYPE::CHECKBOX, PSTR("Auto-connect to External Devices") ) );
	remotePLCTable->AddElement( make_shared<VAR_Datafield>( s_plc_addresses, index++, FIELD_TYPE::TEXTAREA, PSTR("External Device IPs (for Auto-connection)"), 30, 2 ) );

	//Time table stuff
	timeTable->AddElement( make_shared<VAR_Datafield>( &b_enableNIST, index++, FIELD_TYPE::CHECKBOX, PSTR("Enable NIST Time Updating (Requires internet connection)") ) );
	timeTable->AddElement( make_shared<VAR_Datafield>( s_NISTServer, index++, FIELD_TYPE::TEXT, PSTR("NIST Time Update Server") ) );
	timeTable->AddElement( make_shared<VAR_Datafield>( &i_NISTPort, index++, FIELD_TYPE::NUMBER, PSTR("Time Server Port"), 5 ) );
	timeTable->AddElement( make_shared<VAR_Datafield>( &i_NISTupdateFreq, index++, FIELD_TYPE::NUMBER, PSTR("NIST Time Update Frequency"), 5, 1, false ) );
	shared_ptr<Select_Datafield> updateUnits( new Select_Datafield( &i_NISTUpdateUnit, index++, "" ) );
	updateUnits->addOption(PSTR("Second(s)"));
	updateUnits->addOption(PSTR("Minute(s)"));
	updateUnits->addOption(PSTR("Hour(s)"));
	updateUnits->addOption(PSTR("Day(s)"));
	updateUnits->addOption(PSTR("Month(s)"));
	updateUnits->addOption(PSTR("Year(s)"));
	timeTable->AddElement(updateUnits);
	timeTable->AddElement( make_shared<VAR_Datafield>( s_TimeString, index++, FIELD_TYPE::TEXT, PSTR("Current System Time (YY:MM:DD:HR:MN:SE)") ) );
	//
	
	//Save Table Stuff
	saveTable->AddElement( make_shared<VAR_S_Datafield>( &UICore::applyDeviceSettings, &b_SaveConfig, index++, FIELD_TYPE::CHECKBOX, PSTR("Save As Defaults"), false ) );
	saveTable->AddElement( make_shared<DataField>(index++, FIELD_TYPE::SUBMIT, PSTR("Apply Settings") ) );
	//
	
	//Add to our list of config tables
	p_UIDataTables.push_back( alertsTable );
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

	if ( p_server->args() ) //Do we have some args to input? Apply settings if so (before generating the rest of the HTML)
		UpdateWebFields( p_UIDataTables );
	
	String HTML = generateHeader();
	HTML += generateTitle(PSTR("Admin Page"));
	HTML += generateAlertsScript( 1 ); //hackhack for now -- index may vary, unless explicitly assigned to '1'
	
	HTML += html_form_Begin + adminDir + html_form_Middle; //Construct the form entry
	
	for ( uint8_t x = 0; x < p_UIDataTables.size(); x++ )
		HTML += p_UIDataTables[x]->GenerateTableHTML(); //Add each datafield table to the HTML body
	
	HTML += html_form_End;
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