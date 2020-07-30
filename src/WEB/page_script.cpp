/*
 * page_script.cpp
 *
 * Created: 12/30/2019 4:05:10 PM
 *  Author: Andrew Ward
 * The purpose of this file is to house the HTML generator for the PLC logic script page and related functions.
 * Essentially, the user wil access this page to modify the logic script that the PLC routine processes and runs.
 * basically all that needs to be here is a text box that shows the current script running, and an apply button. 
 * Other options may be included, but should be limited.
 */ 


#include <CORE/UICore.h>

#include <PLC/PLC_Main.h>

extern PLC_Main PLCObj;
extern UICore Core;

void UICore::createScriptFields() //Probably shouldnt be called more than once.
{
	shared_ptr<DataTable> alertsTable( new DataTable( table_title_messages ) );
	shared_ptr<DataTable> logicTable( new DataTable( PSTR("PLC Logic Script") ) );
	shared_ptr<DataTable> saveTable( new DataTable() ); //used for settings aplication

	uint8_t index = 1;
	alertsTable->AddElement( make_shared<VAR_Datafield>( make_shared<String>(""), index++, FIELD_TYPE::TEXTAREA, field_title_alerts, 50, 3 ) );

	logicTable->AddElement( make_shared<VAR_S_Datafield>(UICore::applyLogic, PLCObj.getLogicScript(), index++, FIELD_TYPE::TEXTAREA, PSTR("Logic Script"), 50, 25 ) );
	logicTable->AddElement( make_shared<VAR_Datafield>( &b_SaveScript, index++, FIELD_TYPE::CHECKBOX, PSTR("Save Script in Flash RAM") ) );

	saveTable->AddElement( make_shared<DataField>(index++, FIELD_TYPE::SUBMIT, PSTR("Apply Settings") ) );

	p_UIDataTables.push_back(alertsTable);
	p_UIDataTables.push_back(logicTable);
	p_UIDataTables.push_back(saveTable);
}

void UICore::handleScript() //Generate the HTML for our main page.
{	  
	if (!handleAuthorization()) //make sure we have proper access to this page first.
		return;

	createScriptFields();
	if ( p_server->args() ) //Do we have some args to input? Apply settings if so (before generating the rest of the HTML)
		UpdateWebFields( p_UIDataTables );

	String HTML = generateHeader();
	HTML += generateTitle(PSTR("Ladder Logic Script Page")); //generates the title for the page
	HTML += generateAlertsScript( 1 ); //hackhack for now -- index may vary, unless explicitly assigned to '1'
	
	HTML += html_form_Begin + scriptDir + html_form_Middle;
	HTML += html_paragraph_begin;
	for ( uint8_t x = 0; x < p_UIDataTables.size(); x++ )
		HTML += p_UIDataTables[x]->GenerateTableHTML(); //Add each datafield to the HTML body
		
	HTML += html_paragraph_end;
	HTML += html_form_End;
	HTML += generateFooter(); //Add the footer stuff.
	p_server->send(200, transmission_HTML, HTML ); //And we're off.
	p_UIDataTables.clear();
}


void UICore::applyLogic()
{
	if ( PLCObj.parseScript( PLCObj.getScript()) )  //Parses the PLC logic script, and also performs eror checking along the way.
	{
		if ( Core.b_SaveScript ) //Only save the script if we have properly parsed it.
		{
			if (!Core.savePLCScript(PLCObj.getScript()))
				Core.sendMessage(PSTR("Failed to save logic script."), PRIORITY_HIGH);
		}
	}
	/*else //the parse of the new script failed, so revert the script to the perviously stored (and hopefully successful) logic script.
	{ //lets just leave this for now, so we can give the user a chance to edit the failed PLC script.
		Core.loadPLCScript(PLCObj.getScript());
	}*/
	Core.b_SaveScript = false; //Just a one off.
}