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
	NONE = 0, RADIO, TEXT, SUBMIT, CHECKBOX, PASSWORD, SELECT, TEXTAREA, HYPERLINK
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

	bool IsEnabled(){ return b_enabled; }
	//Returns the field type (used for the generation of the HTML code)
	uint8_t GetType() { return i_Type; } 
	virtual bool SetFieldValue( shared_ptr<String> ); //This is used for setting the data within the field.
	virtual bool SetFieldValue( const String & );
	//void SetEnabled( bool en ){ SetSetting(en, 1, 1); /*b_enabled = en;*/ }
	//returns the address of the field (Used to make sure we're updating the proper field)
	uint8_t GetAddress()
	{
		//uint8_t addr = GetSetting(8,3);
		return i_Address;
	}
	String GetFieldName() { return String(GetAddress()); }
	const String &GetFieldValue() { return *s_fieldValue; }
	const String &GetFieldLabel() { return s_fieldLabel; }
	virtual String GenerateHTML(); //Used to create the HTML to be appended to the body of the web page.
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
	
	void SetTableName( const String &name ) { s_tableName = name; }
	String GenerateTableHTML(); //Generates the HTML for the Table.
	bool RemoveElement( unsigned int );
	bool AddElement( shared_ptr<DataField> );
	int8_t IteratorFromAddress( unsigned int ); //Used to search for an object with the inputted address. If that object is found, the position in the vector is returned, else -1
	shared_ptr<DataField> GetElementByID( unsigned int ); //Retrieves the element with the corresponding ID
	shared_ptr<DataField> GetElementByName( const String & ); //Get the element by its assigned name.
	const vector<shared_ptr<DataField>> &GetFields(){ return p_fields; }
		
	private: 
	String s_tableName;
	vector<shared_ptr<DataField>> p_fields; //vector containing the pointers to all of our data fields (regular fields only).
};

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

//SPECIAL DATAFIELDS - For directly modifying settings.
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


//Reactive Settings (They execute a function upon value being changed)
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

//This field is used specifically for selecting and setting the device wifiSSID 
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