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
	shared_ptr<DataTable> indexTable(new DataTable( ("Device Controls") ) );
	uint8_t index = 1;
	indexTable->AddElement( make_shared<Hyperlink_Datafield>( index++, PSTR("PLC Logic Script"), scriptDir ) );
	indexTable->AddElement( make_shared<Hyperlink_Datafield>( index++, PSTR("PLC Object Status"), statusDir ) );
	indexTable->AddElement( make_shared<Hyperlink_Datafield>( index++, PSTR("Device Configuration"), adminDir ) );
	indexTable->AddElement( make_shared<Hyperlink_Datafield>( index++, PSTR("UI Style Sheet"), styleDir ) );
	indexTable->AddElement( make_shared<Hyperlink_Datafield>( index++, PSTR("Firmware Update"), updateDir ) );

	p_UIDataTables.push_back(indexTable);
}

void UICore::handleIndex() //Generate the HTML for our main page.
{	  
	createIndexFields();
	
	String HTML = generateHeader();
	HTML += generateTitle();
	for ( uint8_t x = 0; x < p_UIDataTables.size(); x++ )
		HTML += p_UIDataTables[x]->GenerateTableHTML(); //Add each datafield to the HTML body
		
	HTML += generateFooter(); //Add the footer stuff.
	getWebServer().sendHeader(http_header_connection, http_header_close);
	getWebServer().send(200, transmission_HTML, HTML ); //And we're off.
	resestFieldContainers();
}