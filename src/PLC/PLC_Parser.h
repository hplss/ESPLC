#ifndef PLC_PARSER_H_
#define PLC_PARSER_H_

#include <map>

using namespace std;


struct NestContainer
{
	NestContainer( uint8_t pTier, uint8_t orTier )
	{
		i_pTier = pTier;
		i_orTier = orTier;
	}

	const vector<shared_ptr<Ladder_OBJ_Wrapper>> &getEQObjects(){ return eqObjects; }
	const vector<shared_ptr<Ladder_OBJ_Wrapper>> &getORObjects(){ return orObjects; }
	const vector<shared_ptr<Ladder_OBJ_Wrapper>> &getANDObjects(){ return andObjects; }
	void setLastObjects( const vector<shared_ptr<Ladder_OBJ_Wrapper>> &vec ){ lastObjects = vec; }
	uint8_t getNumORObjects(){ return orObjects.size(); }
	uint8_t getNumANDObjects(){ return andObjects.size(); }
	uint8_t getNumEQObjects(){ return eqObjects.size(); }

	bool addORObject( shared_ptr<Ladder_OBJ_Wrapper> obj)
	{ 
		if (obj) 
			orObjects.push_back(obj); //ELSE ERROR 
		else
			return false;

		return true;
	}
	bool addANDObject( shared_ptr<Ladder_OBJ_Wrapper> obj)
	{ 
		if (obj) 
			andObjects.push_back(obj); //ELSE ERROR 
		else
			return false;
			
		return true;
	}
	bool addEQObject( shared_ptr<Ladder_OBJ_Wrapper> obj)
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
	bool connectObjects()
	{
		//Connect the series objects first.
		for ( uint8_t p = 0; p < getNumANDObjects(); p++)
		{
			if ( (p+1) < getNumANDObjects() )
			{
				if ( !andObjects[p]->addNextObject(andObjects[p+1]) ) //logically connect the current to the next
					return false;
			}
		}

		//If we have some assignment (=) objects, make sure to factor those into the logic as needed.
		for ( uint8_t y = 0; y < getNumEQObjects(); y++ )
		{
			if ( getNumANDObjects() )
				andObjects.back()->addNextObject(eqObjects[y]); //append to the end of the series 

			for ( uint8_t x = 0; x < orObjects.size(); x++ )
			{
				orObjects[x]->addNextObject(eqObjects[y]); //also append to any parallel operations that we have.
			}
		}

		//Generate the lastObjects vector;
		if ( getNumEQObjects() )
			lastObjects = eqObjects;

		else if ( getNumORObjects() || getNumANDObjects() )
		{
			lastObjects = orObjects; 
			if ( getNumANDObjects() )
				lastObjects.push_back(andObjects.back());
		}
		//

		//generate the firstObjects vector
		if ( getNumORObjects() || getNumANDObjects() )
		{
			firstObjects = orObjects; //parallel objects are always "first"
			if ( getNumANDObjects() ) //have some AND objects? 
				firstObjects.push_back(andObjects.front()); //only use the first AND object (because the others are in series with it)
		}
		else if ( getNumEQObjects() )//if nothing else.. though syntactically this doesn't make sense.. we'll allow it
			firstObjects = eqObjects;
		//


		return true;
	}

	//Get a reference to the lastObjects vector. 
	vector<shared_ptr<Ladder_OBJ_Wrapper>> &getLastObjects(){ return lastObjects; }
	//Get a reference to the firstObjects vector.
	vector<shared_ptr<Ladder_OBJ_Wrapper>> &getFirstObjects(){ return firstObjects; }
	const vector<shared_ptr<NestContainer>> &getNextNests(){ return pNextNestContainers; }

	uint8_t getNumNextNests(){ return pNextNestContainers.size(); }

	void addNextNestContainer( shared_ptr<NestContainer> ptr)
	{ 
		//Serial.println( "Nest at: "+ String(getPTier()) + ":" + String(getOrTier()) + " is adding: " + String(ptr->getPTier()) + ":" + String(ptr->getOrTier()) );
		pNextNestContainers.push_back(ptr); 
	}



	private:
	uint8_t i_pTier, i_orTier; //Nest Identifiers (does i_orTier do anything these days? Hmm)
	vector<shared_ptr<Ladder_OBJ_Wrapper>> eqObjects, andObjects, orObjects, lastObjects, firstObjects; //Our little gaggle of vectors
	vector<shared_ptr<NestContainer>> pNextNestContainers; //Storage for the NestContainers that are referenced by the current NestContainer (as dictated by the parser).
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
		bitOperator = false;
		bitNot = false; 
		argsOP = false;
		sParsedLine = parsed;
		iRung = rung;
	}

	//Forwards an error of a given type (with additional info message as second argument) to the client.
    void sendError(uint8_t, const String & = "");

    //Parses the individual lines for logic operations and declarations (called from parseScript())
	bool parseLine();

	//This function breaks up the arguments that are passed in during object declaration and instantiation. Putting them into a vector of Strings
    vector<String> parseObjectArgs();

	//Appends an inputted char to the string that stores the given object's name.
	void appendToObjName( const char c ) { sParsedObj += c; }
	//Appends an inputted char to the string that stores the given object's bit name.
	void appendToBitName( const char c ){ sParsedBit += c; }
	//Appends an inputted char to the string that stores the given object's initializer arguments.
	void appendToObjArgs( const char c ){ sParsedArgs += c; }
	//Resets the values typically used by the parser to their default values.
	void reset(){ sParsedObj.clear(); sParsedBit.clear(); sParsedArgs.clear(); bitNot = false; bitOperator = false; argsOP = false; }
	
	//Attempts to create a new object wrapper using an already defined ladder object, applies necessary wrapper flags, and returns the created object.
	shared_ptr<Ladder_OBJ_Wrapper> getObjectVARWrapper(shared_ptr<Ladder_OBJ> ptr)
	{
		shared_ptr<Ladder_VAR> pVar = ptr->getObjectVAR(sParsedBit);
		if ( !pVar ) //couldn't find it, so let's try to add it
			pVar = ptr->addObjectVAR( sParsedBit ); //try to create the VAR object
		
		if ( pVar ) //If successful (not null), make the wrapper and return it
			return make_shared<Ladder_OBJ_Wrapper>( pVar, getRungNum(), getNotOP() );

		return 0; //failed, return NULL
	}
	//This is responsible for generating the object wrapper by associating it with the (already initialized) object. May return a wrapper for the object itself, or an associated bit in the object.
	shared_ptr<Ladder_OBJ_Wrapper> createNewWrapper( shared_ptr<Ladder_OBJ> obj )
	{
		if ( obj ) //pointer must be valid
		{
			shared_ptr<Ladder_OBJ_Wrapper> newOBJWrapper = 0; //init
			if ( bitOperator ) //accessing a specific bit?
			{
				newOBJWrapper = getObjectVARWrapper(obj);
			}
			else
			{
				newOBJWrapper = make_shared<Ladder_OBJ_Wrapper>( obj, getRungNum(), getNotOP() );
			}

			if ( getRung()->addRungObject(newOBJWrapper) ) //Add to the new rung in order to perform updates on the object, post line scanning.
			{
				return newOBJWrapper; //return the new wrapper for later use.
			}
		}
		return 0; //return null
	}

	void setBitOP( bool bit ){ bitOperator = bit; }
	void setNotOP( bool bit ){ bitNot = bit; }
	void setArgsOP( uint8_t op ){ argsOP = op; }

    shared_ptr<Ladder_OBJ_Wrapper> handleObject();
	
	const String &getParsedObjectStr(){ return sParsedObj; } 
	const String &getParsedBitStr(){ return sParsedBit; } 
	const String &getParsedArgs(){ return sParsedArgs; } 

	vector<shared_ptr<NestContainer>> &getNests() { return nestData; }
	vector<shared_ptr<NestContainer>> getNestsByTier( uint8_t tier )
	{
		vector<shared_ptr<NestContainer>> tempVec;
		for (uint8_t x = 0; x < getNests().size(); x++)
		{
			if ( getNests()[x]->getPTier() == tier )
				tempVec.push_back(getNests()[x]);
		}

		return tempVec;
	}

	uint8_t getHighestNestTier()
	{
		uint8_t tier = 0; 
		for (uint8_t x = 0; x < getNests().size(); x++)
		{
			if ( getNests()[x]->getPTier() > tier )
				tier = getNests()[x]->getPTier();
		}
		return tier;
	}
	
	//This function returns a vector containing all objects that are the last to be referenced across all NestContainer objects
	vector<shared_ptr<Ladder_OBJ_Wrapper>> getLastNestObjects()
	{
		vector<shared_ptr<Ladder_OBJ_Wrapper>> allLastObjects;

		for (uint8_t x = 0; x < getNests().size(); x++)
		{
			if ( !getNests()[x]->getNumNextNests() ) //have we reach a nest that has no nests following it? Must be the last at the enf of the chain.
			{
				vector<shared_ptr<Ladder_OBJ_Wrapper>> lastObjects = getNests()[x]->getLastObjects();
				for ( uint8_t y = 0; y < lastObjects.size(); y++ )
				{
					allLastObjects.push_back(lastObjects[y]);
				}
			}
		}

		return allLastObjects;
	}

	//This function gathers a list of the first object's that are to be referenced by the PLC_Rung object when beginning a logic scan.
	vector<shared_ptr<Ladder_OBJ_Wrapper>> getFirstNestObjects()
	{
		vector<shared_ptr<Ladder_OBJ_Wrapper>> allFirstObjects;

		for (uint8_t x = 0; x <= getHighestNestTier(); x++)
		{
			vector<shared_ptr<NestContainer>> nests = getNestsByTier(x);
			for ( uint8_t y = 0; y < nests.size(); y++ )
			{
				vector<shared_ptr<Ladder_OBJ_Wrapper>> nestFirstObjects = nests[y]->getFirstObjects(); 
				for ( uint8_t z = 0; z < nestFirstObjects.size(); z++ ) //have we reach a nest that has no nests following it? Must be the last at the enf of the chain.
				{
					allFirstObjects.push_back(nestFirstObjects[z]);
				}
			}

			if ( allFirstObjects.size() ) //we have some objects after going through the first tier, so end here.
				break;
		}

		return allFirstObjects;
	}

	bool buildObjectStr( const String & );

	shared_ptr<Ladder_Rung> &getRung(){ return pRung; }
	bool getNotOP(){ return bitNot; }
	bool getBitOP(){ return bitOperator; }
	bool getArgsOP(){ return argsOP; }
	uint16_t getRungNum(){ return iRung; } 
	uint16_t &getLinePos() { return iLinePos; }
	uint16_t getLineLength() { return sParsedLine.length(); }
	const String &getParsedLineStr(){ return sParsedLine; }
	const char getCurrentLineChar(){ return getParsedLineStr()[getLinePos()]; }

private:
	bool bitOperator;
	bool bitNot; 
	bool argsOP; //Last operator used by the parser
	uint16_t iLinePos; //position at which the line is currently being parsed (in the string character array)
	uint16_t iLineLength; //total length of the parsed line (number of chars)
	uint16_t iRung; 
	String sParsedLine,
		   sParsedArgs, //object arguments (if applicable)
		   sParsedObj, //object name 
		   sParsedBit; //for bit operations on objects

	shared_ptr<Ladder_Rung> pRung; //Rung object that is being created by the parser
	vector<shared_ptr<NestContainer>> nestData;
};

struct LogicObject
{
	public:
	//This object is responsible for breaking up an inputted line based on the different types of operators that we are using for the PLC. 
	//Parenthetical tiers are also processed by recursively creating a new object from within the current object's constructor. 
	//In this sense, each newly created object represents the data that has been parsed for each specific parenthetical tier. 
	LogicObject(const String &line, PLC_Parser *parser, uint8_t pTier = 0, uint8_t orTier = 0)
	{
		//Serial.println("new LogicOP with str: " + line + " at tier: " + String(pTier) );
		pNestContainer = make_shared<NestContainer>(pTier, orTier); //create a new nest container for this LogicObject
		parser->getNests().emplace_back(pNestContainer); //add this new "block" parser object to the storage container in the parser object

		if ( !strContains(line, vector<char>{CHAR_OR, CHAR_EQUALS, CHAR_AND, CHAR_P_END, CHAR_P_START} )) //contains no logic operators at all? (could be a single object in a parallel operation)
		{
			if ( parser->buildObjectStr(line) )
			{
				if ( pNestContainer->addORObject(parser->handleObject()) ) //attempt to build the object using the parsed info in buildObjectStr() and push into NestContainer
				{
					//Serial.println("ADDED OR: " + line );
				}
			}
		}
		else
		{
			vector<String> outer = splitString(line, CHAR_EQUALS, true, CHAR_P_START, CHAR_P_END); //Split the inputted line up based on CHAR_EQUALS, with ( and ) as limiters
			for ( uint8_t x = 0; x < outer.size(); x++ )
			{
				if ( !strContains( outer[x], vector<char>{CHAR_OR, CHAR_AND, CHAR_P_END, CHAR_P_START})) //Does it contain any of these chars?
				{
					if ( parser->buildObjectStr(outer[x]) )
					{
						if ( pNestContainer->addEQObject(parser->handleObject()) ) //attempt to build the object using the parsed info in buildObjectStr() and push into NestContainer
						{
							//Serial.println("ADDED EQ: " + outer[x] );
						}
					}
					continue;
				}

				vector<String> inner = splitString(outer[x], CHAR_OR, true, CHAR_P_START, CHAR_P_END ); //Break up what's left by CHAR_OR
				for ( uint8_t y = 0; y < inner.size(); y++ )
				{
					if ( y > orTier )
						orTier = y;

					if ( !strContains( inner[y], vector<char>{CHAR_AND, CHAR_P_END, CHAR_P_START})) //Does the line contain any of these chars?
					{
						if ( parser->buildObjectStr(inner[y]) )
						{
							if ( pNestContainer->addORObject(parser->handleObject()) ) //attempt to build the object using the parsed info in buildObjectStr() and push into NestContainer
							{
								//Serial.println("ADDED OR: " + inner[y] );
							}
						}
						continue;
					}

					vector<String> current = splitString(inner[y], CHAR_AND, true, CHAR_P_START, CHAR_P_END ); //Split the string based on CHAR_AND
					for ( uint8_t z = 0; z < current.size(); z++ )
					{
						if ( !strContains( current[z], vector<char>{CHAR_P_END, CHAR_P_START}))
						{
							if ( parser->buildObjectStr(current[z]) )
							{
								if ( pNestContainer->addANDObject(parser->handleObject()) ) //attempt to build the object using the parsed info in buildObjectStr() and push into NestContainer
								{
									//Serial.println("ADDED AND: " + current[z] );
								}
							}
							continue;
						}

						multimap<int8_t, String> tierMap = textWithin(current[z], CHAR_P_START, CHAR_P_END, 1); //break everything up into tiers and subtiers based on parenthesis in statement
						typedef multimap<int8_t, String> :: iterator itr;
						for (int8_t ptier = 0; ptier < tierMap.end()->first; ptier++ ) //find the highest tier level
						{
							if ( !tierMap.count(ptier) ) //make sure there's some data stored for the given tier
								continue;

							pair<itr, itr> nestedStatements = tierMap.equal_range(ptier); 
							for ( itr it = nestedStatements.first; it != nestedStatements.second; it++ )
							{
								shared_ptr<LogicObject> newLogicTier = make_shared<LogicObject>(it->second, parser, pTier + 1, orTier );
								pNestContainer->addNextNestContainer( newLogicTier->getNestContainer() );
							} //end iterations through nested tier
						} //end equal range for tier number
					}
				}
			}
		}

		pNestContainer->connectObjects(); //tether the objects together for this logic block if necessary (particularly for series - AND - objects). 
	}
	//Deconstructor
	~LogicObject(){}

	shared_ptr<NestContainer> getNestContainer(){ return pNestContainer; }

	private: 
	shared_ptr<NestContainer> pNestContainer;
};

/*ladderOBJdata is mostly for tying a name (String) to an object for the brief time it matters.
* This is typically only supposed to be used during the parsing of an inputted ladder logic script, and objects of this type are deleted once the script has been parsed and object associations are established.
*/
struct ladderOBJdata 
{
	ladderOBJdata( const String &name, shared_ptr<Ladder_OBJ> createdObj )
	{
		objName = name;
		LadderOBJ = createdObj;
	}
	~ladderOBJdata()
	{ 
	}
	
	//Returns a pointer to the ladder object stored in this object (if applicble).
	shared_ptr<Ladder_OBJ> getObject(){ return LadderOBJ; } 
	//Returns a reference to the name (string) of the object as it has been parsed. This is only used while the parser is generating the logic functionality.
	const String &getName(){ return objName; }
	
	private:
	shared_ptr<Ladder_OBJ> LadderOBJ;
	String objName;
};

#endif