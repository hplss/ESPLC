/**
 * PLC_Parser.cpp
 *
 * Created: 8/11/2020 5:35:08 PM
 *  Author: Andrew Ward
 * This file serves as a container for all functionality related to parsing a logic script for ladder logic object generation and process order.
 */ 

#include "PLC_Main.h"

/**
 * NestContainer object definitions below here:
*/

bool NestContainer::connectObjects()
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


/**
 * PLC_Parser Object Definitions located below here:
*/

bool PLC_Parser::parseLine()
{
    //This parser code can be cleaned up, but we'll worry about getting it working first.
    vector<shared_ptr<Ladder_OBJ_Wrapper>> firstEQObjects; //container for all "global" assignment operations for the line being parsed.
    vector<String> firstOrObjects; //continer for all strings that are split at the OR operator (parallel operations)
    //Before handling each tier, break up any "global" object operations (per line). An example of this is the Objects being assigned (=) at the end of the line (not in parenthesis).
    vector<String> outer = splitString(getParsedLineStr(), CHAR_EQUALS, true, CHAR_P_START, CHAR_P_END); //Split on '=' char first
    for ( uint8_t x = 0; x < outer.size(); x++ )
    {
        if ( !strContains( outer[x], vector<char>{CHAR_OR, CHAR_AND, CHAR_P_END, CHAR_P_START})) //Does the split string contain other operators?
        {
            if ( buildObjectStr(outer[x]) ) //If not, 
            {
                shared_ptr<Ladder_OBJ_Wrapper> newObj = handleObject(); //perform any logic specific operations and initialize it.
                if ( newObj ) //attempt to build the object using the parsed info in buildObjectStr() and push into NestContainer
                {
                    firstEQObjects.push_back(newObj);
                }
            }
            continue; //It doesn't, so move to the next entry
        }

        firstOrObjects = splitString(outer[x], CHAR_OR, true, CHAR_P_START, CHAR_P_END ); //otherwise break the remainder of the initial string into pieces based on CHAR_OR
    }

    if ( firstOrObjects.size() )
    {
        for ( uint8_t p = 0; p < firstOrObjects.size(); p++) //start processing each individual chunk that remained from the previous operation 
        {
            make_shared<LogicObject>(firstOrObjects[p], this); //This wonderful object handles all of the parsing, and stores the generated data into the PLC_Parser object (this)

            for ( uint8_t x = 0; x < getHighestNestTier(); x++ ) //Start at the lowest tier that was parsed (this should be 0 in all cases)
            {
                vector<shared_ptr<NestContainer>> tierNest = getNestsByTier(x); //Get the NestContainers for the given tier
                for ( uint8_t y = 0; y < tierNest.size(); y++ ) //Iterate through all nests for the current tier..
                {
                    vector<shared_ptr<NestContainer>> nextNests = tierNest[y]->getNextNests(); //For the given nest, find the nest (tethered) NestContainer objects 
                    vector<shared_ptr<Ladder_OBJ_Wrapper>> currentLastObjects = tierNest[y]->getLastObjects();
                    vector<shared_ptr<Ladder_OBJ_Wrapper>> currentFirstObjects = tierNest[y]->getFirstObjects();

                    for ( uint8_t z = 0; z < nextNests.size(); z++ ) //Iterate through the next nests for the given tier object...
                    {   
                        vector<shared_ptr<Ladder_OBJ_Wrapper>> nextNestObjects = nextNests[z]->getFirstObjects();

                        if ( !nextNestObjects.size() && currentLastObjects.size() ) //no objects stored in the next tier up, so forward the current (last) objects to the next tier.
                        {
                            nextNests[z]->setLastObjects( currentLastObjects );
                        }
                        else //we do have some objects available.. so tie them together (logically speaking)
                        {
                            for(uint8_t q = 0; q < currentLastObjects.size(); q++ )
                            {
                                for ( uint8_t r = 0; r < nextNestObjects.size(); r++ )
                                {
                                    currentLastObjects[q]->addNextObject(nextNestObjects[r]);
                                }
                            }
                        }
                    }
                } //End of iteration through all nests of a given tier
            } //end of iteration through all parallel operation blocks
        }
    }
    else if ( firstEQObjects.size() > 1 ) //no objects in the OR objects list.. means that all of the objects were stored into the EQobjects vector. Time for a bit of a hack... 
    {
        shared_ptr<Ladder_OBJ_Wrapper> firstObj = firstEQObjects.front();
        getRung()->addInitialRungObject(firstObj);
        for (uint8_t x = 1; x < firstEQObjects.size(); x++ )
            firstObj->addNextObject(firstEQObjects[x]);
    }

    //Finally, we append the "global" assignments to the end of each "last" object of each nest.
    for ( uint8_t q = 0; q < firstEQObjects.size(); q++ )
    {
        vector<shared_ptr<Ladder_OBJ_Wrapper>> allLastObjects = getLastNestObjects();
        for ( uint8_t x = 0; x < allLastObjects.size(); x++ )
        {
            allLastObjects[x]->addNextObject(firstEQObjects[q]);
        }
    }

    getRung()->addInitialRungObject(getFirstNestObjects()); //Find and add the appropriate objects to the initial objects list for the PLC scan.


    //Finally, add the rung to the list of rungs in PLC_Main for processing.
	if ( PLCObj.addLadderRung(getRung()) )
	{
		#ifdef DEBUG
		Serial.print(PSTR("Rung Created. Objects: "));
		Serial.println(getRung()->getNumRungObjects());
		#endif
	}
    //

	return true; //success
}

bool PLC_Parser::buildObjectStr(const String &str)
{
    bool accessor = strContains(str, CHAR_ACCESSOR_OPERATOR);

    for (uint8_t char_index = 0; char_index < str.length(); char_index++)
    { 
        switch (str[char_index])
        {
            case CHAR_VAR_OPERATOR: // For Bit operators for existing objects. Example: Timer1.DN, where DN is the DONE bit for the timer.
            {
                if ( !accessor ) //no var operators allowed when parsing accessor name
                {
                    if ( !getArgsOP() ) //Must not currently be parsing args
                        setBitOP(true); //chars parsed are now an operator to an object variable (or bit tag)
                    else //Actually, this is a part of an argument being passed in
                        appendToObjArgs( str[char_index] );
                }
            }
            break;
            case CHAR_NOT_OPERATOR: //Indicates NOT logic
            {
                if ( !getArgsOP() ) //Must not currently be parsing args
                    setNotOP(!getNotOP()); //invert from previous value / = not -> // == NOT NOT 
            }
            break;
            case CHAR_BRACKET_START:
            case CHAR_BRACKET_END:
            {
                setArgsOP( !getArgsOP() );
            }
            break;
            case CHAR_ACCESSOR_OPERATOR:
            {
                accessor = false; //looks like we have built the accessor name
            }
            break;
            default: //standard case, which simply entails appending the current character in the line to our temporary string used in the parser.
            {
                if ( accessor )
                {
                    appendToAccessorName( str[char_index] );
                }
                else
                {
                    if ( !getArgsOP() ) //Must not currently be parsing args
                    {
                        if ( !getBitOP() )
                            appendToObjName( str[char_index] ); //looks like we're building the name of an object so far
                        else //nope, it's a bit, so build the bit ID tag string instead.
                            appendToBitName( str[char_index] );
                    }
                    else
                        appendToObjArgs( str[char_index] );

                //here there should probably be some detection to see if we're still parsing args at the end of the string? ERROR -> to console
                }
            }
            break;
        } 
    }

    if ( !getParsedObjectStr().length() ) //must have some name for the object at least
        return false;

    return true; //default condition (success)
}

shared_ptr<Ladder_OBJ_Wrapper> PLC_Parser::getObjectVARWrapper(shared_ptr<Ladder_OBJ_Logical> ptr)
{
    shared_ptr<Ladder_VAR> pVar = ptr->getObjectVAR(sParsedBit);
    if ( !pVar ) //couldn't find it, so let's try to add it
        pVar = ptr->addObjectVAR( sParsedBit ); //try to create the VAR object
    
    if ( pVar ) //If successful (not null), make the wrapper and return it
        return make_shared<Ladder_OBJ_Wrapper>( pVar, getRungNum(), getNotOP() );

    return 0; //failed, return NULL
}

shared_ptr<Ladder_OBJ_Wrapper> PLC_Parser::createNewWrapper( shared_ptr<Ladder_OBJ_Logical> obj )
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

shared_ptr<Ladder_OBJ_Wrapper> PLC_Parser::handleObject()
{   
    shared_ptr<Ladder_OBJ_Wrapper> obj = 0;
    if ( getParsedAccessorStr().length() ) //Do we have some accessor that we are referencing?
    {
        shared_ptr<Ladder_OBJ_Accessor> accessor = PLCObj.findAccessorByID(getParsedAccessorStr()); 
        if ( !accessor ) //didn't find the accessor from the list. 
        {
            sendError(ERR_DATA::ERR_INVALID_ACCESSOR, getParsedAccessorStr());
        }
        else
        {
            Core.sendMessage("Found the accessor");
        }
    }   
    else
    {
        obj = createNewWrapper(PLCObj.findLadderObjByID(getParsedObjectStr()));

        if ( !obj ) //Invalid object? Probably because it doesn't exist
        {
            if ( PLCObj.createNewLadderObject(getParsedObjectStr(), parseObjectArgs()) )  //so try to create it
            {
                obj = createNewWrapper(PLCObj.findLadderObjByID(getParsedObjectStr())); //create a wrapper from the newly generated object
            }
            else //guess not
            {
                sendError(ERR_DATA::ERR_INVALID_OBJ, getParsedObjectStr());
            }
        }
    }
    reset(); //reset temp storage variables
    return obj; //default return path
}

vector<String> PLC_Parser::parseObjectArgs()
{
    const String &args = getParsedArgs(); //retrieve the arguments that were already stored earlier in the parsing operation (for this object)
    String parsed;
	vector<String> ObjArgs;
    
    for ( uint8_t x = 0; x < args.length(); x++ ) //At this point, we're parsing the arguments within the () of an object. IE: OUTPUT( PIN )
    {
        switch(args[x])
        {
            case CHAR_COMMA:
            {
                ObjArgs.push_back(parsed); 
                parsed.clear();
                continue;
            }
            break;
            default:
            {
                parsed += args[x];
            }
        }
    }

    if ( parsed.length() )
        ObjArgs.push_back(parsed);

    #ifdef DEBUG
    for ( uint8_t x = 0; x < ObjArgs.size(); x++ )
        Serial.println("Arg: " + ObjArgs[x]);
    #endif

    return ObjArgs;
}

void PLC_Parser::sendError(uint8_t err, const String &str)
{ 
    PLCObj.sendError(err,str);
}

vector<shared_ptr<Ladder_OBJ_Wrapper>> PLC_Parser::getFirstNestObjects()
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

vector<shared_ptr<Ladder_OBJ_Wrapper>> PLC_Parser::getLastNestObjects()
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

vector<shared_ptr<NestContainer>> PLC_Parser::getNestsByTier( uint8_t tier )
{
    vector<shared_ptr<NestContainer>> tempVec;
    for (uint8_t x = 0; x < getNests().size(); x++)
    {
        if ( getNests()[x]->getPTier() == tier )
            tempVec.push_back(getNests()[x]);
    }

    return tempVec;
}

uint8_t PLC_Parser::getHighestNestTier()
{
    uint8_t tier = 0; 
    for (uint8_t x = 0; x < getNests().size(); x++)
    {
        if ( getNests()[x]->getPTier() > tier )
            tier = getNests()[x]->getPTier();
    }
    return tier;
}

/**
 *  LogicObject function definitions are located below here:
 */
LogicObject::LogicObject(const String &line, PLC_Parser *parser, uint8_t pTier, uint8_t orTier)
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