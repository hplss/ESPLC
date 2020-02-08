#include <CORE/UICore.h>
#include <PLC/PLC_Main.h>

extern UICore Core; //Used to identify our initialized core object.

void UICore::createStyleSheetFields() //Probably shouldnt be called more than once.
{
	shared_ptr<DataTable> sheetTable( new DataTable( F("Web UI Style Sheet") ) ),
						  saveTable ( new DataTable() ); //used for settings aplication

	uint8_t index = 1;
	sheetTable->AddElement( make_shared<STRING_S_Datafield>(UICore::applyStyleSheet, s_StyleSheet, index++, FIELD_TYPE::TEXTAREA, F("Style Sheet") ) );
	sheetTable->AddElement( make_shared<BOOL_Datafield>( &b_SaveStyleSheet, index++, FIELD_TYPE::CHECKBOX, F("Save StyleSheet in Flash RAM") ) );

	saveTable->AddElement( make_shared<DataField>(index++, FIELD_TYPE::SUBMIT, F("Apply Settings") ) );

	p_UIDataTables.push_back(sheetTable);
	p_UIDataTables.push_back(saveTable);
}

void UICore::handleStyleSheet() //Generate the HTML for our main page.
{	  
	if ( !handleAuthorization() )
		return;

	createStyleSheetFields();
	String AlertHTML;
	if ( p_server->args() ) //Do we have some args to input? Apply settings if so (before generating the rest of the HTML)
		UpdateWebFields( p_UIDataTables, AlertHTML );

	String HTML = generateHeader(); //start with the header
	HTML += generateTitle("Style");
	HTML += AlertHTML; //Add any additional HTML generated after post method.
	
	//HTML += PSTR("<FORM action=\".") + styleDir + PSTR("\" method=\"post\" id=\"form\">");
	HTML += form_Begin + styleDir + form_Middle;
	HTML += "<P>";
	for ( uint8_t x = 0; x < p_UIDataTables.size(); x++ )
		HTML += p_UIDataTables[x]->GenerateTableHTML(); //Add each datafield to the HTML body
		
	HTML += PSTR("</P>");
	HTML += form_End;
	HTML += generateFooter(); //Add the footer stuff.
	p_server->send(200, transmission_HTML, HTML ); //And we're off.
	p_UIDataTables.clear();
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