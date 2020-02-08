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
	shared_ptr<DataTable> logicTable( new DataTable( PSTR("PLC Logic Script") ) );
	shared_ptr<DataTable> saveTable( new DataTable() ); //used for settings aplication

	uint8_t index = 1;
	logicTable->AddElement( make_shared<STRING_S_Datafield>(UICore::applyLogic, PLCObj.getSharedScript(), index++, FIELD_TYPE::TEXTAREA, PSTR("Logic Script") ) );
	logicTable->AddElement( make_shared<BOOL_Datafield>( &b_SaveScript, index++, FIELD_TYPE::CHECKBOX, PSTR("Save Script in Flash RAM") ) );

	saveTable->AddElement( make_shared<DataField>(index++, FIELD_TYPE::SUBMIT, PSTR("Apply Settings") ) );

	p_UIDataTables.push_back(logicTable);
	p_UIDataTables.push_back(saveTable);
}

void UICore::HandleScript() //Generate the HTML for our main page.
{	  
	if (!handleAuthorization())
		return;

	createScriptFields();

	String AlertHTML;
	if ( p_server->args() ) //Do we have some args to input? Apply settings if so (before generating the rest of the HTML)
		UpdateWebFields( p_UIDataTables, AlertHTML );

	String HTML = generateHeader();
	HTML += generateTitle(); //generates the title for the page
	HTML += AlertHTML; //Add any additional HTML generated after post method.
	
	HTML += form_Begin + scriptDir + form_Middle;
	HTML += "<P>";
	for ( uint8_t x = 0; x < p_UIDataTables.size(); x++ )
		HTML += p_UIDataTables[x]->GenerateTableHTML(); //Add each datafield to the HTML body
		
	HTML += "</P>";
	HTML += form_End;
	HTML += generateFooter(); //Add the footer stuff.
	p_server->send(200, transmission_HTML, HTML ); //And we're off.
	p_UIDataTables.clear();
}


void UICore::applyLogic()
{
	Core.sendMessage("yeah");
	PLCObj.parseScript(PLCObj.getScript()); //Parses the PLC logic script.
	if ( Core.b_SaveScript )
	{
		Core.savePLCScript(PLCObj.getScript());
		Core.b_SaveScript = false; //Just a one off.
	}
}