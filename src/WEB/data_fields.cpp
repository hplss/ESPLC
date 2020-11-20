/*
 * data_fields.cpp
 *
 * Created: 2/2/2018 11:26:31 AM
 *  Author: Andrew Ward
 * This file contains the function definitions for all types of fields that are displayed in the WebUI for ESPLC.
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
	if ( GetType() == FIELD_TYPE::FILE_UPLOAD )
		return true;

	else if ( input.length() > iCols && GetType() != FIELD_TYPE::TEXTAREA ) //columns used as a data limiter size for non-textarea fields.
	{
		return false; //Die here, data string too long.
	}

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
		HTML += PSTR("<INPUT ");

		switch ( GetType() )
		{
			case FIELD_TYPE::TEXT:
				HTML += PSTR("type=\"text\" maxlength=\"") + String(iCols) + PSTR("\"") + PSTR("size=\"") + String(iCols) + PSTR("\"");
				break;
			case FIELD_TYPE::SUBMIT:
				HTML += PSTR("type=\"submit\" ");
				break;
			case FIELD_TYPE::RADIO:
				HTML += PSTR("type=\"radio\" ");
				break;
			case FIELD_TYPE::CHECKBOX:
				HTML += PSTR("type=\"checkbox\" ");
				break;
			case FIELD_TYPE::PASSWORD:
				HTML += PSTR("type=\"password\" maxlength=\"") + String(iCols) + PSTR("\" ") + PSTR("size=\"") + String(iCols) + PSTR("\" ");
				break;
			case FIELD_TYPE::NUMBER:
				HTML += PSTR("type=\"number\" min=\"0\" size=\"") + String(iCols) + PSTR("\" oninput=\"validity.valid||(value=min);\" ");
				break;
			default:
			break;
		}
	
		switch ( GetType() ) //This switch case is exclusively used for setting the "value" html tag for certain forms. 
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
		HTML += PSTR("<textarea form=\"");
		if ( getSpecialParams().size() )
			HTML += getSpecialParams().front(); //first only	
		HTML += PSTR("\" id=\"") + String(GetAddress()) + PSTR("\" rows=\"") + String(iRows) + PSTR("\" cols=\"") + String (iCols) + "\"";//"form" is the default form name. It'll work for now
		if ( strlen( GetFieldName().c_str() ) ) //If we even have a name.
			HTML += PSTR("name=\"") + GetFieldName() + "\"";	
		HTML += ">";
		HTML += DataField::GetFieldValue(); //put stored text values here
		HTML += PSTR("</textarea>");
	}

	if ( DoNewline() )	//Generate newline?
		HTML += PSTR("<br>");

	return HTML;
}

String SSID_Datafield::GenerateHTML() //This is a specific type of field, tailored to a specific function.
{
	String HTML;

	int16_t networks = WiFi.scanNetworks(false, false, false, 200); //200MS instead of 300ms - make scans faster

	if ( strlen( GetFieldLabel().c_str() ) ) //Is there a label?
		HTML += PSTR("<label for=\"") + String(GetAddress()) + "\">" + GetFieldLabel() + PSTR(": </label>");
	
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
		HTML += PSTR("<label for=\"") + String(GetAddress()) + "\">" + GetFieldLabel() + PSTR(": </label>");
	
	HTML += PSTR("<select ");
	HTML += PSTR("id=\"") + String(GetAddress()) + PSTR("\" name=\"") + GetFieldName() + PSTR("\" >");
	
	if ( getSpecialParams().size() > 0 )
	{
		for ( uint8_t x = 0; x < getSpecialParams().size(); x++ ) //more than 255 networks? Hmm
		{
			HTML += PSTR("<option value=\"") + String(x) + "\"";
			if ( x == intFromValue() )
			{
				HTML += PSTR(" selected ");
				HTML += ">" + getSpecialParams()[x] + PSTR(" (Selected)");
			}
			else
				HTML += ">" + getSpecialParams()[x]; 
				
			HTML += PSTR("</option>");
		}
	}
	
	HTML += PSTR("</select>");
	
	if ( DoNewline() )	//Generate newline?
		HTML += PSTR("<br>");

	return HTML;
}

String FILE_Datafield::GenerateHTML()
{
	String HTML;
	if ( strlen( GetFieldLabel().c_str() ) ) //Is there a label for the field?
		HTML += PSTR("<label for=\"") + String(GetAddress()) + "\">" + GetFieldLabel() + PSTR(": </label>");

	HTML += PSTR("<Input type=\"file\" accept=\"");

	for (uint8_t x = 0; x < getFileTypes().size(); x++ ) //iterate through the list of available file types.
	{
		if ( !x ) //first iteration
			HTML += "." + getFileTypes()[x];
		else //all others
			HTML += ",." + getFileTypes()[x];
	}
	HTML += "\" "; //end accept param
	HTML += PSTR("name=\"") + GetFieldName() + "\" ";		
	HTML += PSTR("id=\"") + String(GetAddress()) + "\" ";
	if (b_multiple)
		HTML += PSTR("multiple");
	HTML += ">"; //End of the <INPUT 
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
	//HTML += PSTR("<h2>") + s_tableName + PSTR("</h2>");
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
	String HTML = PSTR("<li><a href=\"") + GetFieldValue() + "\">" + GetFieldLabel() + PSTR("</a></li>");
	if ( DoNewline() )	//Generate newline?
		HTML += PSTR("<br>");
	return HTML;
}

bool VAR_Datafield::SetFieldValue( const String &value )
{
	switch(iVarType)
	{
		case OBJ_TYPE::TYPE_VAR_BOOL:
		{
			bool temp = false;
			int64_t parsed = parseInt( value );
			if ( parsed > 0 || value.length() )
				temp = true;

			if ( *variablePtr.bVar != temp && DataField::SetFieldValue(intToStr(temp)) ) //only update if different
			{
				*variablePtr.bVar = temp;
				return true;
			}
		}
		break;
		case OBJ_TYPE::TYPE_VAR_UBYTE:
		{
			uint8_t temp = 0;
			int64_t parsed = parseInt( value );
			if ( parsed > UINT8_MAX )
			{
				temp = UINT8_MAX;
			}
			else if ( parsed > 0 )
			{
				temp = parsed;
			}

			if ( *variablePtr.uByteVar != temp && DataField::SetFieldValue(intToStr(temp)) ) //only update if different
			{
				*variablePtr.uByteVar = temp;
				return true;
			}
		}
		break;
		case OBJ_TYPE::TYPE_VAR_UINT:
		{
			uint_fast32_t temp = 0;
			int64_t parsed = parseInt( value );
			if ( parsed > UINT_FAST32_MAX )
			{
				temp = UINT_FAST32_MAX;
			}
			else if ( parsed > 0 )
			{
				temp = parsed;
			}
				
			if ( *variablePtr.uiVar != temp && DataField::SetFieldValue(intToStr(temp)) ) //only update if different
			{
				*variablePtr.uiVar = temp;
				return true;
			}
		}
		break;
		case OBJ_TYPE::TYPE_VAR_INT:
		{
			int_fast32_t temp = 0;
			int64_t parsed = parseInt( value );
			if ( parsed > INT_FAST32_MAX )
			{
				temp = INT_FAST32_MAX;
			}
			else if ( parsed < INT_FAST32_MIN )
			{
				temp = INT_FAST32_MIN;
			}
			else
				temp = parsed;

			if ( *variablePtr.iVar != temp && DataField::SetFieldValue(intToStr(temp)) ) //only update if different
			{
				*variablePtr.iVar = temp;
				return true;
			}
		}
		break;
		case OBJ_TYPE::TYPE_VAR_USHORT:
		{
			uint16_t temp = 0;
			int64_t parsed = parseInt( value );
			if ( parsed > UINT16_MAX )
			{
				temp = UINT16_MAX;
			}
			else if ( parsed > 0 )
			{
				temp = parsed;
			}	

			if ( *variablePtr.uiShortVar != temp && DataField::SetFieldValue(intToStr(temp)) ) //only update if different
			{
				*variablePtr.uiShortVar = temp;
				return true;
			}
		}
		break;
		case OBJ_TYPE::TYPE_VAR_LONG:
		{
			int64_t temp = parseInt( value ); //same data type as parser function
			if ( *variablePtr.lVar != temp && DataField::SetFieldValue(intToStr(temp)) ) //only update if different
			{
				*variablePtr.lVar = temp;
				return true;
			}
		}
		break;
		case OBJ_TYPE::TYPE_VAR_ULONG:
		{
			uint64_t temp = 0;
			int64_t parsed = parseInt( value );
			if ( parsed > 0 )
			{
				temp = parsed;
			}

			if ( *variablePtr.ulVar != temp && DataField::SetFieldValue(intToStr(temp)) ) //only update if different
			{
				*variablePtr.ulVar = temp;
				return true;
			}
		}
		break;
		case OBJ_TYPE::TYPE_VAR_FLOAT:
		{
			float temp = parseFloat( value );
			if ( *variablePtr.fVar != temp ) //only update if different
			{
				*variablePtr.fVar = temp;
				return true; //BUGBUG here - Updated value won't be immediately written to the datafield value (fix later)
			}
		}
		break;
		case OBJ_TYPE::TYPE_VAR_STRING:
		{
			return DataField::SetFieldValue(value);
		}
		break;
		default:
		break;
	}

	return false; //default return path
}
int_fast32_t VAR_Datafield::intFromValue()
{
	switch(iVarType)
	{
		case OBJ_TYPE::TYPE_VAR_BOOL:
			return *variablePtr.bVar;
		case OBJ_TYPE::TYPE_VAR_INT:
			return *variablePtr.iVar;
		case OBJ_TYPE::TYPE_VAR_FLOAT:
			return *variablePtr.fVar;
		case OBJ_TYPE::TYPE_VAR_LONG:
			return *variablePtr.lVar;
		case OBJ_TYPE::TYPE_VAR_UBYTE:
			return *variablePtr.uByteVar;
		case OBJ_TYPE::TYPE_VAR_USHORT:
			return *variablePtr.uiShortVar;
		case OBJ_TYPE::TYPE_VAR_STRING:
			return parseInt(GetFieldValue());
		default:
			break;
	}
	return 0;
}

String LADDER_OBJ_Datafield::GenerateHTML() 
{
 	String html = PSTR("<table>\n<thead>\n<tr>\n<th Colspan=\"2\">") + String(pObj->getID()) + PSTR("</th>\n</tr>\n<thead>\n</thead>\n<tbody>\n");
	html += PSTR("<tr>\n<th Colspan=\"2\">") + getObjectType(pObj->getType()) + PSTR("</th>\n</tr>\n");
	if ( pObj->getType() >= OBJ_TYPE::TYPE_VAR_UBYTE && pObj->getType() <= OBJ_TYPE::TYPE_VAR_STRING ) //if it's a variable type..
    {
		shared_ptr<Ladder_VAR> varPtr = static_pointer_cast<Ladder_VAR>(pObj); //declare a pointer to the variable object

		html += PSTR("<tr>\n");
		html += PSTR("<td id=\"") + pObj->getID() + varPtr->getID() + "_Type\">" + varPtr->getID() + "</td>";
		html += PSTR("<td id=\"") + pObj->getID() + varPtr->getID() + "\">" + varPtr->getValueStr() + "</td>\n"; 
		html += PSTR("</tr>\n");
	}
	else
	{
		for (uint8_t i = 0; i < pObj->getObjectVARs().size(); i++)
		{
			shared_ptr<Ladder_VAR> varPtr = pObj->getObjectVARs()[i]; //declare a pointer to the variable object

			html += PSTR("<tr>\n");
			html += PSTR("<td id=\"") + pObj->getID() + varPtr->getID() + "_Type\">" + varPtr->getID() + "</td>";
			html += PSTR("<td id=\"") + pObj->getID() + varPtr->getID() + "\">" + varPtr->getValueStr() + "</td>\n"; 
			html += PSTR("</tr>\n");
		}
	}
	
	html += PSTR("</tbody>\n</table>\n");
	return html;
}

String getObjectType(OBJ_TYPE val)
{
	String obj_type = "";
	switch (val)
	{
		case OBJ_TYPE::TYPE_INPUT:
			obj_type = inputTag1;
			break;
		case OBJ_TYPE::TYPE_INPUT_ANALOG:
			obj_type = inputTag1 + CHAR_SPACE + typeTagAnalog;
			break;
		case OBJ_TYPE::TYPE_OUTPUT:
			obj_type = outputTag1;
			break;
		case OBJ_TYPE::TYPE_OUTPUT_PWM:
			obj_type = outputTag1 + CHAR_SPACE + typeTagPWM;
			break;
		case OBJ_TYPE::TYPE_CLOCK:
			obj_type = "CLK";
			break;
		case OBJ_TYPE::TYPE_TIMER_ON:
			obj_type = typeTagTON;
			break;
		case OBJ_TYPE::TYPE_TIMER_OFF:
			obj_type = typeTagTOF;
			break;
		case OBJ_TYPE::TYPE_TIMER_RET:
			obj_type = "RETT";
			break;
		case OBJ_TYPE::TYPE_COUNTER_UP:
			obj_type = typeTagCTU;
			break;
		case OBJ_TYPE::TYPE_COUNTER_DOWN:
			obj_type = typeTagCTD;
			break;
		case OBJ_TYPE::TYPE_ONS:
			obj_type = "ONS";
			break;
		case OBJ_TYPE::TYPE_MATH_EQ:
			obj_type = typeTagMEQ;
			break;
		case OBJ_TYPE::TYPE_MATH_GRT:
			obj_type = typeTagMGRE;
			break;
		case OBJ_TYPE::TYPE_MATH_LES:
			obj_type = typeTagMLES;
			break;
		case OBJ_TYPE::TYPE_MATH_GRQ:
			obj_type = typeTagMGREE;
			break;
		case OBJ_TYPE::TYPE_MATH_LEQ:
			obj_type = typeTagMLESE;
			break;
		case OBJ_TYPE::TYPE_MATH_SIN:
			obj_type = typeTagMSIN;
			break;
		case OBJ_TYPE::TYPE_MATH_COS:
			obj_type = typeTagMCOS;
			break;
		case OBJ_TYPE::TYPE_MATH_TAN:
			obj_type = typeTagMTAN;
			break;
		case OBJ_TYPE::TYPE_MATH_ASIN:
			obj_type = typeTagMASIN;
			break;
		case OBJ_TYPE::TYPE_MATH_ACOS:
			obj_type = typeTagMACOS;
			break;
		case OBJ_TYPE::TYPE_MATH_ATAN:
			obj_type = typeTagMATAN;
			break;
		case OBJ_TYPE::TYPE_MATH_LIMIT:
			obj_type = "MLIM";
			break;
		case OBJ_TYPE::TYPE_MATH_INC:
			obj_type = typeTagMINC;
			break;
		case OBJ_TYPE::TYPE_MATH_DEC:
			obj_type = typeTagMDEC;
			break;
		case OBJ_TYPE::TYPE_MATH_CPT:
			obj_type = "CPT";
			break;
		case OBJ_TYPE::TYPE_MATH_MOV:
			obj_type = typeTagMMOV;
			break;
		case OBJ_TYPE::TYPE_VAR_BOOL:
		case OBJ_TYPE::TYPE_VAR_STRING:
		case OBJ_TYPE::TYPE_VAR_FLOAT:
		case OBJ_TYPE::TYPE_VAR_INT:
		case OBJ_TYPE::TYPE_VAR_LONG:
		case OBJ_TYPE::TYPE_VAR_UBYTE:
		case OBJ_TYPE::TYPE_VAR_USHORT:
		case OBJ_TYPE::TYPE_VAR_UINT:
		case OBJ_TYPE::TYPE_VAR_ULONG:
			obj_type = variableTag1;
			break;
	
		default:
			obj_type = "N/A";
			break;
	}

	return obj_type;
}