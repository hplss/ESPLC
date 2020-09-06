/*
 * page_status.cpp
 *
 * Created: 7/19/2020 2:38:10 PM
 *  Author: Andrew Ward
 * The purpose of this file is to house the HTML generator for the PLC Ladder Object status page.
 * This allows a user to easily monitor the status of current initialized ladder logic objects, including individual bits (accumulator/count values, enable bits, etc.)
 */ 
#include <CORE/UICore.h>

#include <PLC/PLC_Main.h>

extern PLC_Main PLCObj;
//Each object should have its ID, TYPE, and any related values listed into a table.

void UICore::createStatusFields()
{
    shared_ptr<DataTable> alertsTable( new DataTable( table_title_messages ) );
    shared_ptr<DataTable> localStatusTable( new DataTable( F("Local Objects") ) );

    uint8_t index = 2;
    alertsTable->AddElement( make_shared<Hyperlink_Datafield>( index++, PSTR("Back to Index"), "/" ) );
    alertsTable->AddElement( make_shared<VAR_Datafield>( make_shared<String>(""), 1, FIELD_TYPE::TEXTAREA, field_title_alerts, vector<String>{}, ALERTS_FIELD_COLS, ALERTS_FIELD_ROWS) );
    //Should display the IP/Hostname of the device that is hosting the object, as well as the info listed for local objects.
    shared_ptr<DataTable> remoteStatusTable(new DataTable( F("Remote Objects") ) ); 

    p_StaticDataTables.push_back(alertsTable);
    p_UIDataTables.push_back(remoteStatusTable);
	  p_UIDataTables.push_back(localStatusTable);
}

void UICore::handleStatus()
{
    if (!handleAuthorization()) //make sure to have the uder log in first.
		  return;

	  createStatusFields();

    if ( getWebServer().args() ) //Do we have some args to input? Apply settings if so (before generating the rest of the HTML)
	    UpdateWebFields( p_UIDataTables );
	
	  String HTML = generateHeader();
	  HTML += generateTitle(PSTR("Status Page"));
    HTML += generateAlertsScript( 1 ); //hackhack for now -- index may vary, unless explicitly assigned to '1'
    HTML += generateStatusScript(); //Currently not used
	
    for ( uint8_t x = 0; x < p_StaticDataTables.size(); x++ ) //Generate static objects that are not included in the FORM
        HTML += p_StaticDataTables[x]->GenerateTableHTML();

	  HTML += html_form_Begin + statusDir + html_form_Middle; //Construct the form entry
	  for ( uint8_t x = 0; x < p_UIDataTables.size(); x++ )
		  HTML += p_UIDataTables[x]->GenerateTableHTML(); //Add each datafield table to the HTML body
	
	  HTML += html_form_End;
	  HTML += generateFooter(); //Add the footer stuff.
    getWebServer().sendHeader(http_header_connection, http_header_close);
	  getWebServer().send(200, transmission_HTML, HTML );

	  resestFieldContainers(); //empty the data table to free memory
}

String UICore::generateStatusJSON()
{
    String JSON = "";
    return JSON;
}

String UICore::generateStatusScript()
{
  return "";
}
/*
<script>
var xmlhttp = new XMLHttpRequest();
var url = "myTutorials.txt";

xmlhttp.onreadystatechange = function() {
  if (this.readyState == 4 && this.status == 200) {
    myFunction(JSON.parse(this.responseText));
  }
};
xmlhttp.open("GET", url, true);
xmlhttp.send();

function myFunction(arr) {
  var out = "";
  var i;
  for(i = 0; i < arr.length; i++) {
    out = arr[i].display;
    var element = document.getElementById("test" + i);
    if (element)
    {
    	element.innerHTML = out;
    }
  }
}*/