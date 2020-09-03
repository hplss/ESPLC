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
	if ( input.length() > iCols && GetType() != FIELD_TYPE::TEXTAREA ) //columns used as a data limiter size for non-textarea fields.
		return false; //Die here, data string too long.

	*s_fieldValue = input;
	return true;
}
String DataField::GenerateHTML() //Baseclass GenerateHTML
{
	String HTML;

	if ( strlen( GetFieldLabel().c_str() ) && GetType() != FIELD_TYPE::SUBMIT ) //Is there a label? If so, generate the appropriate HTML
		HTML += PSTR("<label for=\"") + String(GetAddress()) + "\">" + GetFieldLabel() + PSTR(": </label>");
	
	if ( GetType() != FIELD_TYPE::TEXTAREA )
	{
		HTML += F("<INPUT ");

		switch ( GetType() )
		{
			case FIELD_TYPE::TEXT:
				HTML += PSTR("type=\"text\" maxlength=\"") + String(iCols) + PSTR("\"") + PSTR("size=\"") + String(iCols) + PSTR("\"");
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
				HTML += PSTR("type=\"password\" maxlength=\"") + String(iCols) + PSTR("\" ") + PSTR("size=\"") + String(iCols) + PSTR("\" ");;
				break;
			case FIELD_TYPE::NUMBER:
				HTML += PSTR("type=\"number\" min=\"0\" size=\"") + String(iCols) + PSTR("\" oninput=\"validity.valid||(value=min);\" ");;
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
		if ( strlen( GetFieldName().c_str() ) && GetType() != FIELD_TYPE::SUBMIT ) //If we even have a name.
			HTML += PSTR("name=\"") + GetFieldName() + "\" ";		

		HTML += PSTR("id=\"") + String(GetAddress()) + "\"";
		HTML += ">"; //End of the <INPUT 
	}
	else
	{
		HTML += PSTR("<textarea form=\"form\" id=\"") + String(GetAddress()) + PSTR("\" rows=\"") + String(iRows) + PSTR("\" cols=\"") + String (iCols) + "\"";//"form" is the default form name. It'll work for now
		if ( strlen( GetFieldName().c_str() ) ) //If we even have a name.
			HTML += PSTR("name=\"") + GetFieldName() + "\"";	
		HTML += ">";
		HTML += DataField::GetFieldValue(); //put stored text values here
		HTML += PSTR("</textarea>");
	}

	if ( DoNewline() )	//Generate newline?
		HTML += F("<br>");

	return HTML;
}

String SSID_Datafield::GenerateHTML() //This is a specific type of field, tailored to a specific function.
{
	String HTML;

	int16_t networks = WiFi.scanNetworks(false, false, false, 200); //200MS instead of 300ms - make scans faster

	if ( strlen( GetFieldLabel().c_str() ) ) //Is there a label?
		HTML += GetFieldLabel() + ": ";
	
	HTML += PSTR("<select ");
	HTML += PSTR("id=\"") + String(GetAddress()) + PSTR("\" name=\"") + GetFieldName() + PSTR("\" >");
	
	if ( networks <= 0 )
	{
		if ( WiFi.status() != WL_CONNECTED ) 
			HTML += PSTR("<option >N/A</option>");
		else 
			HTML += PSTR("<option>") + String(WiFi.SSID()) + PSTR(" (Connected)</option>"); //? hmm
	}
	else
	{
		for ( uint8_t x = 0; x < networks; x++ ) //more than 255 networks? Hmm
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

String Select_Datafield::GenerateHTML()
{
	String HTML;
	if ( strlen( GetFieldLabel().c_str() ) ) //Is there a label?
		HTML += GetFieldLabel() + ": ";
	
	HTML += PSTR("<select ");
	HTML += PSTR("id=\"") + String(GetAddress()) + PSTR("\" name=\"") + GetFieldName() + PSTR("\" >");
	
	if ( options.size() > 0 )
	{
		for ( uint8_t x = 0; x < options.size(); x++ ) //more than 255 networks? Hmm
		{
			HTML += PSTR("<option value=\"") + String(x) + "\"";
			if ( x == intFromValue() )
			{
				HTML += PSTR(" selected ");
				HTML += ">" + options[x] + PSTR(" (Selected)");
			}
			else
				HTML += ">" + options[x]; 
				
			HTML += PSTR("</option>");
		}
	}
	
	HTML += PSTR("</select>");
	
	if ( DoNewline() )	//Generate newline?
		HTML += PSTR("<br>");

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
	//HTML += PSTR("<table id =\"\" > ");
	HTML += html_paragraph_begin;
	for ( uint8_t x = 0; x < p_fields.size(); x++ )
	{
		HTML += p_fields[x]->GenerateHTML();
	}
	HTML += html_paragraph_end;
	//HTML += PSTR("</table>");
	return HTML;
}

//////////////////////////////////////////////////////////////////////////
// - SPECIAL FIELD STUFF BELOW - The idea here is that we're modifying the variables that are tied to the objects, 
// - rather than the local variables that are stored in the DataField class. This will allow us to view changes to these variables 
// - made by other methods (serial parser for example) in the HTML, without the need for matching the var and the FieldValue string. 
//////////////////////////////////////////////////////////////////////////

String Hyperlink_Datafield::GenerateHTML()
{
	String HTML = PSTR("<li><a href=\"") + GetFieldValue() + "\">" + GetFieldLabel() + "</a></li>";
	if ( DoNewline() )	//Generate newline?
		HTML += PSTR("<br>");
	return HTML;
}

bool VAR_Datafield::SetFieldValue( const String &value )
{
	switch(iVarType)
	{
		case TYPE_VAR_BOOL:
		{
			bool temp = parseInt( value );
			if ( *variablePtr.bVar != temp )
			{
				*variablePtr.bVar = temp;
				return true;
			}
			else if ( value.length() )
			{
				*variablePtr.bVar = true;
				return true;
			}
			else
			{
				*variablePtr.bVar = false;
				return true;
			}
		}
		break;
		case TYPE_VAR_UBYTE:
		{
			uint8_t temp = parseInt( value );
			if ( *variablePtr.uByteVar != temp )
			{
				*variablePtr.uByteVar = temp;
				return true;
			}
		}
		break;
		case TYPE_VAR_UINT:
		{
			uint_fast32_t temp = parseInt( value );
			if ( *variablePtr.uiVar != temp )
			{
				*variablePtr.uiVar = temp;
				return true;
			}
		}
		break;
		case TYPE_VAR_INT:
		{
			int_fast32_t temp = parseInt( value );
			if ( *variablePtr.iVar != temp )
			{
				*variablePtr.iVar = temp;
				return true;
			}
		}
		break;
		case TYPE_VAR_USHORT:
		{
			uint16_t temp = parseInt( value );
			if ( *variablePtr.uiShortVar != temp )
			{
				*variablePtr.uiShortVar = temp;
				return true;
			}
		}
		break;
		case TYPE_VAR_FLOAT:
		{
			float temp = parseFloat( value );
			if ( *variablePtr.fVar != temp )
			{
				*variablePtr.fVar = temp;
				return true;
			}
		}
		break;
		case TYPE_VAR_STRING:
		{
			return DataField::SetFieldValue(value);
		}
		break;
	}
	return false;
}
int_fast32_t VAR_Datafield::intFromValue()
{
	switch(iVarType)
	{
		case TYPE_VAR_BOOL:
			return *variablePtr.bVar;
		case TYPE_VAR_INT:
			return *variablePtr.iVar;
		case TYPE_VAR_FLOAT:
			return *variablePtr.fVar;
		case TYPE_VAR_LONG:
			return *variablePtr.lVar;
		case TYPE_VAR_UBYTE:
			return *variablePtr.uByteVar;
		case TYPE_VAR_USHORT:
			return *variablePtr.uiShortVar;
		case TYPE_VAR_STRING:
			return parseInt(GetFieldValue());
	}
	return 0;
}
String LADDER_OBJ_Datafield::GenerateHTML() 
{
	/*<script>
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
    document.getElementById("test" + i).innerHTML = out;
  }
}
</script>*/
	return "";
}
