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
    vector<OBJ_WRAPPER_PTR> firstEQObjects; //container for all "global" assignment operations for the line being parsed.
    vector<String> firstOrObjects; //continer for all strings that are split at the OR operator (parallel operations)
    //Before handling each tier, break up any "global" object operations (per line). An example of this is the Objects being assigned (=) at the end of the line (not in parenthesis).
    vector<String> outer = splitString(getParsedLineStr(), CHAR_EQUALS, true, CHAR_P_START, CHAR_P_END); //Split on '=' char first
    for ( uint8_t x = 0; x < outer.size(); x++ )
    {
        if ( !strContains( outer[x], vector<char>{CHAR_OR, CHAR_AND, CHAR_P_END, CHAR_P_START})) //Does the split string contain other operators?
        {
            if ( buildObjectStr(outer[x]) ) //If not, 
            {
                OBJ_WRAPPER_PTR newObj = handleObject(); //perform any logic specific operations and initialize it.
                if ( newObj ) //attempt to build the object using the parsed info in buildObjectStr() and push into NestContainer
                {
                    firstEQObjects.push_back(newObj);
                }
                //else
                   // return false;
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
                    vector<NEST_PTR> nextNests = tierNest[y]->getNextNests(); //For the given nest, find the nest (tethered) NestContainer objects 
                    vector<OBJ_WRAPPER_PTR> currentLastObjects = tierNest[y]->getLastObjects();

                    for ( uint8_t z = 0; z < nextNests.size(); z++ ) //Iterate through the next nests for the given tier object...
                    {   
                        vector<OBJ_WRAPPER_PTR> nextNestObjects = nextNests[z]->getFirstObjects();

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
        OBJ_WRAPPER_PTR firstObj = firstEQObjects.front();
        getRung()->addInitialRungObject(firstObj);

        for (uint8_t x = 1; x < firstEQObjects.size(); x++ )
            firstObj->addNextObject(firstEQObjects[x]);
    }

    //Finally, we append the "global" assignments to the end of each "last" object of each nest.
    for ( uint8_t q = 0; q < firstEQObjects.size(); q++ )
    {
        vector<OBJ_WRAPPER_PTR> allLastObjects = getLastNestObjects();
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
    if ( strBeginsWith( str, CHAR_BRACKET_START ) ) //jumping to special single-use objects such as ONS
    {
        sParsedArgs = removeFromStr(str, { CHAR_BRACKET_START, CHAR_BRACKET_END });
        if ( !sParsedArgs.length() )
            return false;
    }
    else //regular objects
    {
        vector<String> definitions = splitString( str, { CHAR_BRACKET_START, CHAR_BRACKET_END });
        
        if ( definitions.size() > 2 )
        {
            Core.sendMessage(PSTR("Error: Invalid argument syntax."));
            return false;
        } 
        if (definitions.size() > 1) //Have args and object name, with name (including bit/accessor operator) coming first
        {
            sParsedArgs = definitions.back(); //Args are the second element. Save off here
        }
        if ( definitions.size() > 0 ) //have only the object name,accessor, etc.
        {
            vector<String> accessorDetails = splitString( definitions.front(), CHAR_ACCESSOR_OPERATOR );
            vector<String> bitDetails;
            if ( accessorDetails.size() > 2) //multiple accessor operators found
            {
                Core.sendMessage(PSTR("Error: Multiple Accessor operators detected."));
                return false;
            }

            if ( accessorDetails.size() == 2 ) //Accessor found
            {
                sParsedAccessor = accessorDetails.front();      //Store off the accessor name here.       
            }

            bitDetails = splitString(accessorDetails.back(), CHAR_VAR_OPERATOR );

            if ( bitDetails.size() > 2) //variable operator
            {  
                Core.sendMessage(PSTR("Error: Multiple Variable operators detected."));
                return false;
            }

            if ( bitDetails.size() > 1)
            {
                sParsedBit = bitDetails.back(); 
            }

            if ( bitDetails.size() > 0) //lone object
            {
                if (strContains(bitDetails.front(), CHAR_NOT_OPERATOR))
                {
                    setNotOP(!getNotOP());
                    sParsedObj = removeFromStr( bitDetails.front(), {CHAR_NOT_OPERATOR} );
                }
                else
                {
                    sParsedObj = bitDetails.front();
                }
            }
            else
            {
                Core.sendMessage(PSTR("Error: No object found."));
                return false;
            }
        }
        else 
            return false;
    }
    

    return true; //default condition (success)
}

shared_ptr<Ladder_OBJ_Wrapper> PLC_Parser::getObjectVARWrapper(shared_ptr<Ladder_OBJ_Logical> ptr)
{
    VAR_PTR pVar = ptr->getObjectVAR(sParsedBit); //This will attempt to find the existing variable object (or sometimes create it, depending on the object type)
    if ( pVar ) //If successful (not null), make the wrapper and return it
        return make_shared<Ladder_OBJ_Wrapper>( pVar, getRungNum(), getNotOP() );

    return 0; //failed, return NULL
}

shared_ptr<Ladder_OBJ_Wrapper> PLC_Parser::createNewWrapper( shared_ptr<Ladder_OBJ_Logical> obj )
{
    if ( obj ) //pointer must be valid
    {
        OBJ_WRAPPER_PTR newOBJWrapper = 0; //init
        if ( getParsedBitStr().length() && !getParsedAccessorStr().length() ) //accessing a specific bit? -- bit of a hack for now. accessors utilize entire names including bit operators for assigning new objects.
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

OBJ_WRAPPER_PTR PLC_Parser::handleObject()
{   
    OBJ_WRAPPER_PTR obj = 0;
    if ( getParsedAccessorStr().length() ) //Do we have some accessor that we are referencing?
    {
        OBJ_ACC_PTR accessor = PLCObj.findAccessorByID(getParsedAccessorStr()); 
        if ( !accessor ) //didn't find the accessor from the list. 
        {
            sendError(ERR_DATA::ERR_INVALID_ACCESSOR, getParsedAccessorStr());
        }
        else
        {
            String varObject = getParsedObjectStr() + CHAR_VAR_OPERATOR + getParsedBitStr(); //only variable type objects for now

            obj = createNewWrapper(accessor->findAccessorVarByID(varObject));
            if ( !obj )
                Core.sendMessage(PSTR("Failed to find or init the accessor object: ") + varObject );
        }
    }   
    else
    {
        obj = createNewWrapper(PLCObj.findLadderObjByID(getParsedObjectStr()));

        if ( !obj ) //Invalid object? Probably because it doesn't exist
        {
            shared_ptr<Ladder_OBJ> newObj = PLCObj.createNewLadderObject(getParsedObjectStr(), parseObjectArgs());
            if ( newObj )  //so try to create it
            {
                if(newObj->iType == OBJ_TYPE::TYPE_ONS)
                {
                    obj = createNewWrapper(static_pointer_cast<Ladder_OBJ_Logical>(newObj));
                }
                else
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
	vector<String> ObjArgs = splitString(getParsedArgs(), CHAR_COMMA);
    
    #ifdef DEBUG
    for ( uint8_t x = 0; x < ObjArgs.size(); x++ )
        Serial.println("Arg: " + ObjArgs[x]);
    #endif

    return ObjArgs;
}

void PLC_Parser::sendError(ERR_DATA err, const String &str)
{ 
    PLCObj.sendError(err,str);
}

vector<OBJ_WRAPPER_PTR> PLC_Parser::getFirstNestObjects()
{
    vector<OBJ_WRAPPER_PTR> allFirstObjects;

    for (uint8_t x = 0; x <= getHighestNestTier(); x++)
    {
        vector<NEST_PTR> nests = getNestsByTier(x);
        for ( uint8_t y = 0; y < nests.size(); y++ )
        {
            vector<OBJ_WRAPPER_PTR> nestFirstObjects = nests[y]->getFirstObjects(); 
            for ( uint8_t z = 0; z < nestFirstObjects.size(); z++ ) //have we reached a nest that has no nests following it? Must be the last at the end of the chain.
            {
                allFirstObjects.push_back(nestFirstObjects[z]);
            }
        }

        if ( allFirstObjects.size() ) //we have some objects after going through the first tier, so end here.
            break;
    }

    return allFirstObjects;
}

vector<OBJ_WRAPPER_PTR> PLC_Parser::getLastNestObjects()
{
    vector<OBJ_WRAPPER_PTR> allLastObjects;

    for (uint8_t x = 0; x < getNests().size(); x++)
    {
        if ( !getNests()[x]->getNumNextNests() ) //have we reach a nest that has no nests following it? Must be the last at the enf of the chain.
        {
            vector<OBJ_WRAPPER_PTR> lastObjects = getNests()[x]->getLastObjects();
            for ( uint8_t y = 0; y < lastObjects.size(); y++ )
            {
                allLastObjects.push_back(lastObjects[y]);
            }
        }
    }

    return allLastObjects;
}

vector<NEST_PTR> PLC_Parser::getNestsByTier( uint8_t tier )
{
    vector<NEST_PTR> tempVec;
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
                    typedef multimap<int8_t, String>::iterator itr;
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