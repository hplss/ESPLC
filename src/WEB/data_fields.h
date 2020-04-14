/*
 * data_fields.h
 *
 * Created: 2/2/2018 11:26:45 AM
 *  Author: Andrew Ward
 * This header file contains the definitions for all functions related to the 
 */ 

#ifndef DATA_FIELDS_H_
#define DATA_FIELDS_H_

#define MAX_DATA_LENGTH 16
#define MAX_NAME_LENGTH 16 

#include <WiFi.h>
#include <memory>

enum FIELD_TYPE : uint8_t
{
	NONE = 0, 
	RADIO, //This field type creates a radio button for the web UI, and is typically reserved for boolean operations.
	TEXT, //This field type represents a single line text field. Typically reserved for entering small strings of data.
	SUBMIT, //This field type is a special type that is reserved for the POST/GET method. Takes the form of a button. 
	CHECKBOX, //This field type creates a checkbox item for the web UI, and is typically reserved for boolean operations.
	PASSWORD, //This special field type represents a regular single line text box, but the characters are obscured as they are entered.
	SELECT, 
	TEXTAREA, //This field type creates a large text area (multiple lines) in which a user can enter long messages.
	HYPERLINK //This special field type is used for creating hyperlinks that redirect to other pages.
};

class UICore; //predefinition for linker purposes

class DataField
{
	public:
	DataField( uint8_t address, uint8_t type, const String &fieldLabel = "", const String &defaultValue = "", bool newLine = true, bool functional = false ) :
	DataField( make_shared<String>(defaultValue), address, type, fieldLabel, newLine, functional ){}

	DataField( shared_ptr<String>defaultValue, uint8_t address, uint8_t type, const String &fieldLabel = "", bool newLine = true, bool functional = false )
	{
		//settings = ( type : 4 << 4 ) | (address >> 8) | ( functional << 1) | ( 1 << 1) /*enabled*/ | ( newLine << 1 ); //pack all the bits up. 
		/*SetSetting(newLine, 1, 0);
		SetSetting(true, 1, 1); //enabled
		SetSetting(functional, 1, 2);
		SetSetting(address, 8, 3);
		SetSetting(type, 4, 11);*/
		i_Address = address;
		i_Type = type;
		s_fieldLabel = fieldLabel; //Labels are the describing text
		s_fieldValue = defaultValue;
		b_enabled = true;
		b_newLine = newLine;
		b_Function = functional; //always default off
	}
	virtual ~DataField(){} //Destructor

	//Returns true if the DataField is enabled, otherwise false.
	bool IsEnabled(){ return b_enabled; }
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
		//uint8_t addr = GetSetting(8,3);
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
	uint8_t i_Type : 4; //This represents the type of data field we're displaying (Text-box, radio button, etc)
	uint8_t i_Address; //Represents the address number used for updating values in the field.	
	shared_ptr<String> s_fieldValue; //This is the data being displayed within the form (default text in a text-box for example)
	String s_fieldLabel; //Text that describes the field
	bool b_Function : 1; //kinda hackish, but this is used to determine if this field performs a direct function call. This will look better with bit shifting
	/*void SetSetting(uint8_t setting, uint8_t numBits, uint8_t startAddr)
	{
		for (uint8_t x = 0; x < numBits; x++)
			settings |= (setting >> x) << (x + startAddr);
	}
	uint8_t GetSetting(uint8_t numBits, uint8_t startAddr)
	{
		uint8_t setting = 0;
		for (uint8_t x = 0; x < numBits; x++)
			setting |= ((settings >> (x + startAddr)) & 1) << x;
		return setting;
	}*/
	
	bool b_enabled : 1; //Is this field enabled? 
	bool b_newLine : 1; //Generate a newline in HTML following this Datafield.

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

//The UINT_Datafield is a DataField that is responsible for handling all code related to storing and modifying data of type UINT (unsigned 16-bit integer).
class UINT_Datafield : public DataField
{
	public:
	UINT_Datafield( unsigned int *var, uint8_t address, uint8_t type, const String &fieldLabel = "", bool newLine = true, bool functional = false) :
	DataField( address, type, fieldLabel, String(*var), newLine )
	{
		fieldVar = var;
	}
	~UINT_Datafield(){}
	
	unsigned int *GetVar(){ return fieldVar; }
	bool SetFieldValue( const unsigned int & );
	bool SetFieldValue( const String & );
	
	private:
	unsigned int *fieldVar;
};

//The UINT8_Datafield is a DataField that is responsible for handling all code related to storing and modifying data of type UINT8 (unsigned 8-bit integer).
class UINT8_Datafield : public DataField
{
	public:
	UINT8_Datafield( uint8_t *var, uint8_t address, uint8_t type, const String &fieldLabel = "", bool newLine = true, bool functional = false) :
	DataField( address, type, fieldLabel, String(*var), newLine )
	{
		fieldVar = var;
	}
	~UINT8_Datafield(){}
	
	uint8_t *GetVar(){ return fieldVar; }
	bool SetFieldValue( const uint8_t & );
	bool SetFieldValue( const String & );
	
	private:
	uint8_t *fieldVar;
};

//The BOOL_Datafield is a DataField that is responsible for handling all code related to storing and modifying data of type BOOL.
class BOOL_Datafield : public DataField
{
	public:
	BOOL_Datafield( bool *var, uint8_t address, uint8_t type, const String &fieldLabel = "", bool newLine = true, bool functional = false) :
	DataField( address, type, fieldLabel, String(*var), newLine )
	{
		fieldVar = var;
	}
	~BOOL_Datafield(){}
	
	bool *GetVar(){ return fieldVar; }
	bool SetFieldValue( const bool &b );
	bool SetFieldValue( const String &str );
		
	private:
	bool *fieldVar;
};

//The STRING_Datafield is a DataField that is responsible for handling all code related to storing and modifying data of type STRING.
class STRING_Datafield : public DataField
{
	public:
	STRING_Datafield( shared_ptr<String> var, uint8_t address, uint8_t type, const String &fieldLabel = "", bool newLine = true, bool functional = false ) :
	DataField( var, address, type, fieldLabel, newLine, functional )
	{
	}
	~STRING_Datafield(){}
	String GenerateHTML();
};


//The STRING_S_Datafield is a DataField that is responsible for handling all code related to storing and modifying data of type STRING.
//This object also calls another function once the value of the DataField has been altered (successfully).
class STRING_S_Datafield : public STRING_Datafield
{
	public:
	STRING_S_Datafield( const function<void(void)> &onChanged, shared_ptr<String> var, uint8_t address, uint8_t type, const String &fieldLabel = "", bool newLine = true ) :
	STRING_Datafield( var, address, type, fieldLabel, newLine, true )
	{
		func = onChanged;
	}
	~STRING_S_Datafield(){}
	bool SetFieldValue( const String &str ){ if(DataField::SetFieldValue(str)){ func(); return true; } return false; }
	
	private:
	function<void(void)> func;//function reference to be executed on change
};

//The UINT_S_Datafield is a DataField that is responsible for handling all code related to storing and modifying data of type UINT (unsigned 16-bit integer).
//This object also calls another function once the value of the DataField has been altered (successfully).
class UINT_S_Datafield : public UINT_Datafield
{
	public:
	UINT_S_Datafield( const function<void(void)> &onChanged, unsigned int *var, uint8_t address, uint8_t type, const String &fieldLabel = "", bool newLine = true ) :
	UINT_Datafield( var, address, type, fieldLabel, newLine, true )
	{
		func = onChanged;
	}
	
	bool SetFieldValue( const String &str ){ if(UINT_Datafield::SetFieldValue(str)){ func(); return true; } return false; }
	
	private:
	function<void(void)> func;
};

class UINT8_S_Datafield : public UINT8_Datafield
{
	public:
	UINT8_S_Datafield( const function<void(void)> &onChanged, uint8_t *var, uint8_t address, uint8_t type, const String &fieldLabel = "", bool newLine = true ) :
	UINT8_Datafield( var, address, type, fieldLabel, newLine, true )
	{
		func = onChanged;
	}
	
	bool SetFieldValue( const String &str ){ if(UINT8_Datafield::SetFieldValue(str)){ func(); return true; } return false; }
	
	private:
	function<void(void)> func;
};

class BOOL_S_Datafield : public BOOL_Datafield
{
	public:
	BOOL_S_Datafield( const function<void(void)> &onChanged, bool *var, uint8_t address, uint8_t type, const String &fieldLabel = "", bool newLine = true ) :
	BOOL_Datafield( var, address, type, fieldLabel, newLine, true )
	{
		func = onChanged;
	}
	
	bool SetFieldValue( const String &str ){ if(BOOL_Datafield::SetFieldValue(str)){ func(); return true; } return false; }
	
	private:
	function<void(void)> func;
};

//This field is used specifically for selecting and setting the device WiFi SSID for direct connection vie the web UI.
class SSID_Datafield : public DataField
{
	public:
	SSID_Datafield(uint8_t address, const String &fieldLabel = "", bool newLine = true ) :
	DataField( address, FIELD_TYPE::SELECT, fieldLabel, WiFi.SSID(), newLine )
	{
	}
	~SSID_Datafield(){}

	String GenerateHTML(); //See Data_fields.cpp
};

#endif /* DATA_FIELDS_H_ */