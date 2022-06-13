#ifndef PLC_PARSER_H_
#define PLC_PARSER_H_

#include <map>
#include "./OBJECTS/obj_var.h"

using namespace std;

struct NestContainer
{
	NestContainer( uint8_t pTier, uint8_t orTier )
	{
		i_pTier = pTier;
		i_orTier = orTier;
	}

	const vector<OBJ_WRAPPER_PTR> &getEQObjects(){ return eqObjects; }
	const vector<OBJ_WRAPPER_PTR> &getORObjects(){ return orObjects; }
	const vector<OBJ_WRAPPER_PTR> &getANDObjects(){ return andObjects; }
	
	void setLastObjects( const vector<OBJ_WRAPPER_PTR> &vec ){ lastObjects = vec; }
	uint8_t getNumORObjects(){ return orObjects.size(); }
	uint8_t getNumANDObjects(){ return andObjects.size(); }
	uint8_t getNumEQObjects(){ return eqObjects.size(); }

	bool addORObject( OBJ_WRAPPER_PTR obj)
	{ 
		if (obj) 
			orObjects.push_back(obj); //ELSE ERROR 
		else
			return false;

		return true;
	}
	bool addANDObject( OBJ_WRAPPER_PTR obj)
	{ 
		if (obj) 
			andObjects.push_back(obj); //ELSE ERROR 
		else
			return false;
			
		return true;
	}
	bool addEQObject( OBJ_WRAPPER_PTR obj)
	{ 
		if (obj) 
			eqObjects.push_back(obj); //ELSE ERROR 
		else
			return false;
			
		return true;
	}
	const uint8_t getPTier(){ return i_pTier; }
	const uint8_t getOrTier(){ return i_orTier; }

	//This function interconnects the objects logically, based on their type. 
	//It also generated the lists for the first and last objects that need to be referenced later, for connecting between tiers.
	bool connectObjects();
	
	//Get a reference to the lastObjects vector. 
	vector<OBJ_WRAPPER_PTR> &getLastObjects(){ return lastObjects; }
	//Get a reference to the firstObjects vector.
	vector<OBJ_WRAPPER_PTR> &getFirstObjects(){ return firstObjects; }
	//Returns a reference to the storage for the NestContainers that are referenced by the current NestContainer (as dictated by the parser).
	const vector<NEST_PTR> &getNextNests(){ return pNextNestContainers; }

	//Returns the number of NestContainers that are referenced by the current NestContainer (as dictated by the parser).
	uint8_t getNumNextNests(){ return pNextNestContainers.size(); }

	void addNextNestContainer( NEST_PTR ptr)
	{ 
		//Serial.println( "Nest at: "+ String(getPTier()) + ":" + String(getOrTier()) + " is adding: " + String(ptr->getPTier()) + ":" + String(ptr->getOrTier()) );
		pNextNestContainers.push_back(ptr); 
	}

	private:
	uint8_t i_pTier, i_orTier; //Nest Identifiers 
	vector<OBJ_WRAPPER_PTR> eqObjects, andObjects, orObjects, lastObjects, firstObjects; //Our little gaggle of vectors
	vector<NEST_PTR> pNextNestContainers; //Storage for the NestContainers that are referenced by the current NestContainer (as dictated by the parser).
};

/* PLC_Parser serves as a container and accessor for variables that are used for establishing ladder logic object relationships in the parser. */
struct PLC_Parser
{
public:
	PLC_Parser( const String &parsed, uint16_t rung )
	{
		/*create a new ladder rung when the helper is initialized. 
		Presumably each helper represents a line being parsed, and each line represents a "rung" in the ladder logic.*/
		pRung = make_shared<Ladder_Rung>(); 
		bitNot = false; 
		sParsedLine = parsed;
		iRung = rung;
	}

	//Forwards an error of a given type (with additional info message as second argument) to the client.
    void sendError(ERR_DATA, const String & = "");
    //Parses the individual lines for logic operations and declarations (called from parseScript())
	bool parseLine();
	//This function breaks up the arguments that are passed in during object declaration and instantiation. Putting them into a vector of Strings
    vector<String> parseObjectArgs();
	//Resets the values typically used by the parser to their default values.
	void reset(){ sParsedAccessor.clear(); sParsedObj.clear(); sParsedBit.clear(); sParsedArgs.clear(); bitNot = false; }
	
	//Attempts to create a new object wrapper using an already defined ladder object, applies necessary wrapper flags, and returns the created object.
	OBJ_WRAPPER_PTR getObjectVARWrapper( OBJ_LOGIC_PTR );
	//This is responsible for generating the object wrapper by associating it with the (already initialized) object. May return a wrapper for the object itself, or an associated bit in the object.
	OBJ_WRAPPER_PTR createNewWrapper( OBJ_LOGIC_PTR );

	void setNotOP( bool bit ){ bitNot = bit; }

	//This function is responsible for determining what to do with an object that was parsed in the logic script. 
    OBJ_WRAPPER_PTR handleObject();
	
	const String &getParsedObjectStr(){ return sParsedObj; } 
	const String &getParsedBitStr(){ return sParsedBit; } 
	const String &getParsedArgs(){ return sParsedArgs; } 
	const String &getParsedAccessorStr(){ return sParsedAccessor; }

	vector<NEST_PTR> &getNests() { return nestData; }
	vector<NEST_PTR> getNestsByTier( uint8_t tier );
	uint8_t getHighestNestTier();
	
	//This function returns a vector containing all objects that are the last to be referenced across all NestContainer objects
	vector<OBJ_WRAPPER_PTR> getLastNestObjects();
	//This function gathers a list of the first object's that are to be referenced by the PLC_Rung object when beginning a logic scan.
	vector<OBJ_WRAPPER_PTR> getFirstNestObjects();

	bool buildObjectStr( const String & );

	shared_ptr<Ladder_Rung> &getRung(){ return pRung; }
	bool getNotOP(){ return bitNot; }
	uint16_t getRungNum(){ return iRung; } 
	uint16_t getLineLength() { return sParsedLine.length(); }
	const String &getParsedLineStr(){ return sParsedLine; }

private:
	bool bitNot; 
	uint16_t iLinePos; //position at which the line is currently being parsed (in the string character array)
	uint16_t iLineLength; //total length of the parsed line (number of chars)
	uint16_t iRung; 
	String sParsedLine,
		   sParsedArgs, //object arguments (if applicable)
		   sParsedObj, //object name 
		   sParsedAccessor, //name of peripheral component being accessed
		   sParsedBit; //for bit operations on objects

	shared_ptr<Ladder_Rung> pRung; //Rung object that is being created by the parser
	vector<NEST_PTR> nestData;
};

//This object is responsible for breaking up an inputted line based on the different types of operators that we are using for the PLC. 
//Parenthetical tiers are also processed by recursively creating a new object from within the current object's constructor. 
//In this sense, each newly created object represents the data that has been parsed for each specific parenthetical tier. 
struct LogicObject
{
	public:
	LogicObject(const String &line, PLC_Parser *parser, uint8_t pTier = 0, uint8_t orTier = 0);
	//Deconstructor
	~LogicObject(){}

	NEST_PTR getNestContainer(){ return pNestContainer; }

	private: 
	NEST_PTR pNestContainer;
};
#endif