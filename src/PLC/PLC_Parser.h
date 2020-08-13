#ifndef PLC_PARSER_H_
#define PLC_PARSER_H_

using namespace std;

/* The logicOP object serves as a container for individual logic statements as they are read in the parser. 
	Technically speaking, each LogicOP represents a series operation, and a vector containing LogicOPs represents parallel operations.
	LogicOP's are capable of storing multime "tiers" of objects, with each tier representing the level of a nest, as it pertains to the final logic calculation.
	A perfect example of this is SW1*(SW2+SW3)*(SW4+SW5)+SW6=OUT1 -- Here, SW1 has both SW2 and SW3 as the next objects given for comparison testing in a calculation, and SW2 and 3 are given 4 and 5 respectively.
	This is essentially a series operation, with nested parallel operations involved.
	It should be noted that in this example, SW1, SW2/3, and SW4/5 represent individual tiers that reference SW1, whereas SW6 ids its own logicOP.
*/
struct logicOP
{
	public:
	logicOP( uint16_t rung, shared_ptr<Ladder_OBJ_Wrapper> obj )
	{
		iRung = rung;
		iTier = 0; //default tier is at level 0
		objects.emplace(iTier, obj);
	}
	~logicOP()
	{
	}

	//Appends an inputted object to the objects map, with the current tier used as the index for the logical operation.
	bool addParallelObject(shared_ptr<Ladder_OBJ_Wrapper> obj, bool nested = false)
	{ 
        if ( (nested && !applyNextObjectsToLast(obj)) || !applyNextObjects(obj))
            return false;

        objects.emplace(getTier(), obj); //up shifting a tier for certain ops? object on tier 2 and 3 append to tier 1?
		return true;
	}
	//Creates a new tier, which represents a new nest in a single (series) logic operation
	bool addSeriesObject(shared_ptr<Ladder_OBJ_Wrapper> obj, bool nested = false)
	{
		if ( !getNumCurrentObjects() ) //must have at least one valid object in the map for the current tier.
			return false;
		
		iTier++; //increase our tier index
		return addParallelObject(obj, nested); //Think of the Series as the X axis, And the parallel as the Y axis
	}

	//Returns the current number of objects in the last tier of the logicOP
	uint8_t getNumCurrentObjects(){ return getCurrentObjects().size(); }
	uint8_t getNumTierObjects( uint8_t tier ){ return getTierObjects(tier).size(); }
	//This returns a vector of the objects added to the last tier of the logicOP object
	shared_ptr<Ladder_OBJ_Wrapper> getLastObject( uint8_t tier )
    {
        if ( getNumTierObjects(tier) )
            return getTierObjects(tier).back(); 
        
        return 0;//null
    }
	vector<shared_ptr<Ladder_OBJ_Wrapper>> getCurrentObjects() { return getTierObjects(getTier()); }
	vector<shared_ptr<Ladder_OBJ_Wrapper>> getTierObjects( uint8_t tier ) 
	{  
		vector<shared_ptr<Ladder_OBJ_Wrapper>> pObjects; //make the container
		pair<itr, itr> rungObjects = objects.equal_range(tier); //Find all objects for the current rung op
		for ( itr it = rungObjects.first; it != rungObjects.second; it++ )
			pObjects.emplace_back(it->second);

		return pObjects; 
	}
	uint8_t getTier(){ return iTier; }
	void setTier( uint8_t tier )
	{ 
		iTier = tier; 
	}

	private:
	//This tells the previous tier of objects that the current tier's objects are the next in line to be processed in a given logic operation.
	bool applyNextObjects( shared_ptr<Ladder_OBJ_Wrapper> obj )
	{
		if ( getTier() ) //must have multiple tiers (greater than 0)
		{
            vector<shared_ptr<Ladder_OBJ_Wrapper>> pObjects = getTierObjects(getTier() - 1);
            for (uint8_t x = 0; x < pObjects.size(); x++)
            {
                if ( obj->getObject() == pObjects[x]->getObject() || !pObjects[x]->addNextObject(iRung, obj))
                    return false;
            }
        }
        return true;
	}
    bool applyNextObjectsToLast( shared_ptr<Ladder_OBJ_Wrapper> obj )
    {
        shared_ptr<Ladder_OBJ_Wrapper> prev = getLastObject(getTier() - 1);
        if (prev && prev->getObject() != obj->getObject())
            return prev->addNextObject(iRung, obj);

        return false;
    }

	multimap<uint8_t, shared_ptr<Ladder_OBJ_Wrapper>> objects; //<tier, object pointer>
	typedef multimap<uint8_t, shared_ptr<Ladder_OBJ_Wrapper>> :: iterator itr;
	uint8_t iTier;
	uint16_t iRung;
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
		iLastOP = CHAR_AND;
		sParsedLine = parsed;
		iRung = rung;
		iNestDepth = 0;
	}

    void sendError(uint8_t, const String & = "");

    //Parses the individual lines for logic operations and declarations (called from parseScript())
	bool parseLine();
	//Displays an error of a given type, with the option of displaying additional information.
	bool parser_ANDOP();
	bool parser_EQOP();
	bool parser_OROP();
	bool parser_Default();

    vector<String> parseObjectArgs();

	void appendToObjName( const char c ) { sParsedObj += c; }
	void appendToBitName( const char c ){ sParsedBit += c; }
	//Resets the values typically used by the parser to their default values.
	void reset(){ sParsedObj.clear(); sParsedBit.clear(); bitNot = false; bitOperator = false; }
	
	//Attempts to create a new object wrapper using an already defined ladder object, applies necessary wrapper flags, and returns the created object.
	shared_ptr<Ladder_OBJ_Wrapper> getObjectVARWrapper(shared_ptr<Ladder_OBJ> ptr)
	{
		shared_ptr<Ladder_VAR> pVar = ptr->getObjectVAR(sParsedBit);
		if ( !pVar ) //couldn't find it, so let's try to add it
			pVar = ptr->addObjectVAR( sParsedBit ); //try to create the VAR object
		
		if ( pVar ) //If successful (not null), make the wrapper and return it
			return make_shared<Ladder_OBJ_Wrapper>( pVar, getNotOP() );

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
				newOBJWrapper = make_shared<Ladder_OBJ_Wrapper>( obj, getNotOP() );
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
	void setLastOP( uint8_t op ){ iLastOP = op; }
    bool setNumNests( uint8_t num )
    {
        if ( num < 0 )
            return false;
        
        iNestDepth = num;
        return true;
    }

	bool addNest()
	{ 
		//Basically,  in a nest, any objects that are parsed are added into a parallel operation with their respective logicop tier. 
		//Maximum number of nests check? Meh..
		#ifdef DEBUG
		Serial.println(PSTR("Added a Nest"));
		#endif

        
		iNestDepth++;
        iRequestedNestDepth = iNestDepth;
		return true;
	}
	//End a currently initialized nest. This entails linking all of the "previous objects" initialized in the nest as next objects for the previous tier of object(s).
	bool endNest()
	{
		#ifdef DEBUG
		Serial.println(PSTR("Ended a Nest"));
		#endif

		if (iNestDepth > 0)
		{
			//iNestDepth--;
            iRequestedNestDepth--;
		}
		else
		{
			return false; //error, tried to remove too many nests
		}
		
		return true;
	}

    bool handlePreviousOP(bool = false);
    bool handleEQOP( shared_ptr<Ladder_OBJ_Wrapper> );
	
	uint8_t getLastOP(){ return iLastOP; }
	const String &getParsedObjectStr(){ return sParsedObj; } 
	const String &getParsedBitStr(){ return sParsedBit; } 

	bool addLogicOP(shared_ptr<Ladder_OBJ_Wrapper> ptr)
	{ 
		if ( getRung()->addInitialRungObject(ptr) ) //only create it if it's possible to add the pointer to the initial objects list.
		{
			getLogicOPs().emplace_back(make_shared<logicOP>(iRung, ptr));
            return true;
		}

		return false;
	}
	uint8_t getNumLogicOPs(){ return getLogicOPs().size(); }
	//Returns a reference to the vector containing all created logicOPs
	vector<shared_ptr<logicOP>> &getLogicOPs(){ return pLogicOPs; }
	//Returns a reference to the current (last created) previous objects vector. Index of -1 returns the most recently initialized tier of previous objects.
	shared_ptr<logicOP> getLogicOP( int8_t idx = -1 )
	{ 
		if (idx >= 0 && idx <= (pLogicOPs.size() - 1) )
			return pLogicOPs[idx];

		return pLogicOPs.back();
	}

	shared_ptr<Ladder_Rung> &getRung(){ return pRung; }
	uint8_t getNumNests(){ return iNestDepth; }
	bool getNotOP(){ return bitNot; }
	bool getBitOP(){ return bitOperator; }
	uint16_t &getLinePos() { return iLinePos; }
	uint16_t getLineLength() { return sParsedLine.length(); }
	const String &getParsedLineStr(){ return sParsedLine; }
	const char getCurrentLineChar(){ return getParsedLineStr()[getLinePos()]; }

private:
	bool bitOperator;
	bool bitNot; 
	uint8_t iLastOP; //Last operator used by the parser
	uint16_t iLinePos; //position at which the line is currently being parsed (in the string character array)
	uint16_t iLineLength; //total length of the parsed line (number of chars)
	uint16_t iRung; 
	uint8_t iNestDepth, iRequestedNestDepth;
	String sParsedLine,
		   sParsedObj, //object name 
		   sParsedBit; //for bit operations on objects

	vector<shared_ptr<logicOP>> pLogicOPs;

	shared_ptr<Ladder_Rung> pRung; //Rung object that is being created by the parser
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