#include <CORE/UICore.h>
#include <Update.h>

extern UICore Core; //Accessor for the initialized UIcore object.

void UICore::createUpdaterFields()
{
    shared_ptr<DataTable> alertsTable( new DataTable( table_title_messages ) ),                
						  saveTable ( new DataTable( PSTR("Firmware File") ) ); //used for settings aplication

	uint8_t index = 2;
    
    alertsTable->AddElement( make_shared<Hyperlink_Datafield>( index++, PSTR("Back to Index"), "/" ) );
	alertsTable->AddElement( make_shared<VAR_Datafield>( make_shared<String>(""), 1, FIELD_TYPE::TEXTAREA, field_title_alerts, vector<String>{}, ALERTS_FIELD_COLS, ALERTS_FIELD_ROWS ) );

    saveTable->AddElement( make_shared<FILE_Datafield>( vector<String>{PSTR("bin")}, false, index++, PSTR("Firmware File") ) );
	saveTable->AddElement( make_shared<DataField>(index++, FIELD_TYPE::SUBMIT, PSTR("Update Firmware") ) );

    p_StaticDataTables.push_back(alertsTable); //Doesn't doesn't post to the web server
	p_UIDataTables.push_back(saveTable);
}

void UICore::handleUpdater()
{
    if (!handleAuthorization())
		return;

    createUpdaterFields();

    if ( getWebServer().args() ) //Do we have some args to input? Apply settings if so (before generating the rest of the HTML)
		UpdateWebFields( p_UIDataTables );

    String HTML = generateHeader();
	HTML += generateTitle(PSTR("Firmware Update"));
    HTML += generateAlertsScript( 1 ); //hackhack for now -- index may vary, unless explicitly assigned to '1'

    for ( uint8_t x = 0; x < p_StaticDataTables.size(); x++ ) //Generate static objects that are not included in the FORM
        HTML += p_StaticDataTables[x]->GenerateTableHTML();

    HTML += html_form_Begin + updateDir + html_form_Middle_Upload;
	for ( uint8_t x = 0; x < p_UIDataTables.size(); x++ )
		HTML += p_UIDataTables[x]->GenerateTableHTML(); //Add each datafield to the HTML body

    HTML += html_form_End;	
	HTML += generateFooter(); //Add the footer stuff.
    getWebServer().sendHeader(http_header_connection, http_header_close);
	getWebServer().send(200, transmission_HTML, HTML ); //And we're off.
	resestFieldContainers();
}

void UICore::applyRemoteFirmwareUpdate()
{
    HTTPUpload *fileUpload = &Core.getWebServer().upload();
    
    if (fileUpload->status == UPLOAD_FILE_START) 
    {
        Core.sendMessage(PSTR("Firmware Update: ") + fileUpload->filename, PRIORITY_HIGH );
        if (!Update.begin(UPDATE_SIZE_UNKNOWN)) 
        { //start with max available size
            Core.sendMessage(Update.errorString(), PRIORITY_HIGH);
        }
    } 
    else if (fileUpload->status == UPLOAD_FILE_WRITE) 
    {
        if (Update.write(fileUpload->buf, fileUpload->currentSize) != fileUpload->currentSize) //attempt to write the firmware to the ESP32
        {
            Core.sendMessage(Update.errorString(), PRIORITY_HIGH);
        }
    } 
    else if (fileUpload->status == UPLOAD_FILE_END) 
    {
        if (Update.end(true)) 
        { //true to set the size to the current progress
            Core.sendMessage(PSTR("Firmware Update Success: ") + String(fileUpload->totalSize), PRIORITY_HIGH);
            ESP.restart(); //restart the ESP32
        } 
        else 
        {
            Core.sendMessage(Update.errorString(), PRIORITY_HIGH);
        }
    }
}
