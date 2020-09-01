
/*
 * PLC_Parser.cpp
 *
 * Created: 8/11/2020 5:35:08 PM
 *  Author: Andrew Ward
 * This file serves as a container for all functionality related to parsing a logic script for ladder logic object generation and process order.
 */ 

#include "PLC_Main.h"

/*
So basically we need to establish a way to define the objects that are present in the script. This includes all inputs/outputs/timers/counters etc. This also initializes these objects with the default 
arguments that dictate the logic calculations they use. 
Defining Syntax Ex: NAME[TYPE,ARGS]

Logic Syntax Ex: IN1 + IN2 = OUT

If an object is referenced without being initialized, fall out with an error
*/
//TODO: For NOT (/) objects and .BIT arguments, a dummy object needs to be created. .BIT access individual variables in the object, and / gives an inverted state value.
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
    for (uint8_t char_index = 0; char_index < str.length(); char_index++)
    { 
        switch (str[char_index])
        {
            case CHAR_VAR_OPERATOR: // For Bit operators for existing objects. Example: Timer1.DN, where DN is the DONE bit for the timer.
            {
                if ( !getArgsOP() ) //Must not currently be parsing args
                    setBitOP(true); //chars parsed are now an operator to an object variable (or bit tag)
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
            default: //standard case, which simply entails appending the current character in the line to our temporary string used in the parser.
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
            break;
        } 
    } 
    return true; //default condition (success)
}

shared_ptr<Ladder_OBJ_Wrapper> PLC_Parser::handleObject()
{    
    shared_ptr<Ladder_OBJ_Wrapper> obj = createNewWrapper(PLCObj.findLadderObjByID(getParsedObjectStr()));
    if ( !obj ) //Invalid object? Probably because it doesn't exist
    {
        shared_ptr<Ladder_OBJ> newObj = PLCObj.createNewObject(getParsedObjectStr(), parseObjectArgs()); //so try to create it
		if ( newObj ) //was the creation successful?
        {
            obj = createNewWrapper(newObj); //create a wrapper from the newly generated object
        }
		else //guess not
        {
            sendError(ERR_DATA::ERR_INVALID_OBJ, getParsedObjectStr());
        }
    }

    reset(); //reset temp storage variables
    return obj;
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