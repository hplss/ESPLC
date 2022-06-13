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

//Each object should have its ID, TYPE, and any related values listed into a table.

void UICore::createStatusFields()
{
    uint8_t index = 2;
    for (uint16_t x = 0; x < PLCObj.getLadderObjects().size(); x++ )
    {
      OBJ_LOGIC_PTR ptr = PLCObj.getLadderObjects()[x];
      shared_ptr<DataTable> newTable( new DataTable( ptr->sObjID ) );
      newTable->AddElement( make_shared<LADDER_OBJ_Datafield>(ptr, index++) );
      p_UIDataTables.push_back( newTable );
    }
    shared_ptr<DataTable> alertsTable( new DataTable( table_title_messages ) );

    
    alertsTable->AddElement( make_shared<Hyperlink_Datafield>( index++, PSTR("Back to Index"), "/" ) );
    alertsTable->AddElement( make_shared<VAR_Datafield>( make_shared<String>(""), 1, FIELD_TYPE::TEXTAREA, field_title_alerts, vector<String>{}, ALERTS_FIELD_COLS, ALERTS_FIELD_ROWS) );
    //Should display the IP/Hostname of the device that is hosting the object, as well as the info listed for local objects.

    p_StaticDataTables.push_back(alertsTable);
}

void UICore::handleStatus()
{
    if (!handleAuthorization()) //make sure to have the uder log in first.
		  return;

	  createStatusFields();

    if ( getWebServer().args() ) //Do we have some args to input? Apply settings if so (before generating the rest of the HTML)
	    UpdateWebFields( p_UIDataTables );
	
    String HTML;
    HTML.reserve(8192);
    
    HTML += generateHeader();
    HTML += generateTitle(PSTR("Status Page"));
    //HTML += generateAlertsScript( 1 ); //hackhack for now -- index may vary, unless explicitly assigned to '1'
    HTML += generateStatusScript(); //Currently used
	
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

void UICore::handleUpdateStatus()
{
    String JSON = PSTR("{\"Status\":[\n");
    uint16_t numObjects = PLCObj.getLadderObjects().size();

    for (uint16_t i = 0; i < numObjects; i++)
    {
        OBJ_LOGIC_PTR objPtr = PLCObj.getLadderObjects()[i];
        const OBJ_TYPE type = objPtr->iType;

        if ( type >= OBJ_TYPE::TYPE_VAR_UBYTE && type <= OBJ_TYPE::TYPE_VAR_STRING ) //if it's a variable type..
        {
            JSON += PSTR("{\"ID\":\"") + objPtr->sObjID + objPtr->sObjID + PSTR("\", \"Status\":\"") + static_pointer_cast<Ladder_VAR>(objPtr)->getValueStr() + "\"}";

            if (i != (numObjects - 1))
                JSON += ",\n";
            else
            {
                JSON += CHAR_NEWLINE;
            }
        }
        else
        {
            for (uint8_t j = 0; j < objPtr->getObjectVARs().size(); j++)
            {
                VAR_PTR varPtr = objPtr->getObjectVARs()[j];

                JSON += PSTR("{\"ID\":\"") + objPtr->sObjID + varPtr->sObjID + PSTR("\", \"Status\":\"") + varPtr->getValueStr() + "\"}";
                
                if ( i >= (numObjects - 1) && j >= (objPtr->getObjectVARs().size() - 1) ) //are we at the end of the list?
                {
                    JSON += CHAR_NEWLINE;
                }
                else
                {
                    JSON += ",\n";
                }
            }
        }
        
    }
    JSON += "]}";
    getWebServer().send(200, transmission_Plain, JSON);
}

String UICore::generateStatusScript()
{
  const String script PROGMEM = PSTR("\n<script>"
                "setInterval(getObjectStatus, 500)\n"
                "function getObjectStatus()\n"
                "{\n"
                  //Alerts Start
                  "var xml = new XMLHttpRequest();\n"
                  "xml.open(\"GET\", \"alerts\");\n"
                  "xml.onreadystatechange = function()\n"
                  "{\n"
                      "if (this.readyState == 4 && this.status == 200)\n"
                      "{\n"
                          "parse(this.responseText);\n"
                          //Object Update Start
                          "var xml2 = new XMLHttpRequest();\n"
                          "xml2.onreadystatechange = function()\n"
                          "{\n"
                              "if (this.readyState == 4 && this.status == 200)\n"
                              "{\n"
                                  "obj_update(this.responseText);\n"
                              "}\n"
                          "}\n"
                              "xml2.open(\"GET\", \"update\");\n"
                              "xml2.send();\n"
                          
                      "}\n"
                  "}\n"
                      "xml.send();\n"
                  "}\n"

                  "function parse(arr)\n"
                  "{\n"
                    "var doc = document.getElementById(\"1\");\n"
                    "if(doc.innerHTML != arr)\n"// only scrolls to the bottom if data is added
                    "{\n"
                      "doc.innerHTML = arr\n"
                      "doc.scrollTop = doc.scrollHeight\n"
                    "}\n"
                "}\n"

                "function obj_update(data)\n"
                "{\n"
                  "var objData = JSON.parse(data)\n"
                  "for(var i = 0; i < objData.Status.length; i++)\n"
                  "{\n"
                    "document.getElementById(String(objData.Status[i].ID)).innerHTML = String(objData.Status[i].Status);\n"
                  "}\n"
                "}\n"
                "</script>\n");
  return script;
}