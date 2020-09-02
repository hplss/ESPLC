/*
 * data_fields.h
 *
 * Created: 2/2/2018 11:26:45 AM
 *  Author: Andrew Ward
 * This header file contains the definitions for all functions related to the 
 */ 

#ifndef DATA_FIELDS_H_
#define DATA_FIELDS_H_

#define MAX_DATA_LENGTH 24
#define MAX_NAME_LENGTH 16 

#include <WiFi.h>
#include <memory>
#include <initializer_list>
#include "PLC/PLC_IO.h"

enum FIELD_TYPE : uint8_t
{
	NONE = 0, 
	RADIO, //This field type creates a radio button for the web UI, and is typically reserved for boolean operations.
	TEXT, //This field type represents a single line text field. Typically reserved for entering small strings of data.
	NUMBER, //This field type is for inputting numbers only. 
	SUBMIT, //This field type is a special type that is reserved for the POST/GET method. Takes the form of a button. 
	CHECKBOX, //This field type creates a checkbox item for the web UI, and is typically reserved for boolean operations.
	PASSWORD, //This special field type represents a regular single line text box, but the characters are obscured as they are entered.
	SELECT, //This special field is used for creating a field that allows for a drop-down selection menu.
	TEXTAREA, //This field type creates a large text area (multiple lines) in which a user can enter long messages.
	HYPERLINK //This special field type is used for creating hyperlinks that redirect to other pages.
};

class UICore; //predefinition for linker purposes

class DataField
{
	public:
	DataField( uint8_t address, uint8_t type, const String &fieldLabel = "", const String &defaultValue = "", uint8_t cols = MAX_DATA_LENGTH, uint8_t rows = 1, bool newLine = true, bool functional = false ) :
	DataField( make_shared<String>(defaultValue), address, type, fieldLabel, cols, rows, newLine, functional ){}

	DataField( shared_ptr<String>defaultValue, uint8_t address, uint8_t type, const String &fieldLabel = "", uint8_t cols = MAX_DATA_LENGTH, uint8_t rows = 1, bool newLine = true, bool functional = false )
	{
		i_Address = address;
		i_Type = type;
		s_fieldLabel = fieldLabel; //Labels are the describing text
		s_fieldValue = defaultValue;
		b_newLine = newLine;
		b_Function = functional; //always default off
		iRows = rows; 
		iCols = cols;
	}
	virtual ~DataField(){} //Destructor

	//Returns the field type (used for the generation of the HTML code)
	uint8_t GetType() { return i_Type; } 
	//This overloaded function is used for setting the data (stored value) within the field.
	virtual bool SetFieldValue( shared_ptr<String> ); 
	//This overloaded function is used for setting the data (stored value) within the field.
	virtual bool SetFieldValue( const String & );
	//void SetEnabled( bool en ){ SetSetting(en, 1, 1); /*b_enabled = en;*/ }

	//Returns the address of the field (Used to make sure we're updating the proper field)
	uint8_t GetAddress()
	{
		return i_Address;
	}
	String GetFieldName() { return String(GetAddress()); }
	//Returns the string containing the value of the data field (the actual data).
	const String &GetFieldValue() { return *s_fieldValue; }
	//Returns the string containing the label used to describe the data field.
	const String &GetFieldLabel() { return s_fieldLabel; }
	//Used to create the HTML as it corresponds each web UI data field. 
	//Returns a string that contains all of the generated HTML, which can then be appended to the main HTML string before being sent to the end user.
	virtual String GenerateHTML(); 
	//Create a new line?
	bool DoNewline(){ return b_newLine; }
	bool UsesFunction(){ return b_Function; }

	private:
	uint8_t i_Type; //This represents the type of data field we're displaying (Text-box, radio button, etc)
	uint8_t i_Address; //Represents the address number used for updating values in the field.	
	shared_ptr<String> s_fieldValue; //This is the data being displayed within the form (default text in a text-box for example)
	String s_fieldLabel; //Text that describes the field
	bool b_Function; //kinda hackish, but this is used to determine if this field performs a direct function call. This will look better with bit shifting
	bool b_newLine; //Generate a newline in HTML following this Datafield.
	uint8_t iRows, iCols; 

	unsigned int settings; //variables that mutliple settings are shifted into

	//Bit shifting idea: unsigned int(16 bit) = (type = 4 bits), (address = 8 bits), (function = 1 bit), (enabled = 1 bit), (new line = 1 bit) = 15 bits total
};

class DataTable //This class is basically used to create sections for specific types of inputs, like SQL settings, or Time settings, etc.
{
	public:
	DataTable( const String &name = "" )
	{
		s_tableName = name;
	}
	~DataTable()
	{
		p_fields.clear(); //empty the vector - should delete the containd objects
	}
	
	//Sets the name of the table object, which then corresponds to the table's title text generated in HTML.
	void SetTableName( const String &name ) { s_tableName = name; }
	 //Generates and returns the HTML for the Table object.
	String GenerateTableHTML();
	//Removes an element (DataField) from the table object 
	bool RemoveElement( unsigned int );
	//Adds a DataField object to the vector container for the table object.
	bool AddElement( shared_ptr<DataField> );
	//Used to search for an object with the inputted address. If that object is found, the position in the vector is returned, else -1
	int8_t IteratorFromAddress( unsigned int ); 
	//Returns the element (DataField) with the corresponding ID
	//Args: data field ID
	shared_ptr<DataField> GetElementByID( unsigned int ); 
	//returns the element (DataField) by its assigned name.
	//Args: data field name
	shared_ptr<DataField> GetElementByName( const String & ); 
	//Returns the table object's vector containing the DataField objects.
	const vector<shared_ptr<DataField>> &GetFields(){ return p_fields; }
		
	private: 
	String s_tableName;
	//vector containing the shared pointers to all of our DataFields (for the table object).
	vector<shared_ptr<DataField>> p_fields; 
};

//The Hyperlink_Datafield is a DataField that is responsible for handling and generating the HTML code that is needed for embedded hyperlinks in the pages for the web UI.
class Hyperlink_Datafield : public DataField
{
	public:
	Hyperlink_Datafield( uint8_t address, uint8_t type, const String &fieldLabel, const String &link, bool newLine = true ) : 
	DataField( address, type, fieldLabel, link, newLine )
	{
	}
	~Hyperlink_Datafield(){}
	String GenerateHTML(); //special for hyperlinks

	private:
};

class VAR_Datafield : public DataField
{
	public: 
	VAR_Datafield( bool *var, uint8_t address, uint8_t type, const String &fieldLabel = "", uint8_t cols = MAX_DATA_LENGTH, uint8_t rows = 1, bool newLine = true, bool functional = false ) : DataField( address, type, fieldLabel, String(*var), cols, rows, newLine, functional )
	{ variablePtr.bVar = var; iVarType = TYPE_VAR_BOOL; }
	VAR_Datafield( uint16_t *var, uint8_t address, uint8_t type, const String &fieldLabel = "", uint8_t cols = MAX_DATA_LENGTH, uint8_t rows = 1, bool newLine = true, bool functional = false ) : DataField( address, type, fieldLabel, String(*var), cols, rows, newLine, functional )
	{ variablePtr.uiShortVar = var; iVarType = TYPE_VAR_USHORT; }
	VAR_Datafield( float *var, uint8_t address, uint8_t type, const String &fieldLabel = "", uint8_t cols = MAX_DATA_LENGTH, uint8_t rows = 1, bool newLine = true, bool functional = false ) : DataField( address, type, fieldLabel, String(*var), cols, rows, newLine, functional )
	{ variablePtr.fVar = var; iVarType = TYPE_VAR_FLOAT; }
	VAR_Datafield( int_fast32_t *var, uint8_t address, uint8_t type, const String &fieldLabel = "", uint8_t cols = MAX_DATA_LENGTH, uint8_t rows = 1, bool newLine = true, bool functional = false ) : DataField( address, type, fieldLabel, String(*var), cols, rows, newLine, functional )
	{ variablePtr.iVar = var; iVarType = TYPE_VAR_INT; }
	VAR_Datafield( uint_fast32_t *var, uint8_t address, uint8_t type, const String &fieldLabel = "", uint8_t cols = MAX_DATA_LENGTH, uint8_t rows = 1, bool newLine = true, bool functional = false ) : DataField( address, type, fieldLabel, String(*var), cols, rows, newLine, functional )
	{ variablePtr.uiVar = var; iVarType = TYPE_VAR_UINT; }
	VAR_Datafield( uint8_t *var, uint8_t address, uint8_t type, const String &fieldLabel = "", uint8_t cols = MAX_DATA_LENGTH, uint8_t rows = 1, bool newLine = true, bool functional = false ) : DataField( address, type, fieldLabel, String(*var), cols, rows, newLine, functional )
	{ variablePtr.uByteVar = var; iVarType = TYPE_VAR_UBYTE; }
	VAR_Datafield( uint64_t *var, uint8_t address, uint8_t type, const String &fieldLabel = "", uint8_t cols = MAX_DATA_LENGTH, uint8_t rows = 1, bool newLine = true, bool functional = false ) : DataField( address, type, fieldLabel, uLongToStr(*var), cols, rows, newLine, functional )
	{ variablePtr.ulVar = var; iVarType = TYPE_VAR_ULONG; }
	VAR_Datafield( int64_t *var, uint8_t address, uint8_t type, const String &fieldLabel = "", uint8_t cols = MAX_DATA_LENGTH, uint8_t rows = 1, bool newLine = true, bool functional = false ) : DataField( address, type, fieldLabel, longToStr(*var), cols, rows, newLine, functional )
	{ variablePtr.lVar = var; iVarType = TYPE_VAR_LONG; }
	VAR_Datafield( shared_ptr<String> var, uint8_t address, uint8_t type, const String &fieldLabel = "", uint8_t cols = MAX_DATA_LENGTH, uint8_t rows = 1, bool newLine = true, bool functional = false ) : DataField( var, address, type, fieldLabel, cols, rows, newLine, functional )
	{ iVarType = TYPE_VAR_STRING; }
	~VAR_Datafield(){};

	bool SetFieldValue( const String &str );
	int_fast32_t intFromValue();

	private:
	union
	{
		bool *bVar;
		float *fVar;
		uint8_t *uByteVar;
		uint16_t *uiShortVar;
		int_fast32_t *iVar; //signed int
		uint_fast32_t *uiVar; //unsigned int
		int64_t *lVar;
		uint64_t *ulVar;
	} variablePtr;

	uint8_t iVarType;
};

class VAR_S_Datafield : public VAR_Datafield
{
	public:
	template <typename T>
	VAR_S_Datafield(const function<void(void)> &onChanged, T *var, uint8_t address, uint8_t type, const String &fieldLabel = "", uint8_t cols = MAX_DATA_LENGTH, uint8_t rows = 1, bool newLine = true ) : VAR_Datafield( var, address, type, fieldLabel, cols, rows, newLine, true )
	{ func = onChanged; }
	template <typename T>
	VAR_S_Datafield(const function<void(void)> &onChanged, shared_ptr<T> var, uint8_t address, uint8_t type, const String &fieldLabel = "", uint8_t cols = MAX_DATA_LENGTH, uint8_t rows = 1, bool newLine = true ) : VAR_Datafield( var, address, type, fieldLabel, cols, rows, newLine, true )
	{ func = onChanged; }
	~VAR_S_Datafield(){}
	bool SetFieldValue( const String &str ){ if(VAR_Datafield::SetFieldValue(str)){ func(); return true; } return false; }

	private:
	function<void(void)> func;//function reference to be executed on change
};

//This field is used specifically for selecting and setting the device WiFi SSID for direct connection vie the web UI.
class SSID_Datafield : public DataField
{
	public:
	SSID_Datafield( shared_ptr<String> str, uint8_t address, const String &fieldLabel = "", bool newLine = true) :
	DataField( str, address, FIELD_TYPE::SELECT, fieldLabel, 32, 1, newLine )
	{
	}
	~SSID_Datafield(){}

	String GenerateHTML(); //See Data_fields.cpp
	bool SetFieldValue( const String &str )
	{
		return DataField::SetFieldValue(WiFi.SSID(str.toInt())); //translate integer values to the strings as it relates to those stored in the WiFi scanner.
	}

	private:
};

class Select_Datafield : public VAR_Datafield
{
	public:
	template <typename T>
	Select_Datafield( T *var, uint8_t address, const String &fieldLabel = "", bool newLine = true ) : 
	VAR_Datafield( var, address, FIELD_TYPE::SELECT, fieldLabel, newLine )
	{
	}
	~Select_Datafield(){}

	String GenerateHTML();
	//Used for adding an option to the selection list.
	void addOption(const String &str){ options.push_back(str);}
	private:
	vector<String> options;
};

//This field type is used for generating the status objects per each initialized ladder object. 
class LADDER_OBJ_Datafield : public DataField
{
	public:
	LADDER_OBJ_Datafield( shared_ptr<Ladder_OBJ> obj, uint8_t address, const String &fieldLabel = "", bool newLine = true ) : 
	DataField(address, FIELD_TYPE::NONE, fieldLabel, "", newLine )
	{
		pObj = obj;
	}
	~LADDER_OBJ_Datafield()
	{
	}

	String GenerateHTML();
	private:
	shared_ptr<Ladder_OBJ> pObj;
};

#endif /* DATA_FIELDS_H_ */