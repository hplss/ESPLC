/*
 * page_index.cpp
 *
 * Created: 2/12/2018 4:04:16 PM
 *  Author: Andrew
  * The purpose of this file is to house the HTML generator for the index page and related functions.
  */

#include <CORE/UICore.h>
#include <PLC/PLC_Main.h>

extern PLC_Main PLCObj;
extern UICore Core;

void UICore::createIndexFields() //Probably shouldnt be called more than once.
{
	shared_ptr<DataTable> indexTable(new DataTable( F("Device Controls") ) );
	uint8_t index = 1;
	indexTable->AddElement( make_shared<Hyperlink_Datafield>( index++, FIELD_TYPE::HYPERLINK, PSTR("PLC Logic Script"), scriptDir ) );
	indexTable->AddElement( make_shared<Hyperlink_Datafield>( index++, FIELD_TYPE::HYPERLINK, PSTR("Device Configuration"), adminDir ) );
	indexTable->AddElement( make_shared<Hyperlink_Datafield>( index++, FIELD_TYPE::HYPERLINK, PSTR("UI Style Sheet"), styleDir ) );

	p_UIDataTables.push_back(indexTable);
}

void UICore::handleIndex() //Generate the HTML for our main page.
{	  
	//String AlertHTML;
	//if ( p_server->args() ) //Do we have some args to input? Apply settings if so (before generating the rest of the HTML)
	//	UpdateWebFields( p_indexDataTables, AlertHTML );
	createIndexFields();
	
	String HTML = generateHeader();
	HTML += generateTitle();
	//HTML += PSTR("<FORM action=\"/\" method=\"post\" id=\"form\">");
	HTML += "<P>";
	for ( uint8_t x = 0; x < p_UIDataTables.size(); x++ )
		HTML += p_UIDataTables[x]->GenerateTableHTML(); //Add each datafield to the HTML body
		
	HTML += "</P>";
	//HTML += PSTR("</FORM>");
	HTML += generateFooter(); //Add the footer stuff.
	p_server->send(200, transmission_HTML, HTML ); //And we're off.
	p_UIDataTables.clear();
}