/* 
Author: Andrew Ward
Date: 9/5/2020
Description: The purpose of web_core.cpp is to serve as a common container for any webUI-related functions defined for the UICore object that are 
 not directly related to individual page functionality. 
*/

#include "../Core/UICore.h"
#include "../Core/GlobalDefs.h"
#include <Update.h>

const String &HTML_HEADER_INITIAL PROGMEM = PSTR(
"<!DOCTYPE HTML>"
"<html>"
"<head>"
"<meta name = \"viewport\" content = \"width = device-width, initial-scale = 1.0, maximum-scale = 1.0, user-scalable=0\">"
"<script src=\"https://ajax.googleapis.com/ajax/libs/jquery/3.5.1/jquery.min.js\"></script>"
"<style>"),

	&HTML_HEADER_LAST PROGMEM = PSTR( 
"</style>"
"</head>"
"<body>"),

	&HTML_FOOTER PROGMEM = PSTR(
"</body>"
"</html>");

void UICore::setupServer()
{
	p_server = make_shared<WebServer>(80); //Open on port 80 (http)
	//These set up our page triggers, linking them to specific functions.
	getWebServer().on(styleDir, std::bind(&UICore::handleStyleSheet, this) );
	getWebServer().on(PSTR("/"), std::bind(&UICore::handleIndex, this) );
	getWebServer().on(updateDir, std::bind(&UICore::handleUpdateStatus, this) );
	getWebServer().on(adminDir, std::bind(&UICore::handleAdmin, this) );
	getWebServer().on(scriptDir, std::bind(&UICore::handleScript, this) );
	getWebServer().on(statusDir, std::bind(&UICore::handleStatus, this) );
	getWebServer().on(alertsDir, std::bind(&UICore::handleAlerts, this) );
    getWebServer().on(firmwareDir, HTTP_GET, std::bind(&UICore::handleUpdater, this) );
    getWebServer().on(firmwareDir, HTTP_POST, [](){}, applyRemoteFirmwareUpdate ); //continuously call the firmware update function on HTTP POST method
	//
};

bool UICore::handleAuthorization()
{
	if ( getLoginName().length() > 1 && getLoginPWD().length() > 1 )
	{
		if (!getWebServer().authenticate(getLoginName().c_str(), getLoginPWD().c_str()) )
			getWebServer().requestAuthentication(DIGEST_AUTH, String(getUniqueID() + PSTR(" Login")).c_str(), PSTR("Authentication Failed.") );
	}

	return true;
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

String UICore::generateAlertsScript( uint8_t fieldID )
{ 
	return PSTR("<script>var intFunc = function(){\n var xml = new XMLHttpRequest();\n xml.onreadystatechange = function(){\n if (this.readyState == 4 && this.status == 200){parse(this.responseText);};};\n xml.open(\"GET\", \"alerts\");\n xml.send(); };\n function parse(arr){ var doc = document.getElementById(\"1\"); doc.innerHTML = arr; };\nsetInterval(intFunc,500);</script>");
}

String UICore::generateAlertsJSON()
{
	String JSON = "";
	for( size_t x = 0; x < alerts.size(); x++ )
		JSON += alerts[x] + PSTR("\n");

    return JSON;
}

void UICore::handleAlerts()
{
	//generate the JSON and send it off to the client.
    getWebServer().sendHeader(http_header_connection, http_header_close);
	getWebServer().send(200, transmission_HTML, generateAlertsJSON() ); //And we're off.
}

void UICore::UpdateWebFields( const vector<shared_ptr<DataTable>> &tables )
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
            {
                tempField->SetFieldValue();
            }
		}
		//
		//Basically, at this point, we need to sort it so that all data fields that call functions directly are updated last. 
		//The purpose of this is to ensure any modified variables in other fields can be used in function calls.
		//Idea: use some of the code below to build a list of objects that are being modified by args, then build a vector of all that aren't function callers,
		//Then add those to the end.
		
		for ( uint8_t x = 0; x < getWebServer().args(); x++ ) // For each arg...
		{
			tempField = tables[i]->GetElementByName( getWebServer().argName(x) );
			if ( tempField ) //Found an element with this name?
			{
				if ( tempField->GetFieldValue() == getWebServer().arg(x) ) //Is the arg the same as the existing setting?
					continue; //Skip if so
				
				//build a map of function data fields to update last, update all others as they come
				if ( tempField->UsesFunction() ) //All function related fields should go here
				{
					functionFields.emplace(tempField, getWebServer().arg(x)); //place into the map for fields that execute functions
				}
				else
				{
					if ( !tempField->SetFieldValue( getWebServer().arg(x) ) )
						sendMessage( PSTR("Update of '") + tempField->GetFieldLabel() + PSTR("' failed.") );
				}
			}
		}
	}

	//Finally, update the fields as necessary, in the proper order
	for ( std::map<shared_ptr<DataField>, String>::iterator itr = functionFields.begin(); itr != functionFields.end(); itr++ )//handle each object attached to this object.
	{
		if( !itr->first->SetFieldValue( itr->second ) )
			sendMessage( PSTR("Update of '") + itr->first->GetFieldLabel() + PSTR("' failed.") );
	}
}

void UICore::resestFieldContainers()
{
    p_UIDataTables.clear();
    p_StaticDataTables.clear();
}