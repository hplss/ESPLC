#include <CORE/UICore.h>
#include <PLC/PLC_Main.h>

void UICore::createStyleSheetFields() //Probably shouldnt be called more than once.
{
	shared_ptr<DataTable> alertsTable( new DataTable( table_title_messages ) ),
						  sheetTable( new DataTable( PSTR("Web UI Style Sheet") ) ),
						  saveTable ( new DataTable() ); //used for settings aplication

	uint8_t index = 2;
	alertsTable->AddElement( make_shared<Hyperlink_Datafield>( index++, PSTR("Back to Index"), "/" ) );
	alertsTable->AddElement( make_shared<VAR_Datafield>( make_shared<String>(""), 1, FIELD_TYPE::TEXTAREA, field_title_alerts, vector<String>{}, ALERTS_FIELD_COLS, ALERTS_FIELD_ROWS ) );

	sheetTable->AddElement( make_shared<VAR_S_Datafield>(UICore::applyStyleSheet, s_StyleSheet, index++, FIELD_TYPE::TEXTAREA, PSTR("Style Sheet"), vector<String>{"form"}, 50, 25 ) );
	sheetTable->AddElement( make_shared<VAR_Datafield>( &b_SaveStyleSheet, index++, FIELD_TYPE::CHECKBOX, PSTR("Save StyleSheet in Flash") ) );

	saveTable->AddElement( make_shared<DataField>(index++, FIELD_TYPE::SUBMIT, PSTR("Apply Settings") ) );

	p_StaticDataTables.push_back(alertsTable);
	p_UIDataTables.push_back(sheetTable);
	p_UIDataTables.push_back(saveTable);
}

void UICore::handleStyleSheet() //Generate the HTML for our main page.
{	  
	if ( !handleAuthorization() )
		return;

	createStyleSheetFields();
	
	if ( getWebServer().args() ) //Do we have some args to input? Apply settings if so (before generating the rest of the HTML)
		UpdateWebFields( p_UIDataTables );

	String HTML = generateHeader(); //start with the header
	HTML += generateTitle(PSTR("Style"));
	HTML += generateAlertsScript( 1 ); //hackhack for now -- index may vary, unless explicitly assigned to '1'

	for ( uint8_t x = 0; x < p_StaticDataTables.size(); x++ ) //Generate static objects that are not included in the FORM
        HTML += p_StaticDataTables[x]->GenerateTableHTML();
	
	HTML += html_form_Begin + styleDir + html_form_Middle;
	for ( uint8_t x = 0; x < p_UIDataTables.size(); x++ )
		HTML += p_UIDataTables[x]->GenerateTableHTML(); //Add each datafield to the HTML body
		
	HTML += html_form_End;
	HTML += generateFooter(); //Add the footer stuff.
	getWebServer().sendHeader(http_header_connection, http_header_close);
	getWebServer().send(200, transmission_HTML, HTML ); //And we're off.
	resestFieldContainers();
}

/*
void UICore::sendStyleSheet()
{
	if ( !b_FSOpen )
        p_server->send(200, transmission_HTML, s_StyleSheet);

    File styleFile = SPIFFS.open(file_Stylesheet, FILE_READ);
    if (!styleFile)
    {
        p_server->send(200, transmission_HTML, s_StyleSheet);
    }

	while ( styleFile.position() != styleFile.size() )
	{
		String sheetdata = styleFile.readStringUntil(CHAR_NEWLINE);
		p_server->send(200, transmission_HTML, sheetdata);
	}
    styleFile.close();
}*/

void UICore::applyStyleSheet()
{
	if ( Core.b_SaveStyleSheet )
	{
		Core.saveWebStyleSheet(Core.getStyleSheet());
		Core.b_SaveStyleSheet = false; //Just a one off.
	}
}