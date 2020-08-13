
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
Defining Syntax Ex: NAME=TYPE(ARGS)

Logic Syntax Ex: IN1 + IN2 = OUT

If an object is referenced without being initialized, fall out with an error
*/
//TODO: For NOT (/) objects and .BIT arguments, a dummy object needs to be created. .BIT access individual variables in the object, and / gives an inverted state value.

bool PLC_Parser::parseLine()
{
	for (getLinePos() = 0; getLinePos() < getLineLength(); getLinePos()++)
	{ 
		switch (getCurrentLineChar())
		{
			//At this point, we need to make 
			case CHAR_P_START: //for nested objects
			{
				if (!addNest())
				{
					return false; //Some error here
				}
			}
			break;
			case CHAR_P_END: //also for nested objects (not implemented yet)
			{
				if ( !endNest() )
				{
					sendError(0,"Failed to end a nest.");
					return false; 
				}
			}
			break;
			case CHAR_AND: //We've got an AND operator
			{
				#ifdef DEBUG
				Serial.print(PSTR("AND OP at: "));
				Serial.println(getLinePos());
				#endif
				if ( !parser_ANDOP() ) //perform AND OP computations
					return false;
			}
			break;
			case CHAR_OR: //we've got an OR operator
			{
				#ifdef DEBUG 
				Serial.print("OR OP at: ");
				Serial.println(getLinePos());
				#endif 
				if (!parser_OROP()) //perform OR OP logic computations
					return false;
			}
			break;
			case CHAR_EQUALS: //we've got an = operator (for outputs or object creation)
			{
				#ifdef DEBUG
				Serial.print(PSTR("EQ OP at: "));
				Serial.println(getLinePos());
				#endif
				if (!parser_EQOP())
					return false;
			}
			break;
			case CHAR_VAR_OPERATOR: // For Bit operators for existing objects. Example: Timer1.DN, where DN is the DONE bit for the timer.
				setBitOP(true); //chars parsed are now an operator to an object variable (or bit tag)
			break;
			case CHAR_NOT_OPERATOR: //Indicates NOT logic
			{
				setNotOP(!getNotOP()); //invert from previous
			}
			break;
			default: //standard case, which simply entails appending the current character in the line to our temporary string used in the parser.
			{
				if (!parser_Default())
                {
					return false;
                }
			}
			break;
		}
	}

	if ( PLCObj.addLadderRung(getRung()) )
	{
		#ifdef DEBUG
		Serial.print(PSTR("Rung Created. Objects: "));
		Serial.println(getRung()->getNumRungObjects());
		#endif
	}
	return true; //success
}

bool PLC_Parser::parser_Default( )
{
	if ( !getBitOP() )
		appendToObjName( getCurrentLineChar()); //looks like we're building the name of an object so far
	else //nope, it's a bit, so build the bit ID tag string instead.
		appendToBitName( getCurrentLineChar());
		
	if (getLinePos() == getLineLength()-1) //We've reached the end (not including the terminating char, etc). Make sure to handle the logic for the last parsed arg.
	{
        if ( getLastOP() == CHAR_EQUALS ) //create a new logic op
        {
            if (!handlePreviousOP( true ))
                return false;
        }
        else //Should have hit an EQ operator by the end of the line
            return false; //ERROR - can't mix assignment operators with subsequent logic operators

        if ( getNumNests() )
            return false; //ERROR - There are still nests that are initialized at the end of the line for some reason (syntax issue)
	}
	return true;
}

bool PLC_Parser::parser_ANDOP()
{
    if (!handlePreviousOP())
        return false;
	
	reset();
	setLastOP( CHAR_AND );
	return true; //success
}

bool PLC_Parser::parser_OROP()
{
	if (!handlePreviousOP() )
        return false;

	reset();
	setLastOP(CHAR_OR);
	return true; //success
}

bool PLC_Parser::parser_EQOP()
{
    if ( !handlePreviousOP(true) )
        return false;

	reset();
	setLastOP(CHAR_EQUALS);
	return true; //success
}

bool PLC_Parser::handlePreviousOP(bool append)
{
    shared_ptr<Ladder_OBJ_Wrapper> obj = createNewWrapper(PLCObj.findLadderObjByName(getParsedObjectStr()));
    if ( obj ) //Have we found a valid object?
    {
        bool nested = getNumNests() > 0;
        if ( !getNumLogicOPs() )
        {
            if ( !addLogicOP(obj) )
            {
                return false; //error
            }
        }
        else if ( getLastOP() == CHAR_OR ) //create a new logic op
        {
            if ( nested )
            {
                if ( !getLogicOP()->addParallelObject(obj, nested) )
                {
                    return false;
                }
            }
            else
            {
                if ( !addLogicOP(obj) ) //TODO: This is different if we are in a nest
                {
                    return false; //error
                }
            }
        }
        else if ( getLastOP() == CHAR_AND )
        {
            if ( !getLogicOP()->addSeriesObject(obj, nested) ) //add the object to the most recently initialized logicOP
            {
                return false; //error
            }
        }
        else if ( getLastOP() == CHAR_EQUALS)
        {
            if (!append)
                return false; //ERROR - can't mix assignment operators with subsequent logic operators
            else
            {
                if (!handleEQOP(obj) )
                {
                    return false;
                }
            }
        }

        if ( iRequestedNestDepth < getNumNests() ) //modify the nest count AFTER performing logic operations.
            setNumNests( iRequestedNestDepth );
    }
    else  //object doesn't exist, so create it and add to the list of parsed objects (ladder object is explicitly being initialized)
	{
		shared_ptr<ladderOBJdata> newObj = PLCObj.createNewObject(getParsedObjectStr(), parseObjectArgs());
		if ( newObj )
			PLCObj.addParsedObject(newObj); //place the newly reated object into the list of parsed (and created) ladder objects.
		else
        {
            sendError(ERR_DATA::ERR_INVALID_OBJ, getParsedObjectStr());
			return false; //failed to create the new object for some reason
        }
	}

    return true;
}

bool PLC_Parser::handleEQOP( shared_ptr<Ladder_OBJ_Wrapper> obj )
{
    for (uint8_t x = 0; x < getNumLogicOPs(); x++ )	
    {
        for (uint8_t y = 0; y < getLogicOPs()[x]->getNumCurrentObjects(); y++ )
        {
            if ( !getLogicOPs()[x]->getCurrentObjects()[y]->addNextObject(iRung, obj) )
                return false;
        }
    }
    return true;
}

vector<String> PLC_Parser::parseObjectArgs()
{
    String parsed;
	vector<String> ObjArgs;
	getLinePos()++; //Skip the operator (=)
	for ( ;getLinePos() < getParsedLineStr().length(); getLinePos()++ ) //At this point, we're parsing the arguments within the () of an object. IE: OUTPUT( PIN )
	{
		switch(getParsedLineStr()[getLinePos()])
		{
			case CHAR_P_START:	
			case CHAR_P_END:
			case CHAR_COMMA:
			{
				#ifdef DEBUG
				Serial.println(parsed);
				#endif
				ObjArgs.push_back(parsed); 
				parsed.clear();
				continue;
			}
			break;
			default:
			{
				parsed += getParsedLineStr()[getLinePos()];
			}
		}
	}

    return ObjArgs;
}

void PLC_Parser::sendError(uint8_t err, const String &str)
{ 
    PLCObj.sendError(err,str);
}
