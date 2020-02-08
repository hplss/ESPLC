/*
 * data_fields.cpp
 *
 * Created: 2/2/2018 11:26:31 AM
 *  Author: Andrew
 */ 


#include "../CORE/UICore.h"

bool DataField::SetFieldValue( shared_ptr<String> input )
{
	if ( input->length() > MAX_DATA_LENGTH && GetType() != FIELD_TYPE::TEXT && GetType() != FIELD_TYPE::TEXTAREA )
		return false; //Die here, data string too long.
	
	s_fieldValue = input;
	return true;
}

bool DataField::SetFieldValue( const String &input )
{
	if ( input.length() > MAX_DATA_LENGTH && GetType() != FIELD_TYPE::TEXT && GetType() != FIELD_TYPE::TEXTAREA )
		return false; //Die here, data string too long.

	*s_fieldValue = input;
	return true;
}
String DataField::GenerateHTML() //Baseclass GenerateHTML
{
	String HTML;

	if ( strlen( GetFieldLabel().c_str() ) && GetType() != FIELD_TYPE::SUBMIT ) //Is there a label?
		HTML += GetFieldLabel() + ": ";
	
	if ( GetType() != FIELD_TYPE::TEXTAREA )
	{
		HTML += F("<INPUT ");

		switch ( GetType() )
		{
			case FIELD_TYPE::TEXT:
				HTML += F("type=\"text\" ");
				break;
			case FIELD_TYPE::SUBMIT:
				HTML += F("type=\"submit\" ");
				break;
			case FIELD_TYPE::RADIO:
				HTML += F("type=\"radio\" ");
				break;
			case FIELD_TYPE::CHECKBOX:
				HTML += F("type=\"checkbox\" ");
				break;
			case FIELD_TYPE::PASSWORD:
				HTML += F("type=\"password\" ");
				break;
		}
	
		switch ( GetType() )
		{
			case FIELD_TYPE::CHECKBOX: //Checkboxes are special
				if ( strlen( GetFieldValue().c_str() ) && GetFieldValue() != String(false) ) //Only set this if we have some value there, otherwise it'll just count as "enabled" - weird.
					HTML += "checked ";
				break;
			case FIELD_TYPE::SUBMIT:
				HTML += PSTR("value=\"") + GetFieldLabel() + "\" "; //Label text goes into the button itself
				break;
			default: //Everything else does this.
				HTML += PSTR("value=\"") + GetFieldValue() + "\" ";
				break;
		}	
	}

	if ( strlen( GetFieldName().c_str() ) && GetType() != FIELD_TYPE::SUBMIT ) //If we even have a name.
		HTML += PSTR("name=\"") + GetFieldName() + "\" ";		
	
	HTML += ">"; //End of the <INPUT 
	
	if ( DoNewline() )	//Generate newline?
		HTML += F("<br>");

	return HTML;
}

String SSID_Datafield::GenerateHTML() //This is a specific type of field, tailored to a specific function.
{
	String HTML;

	if ( strlen( GetFieldLabel().c_str() ) ) //Is there a label?
		HTML += GetFieldLabel() + ": ";
	
	HTML += F("<select ");
	HTML += "id=\"" + String(GetAddress()) + PSTR("\" name=\"") + GetFieldName() + PSTR("\" >");
	
	if ( WiFi.scanNetworks() <= 0 )
	{
		if ( WiFi.status() != WL_CONNECTED ) 
			HTML += F("<option >N/A</option>");
		else 
			HTML += PSTR("<option>") + String(WiFi.SSID()) + PSTR(" (Connected)</option>"); //? hmm
	}
	else
	{
		for ( uint8_t x = 0; x < WiFi.scanNetworks(); x++ )
		{
			HTML += PSTR("<option value=\"") + String(x) + "\"";
			if ( WiFi.SSID( x ) == WiFi.SSID() )
			{
				HTML += F(" selected ");
				HTML += ">" + String(WiFi.SSID(x)) + PSTR(" (Connected)");
			}
			else
				HTML += ">" + String(WiFi.SSID(x)); 
				
			HTML += PSTR("</option>");
		}
	}
	
	HTML += F("</select>"); //End of the <INPUT
	
	if ( DoNewline() )	//Generate newline?
		HTML += F("<br>");

	return HTML;
}


//  -------------------------------------------------------------------------------
//	-- DATA TABLE STUFF BELOW
//  -------------------------------------------------------------------------------

bool DataTable::RemoveElement( unsigned int index )
{
	int8_t x = IteratorFromAddress( index );
	
	if ( x > -1 )
	{
		p_fields.erase( p_fields.begin() + x );
		return true; //Die here.
	}
	return false;
}

shared_ptr<DataField> DataTable::GetElementByID( unsigned int index )
{
	int8_t x = IteratorFromAddress( index );
	
	if ( x > -1 )
		return p_fields[x];

	return 0; //Default return path
}

shared_ptr<DataField> DataTable::GetElementByName( const String &name )
{
	for ( uint8_t x = 0; x < p_fields.size(); x++ )
	{
		if ( p_fields[x]->GetFieldName() == name )
			return p_fields[x]; 
	}
	
	return 0; //default return path
}

bool DataTable::AddElement( shared_ptr<DataField> field )
{
	if ( IteratorFromAddress( field->GetAddress() ) > -1 ) //Valid iterator found, index already taken.
		return false;
	
	p_fields.push_back( field );
	return true;
}

int8_t DataTable::IteratorFromAddress( unsigned int index )
{
	for ( uint8_t x = 0; x < p_fields.size(); x++ )
	{
		if ( p_fields[x]->GetAddress() == index ) //Address exists?
			return x;
	}
	
	return -1;
}

String DataTable::GenerateTableHTML()
{
	String HTML;
	//sprintf_P(HTML.c_str(), " ", "");
	HTML += PSTR("<h2>") + s_tableName + PSTR("</h2>");
	HTML += PSTR("<table id =\"\"  style=\"width:100%\" > ");
	HTML += PSTR("<P>");
	for ( uint8_t x = 0; x < p_fields.size(); x++ )
	{
		if ( !p_fields[x]->IsEnabled() )
			continue;
			
		HTML += p_fields[x]->GenerateHTML();
	}
	HTML += F("</P>");
	HTML += F("</table>");
	return HTML;
}

//////////////////////////////////////////////////////////////////////////
// - SPECIAL FIELD STUFF BELOW - The idea here is that we're modifying the variables that are tied to the objects, 
// - rather than the local variables that are stored in the DataField class. This will allow us to view changes to these variables 
// - made by other methods (serial parser for example) in the HTML, without the need for matching the var and the FieldValue string. 
//////////////////////////////////////////////////////////////////////////

/*bool STRING_Datafield::SetFieldValue( const String &value )
{
	if ( GetVar() != value  ) //Check the main variable being modified first.
	{
		GetVar() = value;
		return true;
	}
	return false;
}*/

String Hyperlink_Datafield::GenerateHTML()
{
	String HTML = PSTR("<a href=\"") + GetFieldValue() + "\">" + GetFieldLabel() + "</a>";
	if ( DoNewline() )	//Generate newline?
		HTML += F("<br>");
	return HTML;
}

String STRING_Datafield::GenerateHTML() 
{ 
	if ( GetType() != FIELD_TYPE::TEXTAREA ) //Exit here if this isn't a text area (basically this would be a single row textox)
		return DataField::GenerateHTML(); //call default generation function
	
	String HTML;

	HTML += F("<textarea form=\"form\" rows=\"10\" cols=\"50\"");//"form" is the default form name. It'll work for now
	if ( strlen( GetFieldName().c_str() ) ) //If we even have a name.
		HTML += "name=\"" + GetFieldName() + "\" ";	

	HTML += ">";
	HTML += DataField::GetFieldValue(); //put stored text values here
	HTML += PSTR("</textarea><br>");
	return HTML;
}

bool UINT_Datafield::SetFieldValue( const String &value )
{
	unsigned int newValue = parseInt( value );
	if ( *GetVar() != newValue  )
	{
		//if (DataField::SetFieldValue(value))
		{
			*GetVar() = newValue;
			return true;
		}
	}
	return false;
}

bool UINT_Datafield::SetFieldValue( const unsigned int &value )
{
	if ( *GetVar() != value  )
	{
		//if (DataField::SetFieldValue(String(value)))
		{
			*GetVar() = value;
			return true;
		}
	}
	return false;
}

bool UINT8_Datafield::SetFieldValue( const String &value )
{
	uint8_t newValue = parseInt( value );
	if ( *GetVar() != newValue  )
	{
		//if (DataField::SetFieldValue(value))
		{
			*GetVar() = newValue;
			return true;
		}
	}
	return false;
}

bool UINT8_Datafield::SetFieldValue( const uint8_t &value )
{
	if ( *GetVar() != value  )
	{
		//if (DataField::SetFieldValue(String(value)))
		{
			*GetVar() = value;
			return true;
		}
	}
	return false;
}

bool BOOL_Datafield::SetFieldValue( const String &value )
{
	bool newValue = ( parseInt( value ) || value.length() );
	if ( *GetVar() != newValue  )
	{
		//if (DataField::SetFieldValue(value))
		{
			*GetVar() = newValue;
			Serial.println(*GetVar());
			return true;
		}
	}
	return false;
}

bool BOOL_Datafield::SetFieldValue( const bool &value )
{
	if ( *GetVar() != value  )
	{
		//if (DataField::SetFieldValue(String(value)))
		{
			*GetVar() = value;
			return true;
		}
	}
	return false;
}