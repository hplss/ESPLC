/*
* The PLC_Remote file contains ny function definitions related to the communication to and from other ESPLC devices that are initialied on the LAN (or possibly WAN). 
* This includes both PLC_Remote_Server and PLC_Remote_Client object type defintions. Also stored here are some of the definitions that relate to remote functionality as 
* prototypes in PLC_Main.
*  Author: Andrew Ward
*  Date: 9/12/2020
*/

#include "PLC_Main.h"
#include "./ACCESSORS/acc_remote.h"

PLC_Remote_Server::PLC_Remote_Server( uint16_t port )
{
    localServer = make_shared<WiFiServer>( port ); //init on the given port
    localServer->begin( port ); //begin the server
    localServer->setNoDelay(true); //Send data immediately (don't wait for significant packet size unless epcifically told to do so)
    i_Port = port; //store away
    Core.sendMessage(PSTR("Starting Remote Polling Server"));
}

PLC_Remote_Server::~PLC_Remote_Server()
{
    Core.sendMessage(PSTR("Stopping Remote Polling Server"));
    localServer->stop();
    localClients.clear();
}

void PLC_Remote_Server::processRequests()
{
    WiFiClient client = localServer->available();

    if ( client )
    {
        uint32_t storedTime = millis(); //Store the current time

        IPAddress addr = client.remoteIP();
        while( !client.available() && (millis() - storedTime) < 200 ){} //wait to receive a command from the connected client, break out if we have

        if ( client.available() ) //have we actually received something from?
        {
            client.setNoDelay(true); 
            client.setTimeout(200);
            client.println(handleRequest(removeFromStr(client.readStringUntil(CHAR_NEWLINE), {CHAR_NEWLINE, CHAR_CARRIAGE}))); //Figure out what the client wants and then write the reply
        }
        
        client.stop(); //end this connection after sending the reply
    }
}

String PLC_Remote_Server::handleRequest( const String &request ) 
{
    if ( strBeginsWith(request, CMD_REQUEST_UPDATE) ) //Requesting update info for an object that is already initialized on the remote client.
    {
        return handleUpdate(removeFromStr( request, CMD_REQUEST_UPDATE ));
    }
    else if ( strBeginsWith(request, CMD_REQUEST_INIT) ) //requesting enough information to initialize a new object on the client
    {
        return handleInit(removeFromStr( request, CMD_REQUEST_INIT ));
    }

    return String(CMD_REQUEST_INVALID) + CHAR_QUERY_END; //if strings are over a certain size.. split up? (Packet Size Management)
}

bool PLC_Remote_Server::clientExists( const WiFiClient &client )
{
    for ( uint8_t x = 0; x < localClients.size(); x++ )
    {
        if ( localClients[x] == client.remoteIP() )
            return true;
    }  

    return false;
}

String PLC_Remote_Server::handleInit( const String &str )
{
    String initList(CMD_SEND_INIT);
    vector<String> initObjects = splitString(str, CHAR_UPDATE_RECORD); //May have multiple init requests in a single line.. maybe not.

    for ( uint16_t x = 0; x < initObjects.size(); x++ )
    {
        vector <String> args = splitString( initObjects[x], CHAR_VAR_OPERATOR ); //see if we are requesting a var to init

        if ( args.size() > 1) // Looks like we have a var type object.
        {
            shared_ptr<Ladder_OBJ_Logical> pObj = PLCObj.findLadderObjByID(args[0]);

            if (pObj)
            {
                shared_ptr<Ladder_VAR> pVar = pObj->getObjectVAR( args[1] );
                //Format: <ID><TYPE><VALUE> // other arguments may be added later such as STATE, LOGIC, etc.
                if ( pVar )
                    initList += initObjects[x] + CHAR_UPDATE_RECORD + static_cast<uint16_t>(pVar->getType()) + CHAR_UPDATE_RECORD + pVar->getValueStr();
            }
        }
        else if ( args.size() ) //just a regular object
        {
            initList += CMD_REQUEST_INVALID; //for now, we only allow for the init of var objects
        }

        initList += ( (x == initObjects.size()) ? : CHAR_UPDATE_GROUP); //end the group list
    }

    return initList + CHAR_QUERY_END; //append the ending char and send off the string
}

String PLC_Remote_Server::handleUpdate( const String &str )
{
    //Update requests from the client will only contain the names (ID's) of the objects that need to be updated. 
    //Maybe at some point the Object ID's will be mapped to some numeric ID to reduce network traffic and computation overhead (increases RAM usage a bit)
    vector<String> updateObjects = splitString(str, CHAR_UPDATE_RECORD);

    String updateList(CMD_SEND_UPDATE); 

    //Update Objects may be (and probably are) variables that are stored inside of other objects. Accessed like TIMER1.DN, etc.
    for ( uint16_t x = 0; x < updateObjects.size(); x++ )
    {
        vector<String> args = splitString( updateObjects[x], CHAR_VAR_OPERATOR );

        if (args.size() > 1) //Got a variable type.
        { //<ID>,<VALUE> //For now  -- presumably the client device knows the object's type following the init
            shared_ptr<Ladder_OBJ_Logical> pObj = PLCObj.findLadderObjByID(args[0]);

            if (pObj)
            {
                shared_ptr<Ladder_VAR> pVar = pObj->getObjectVAR(args[1]);
                if (pVar)
                {
                    updateList += updateObjects[x] + CHAR_UPDATE_RECORD + pVar->getValueStr(); //split with a 'record' char
                }
            }
            else //invalid object
            {
                updateList += updateObjects[x] + CHAR_UPDATE_RECORD + CMD_REQUEST_INVALID; //Object does not exist. Let the client know
            }
        } 
        else if ( args.size() ) //Just a regular object
        { 
            updateList += CMD_REQUEST_INVALID; //for now
        }
        updateList += ( (x == updateObjects.size()) ? : CHAR_UPDATE_GROUP); //end the group list
    }

    return updateList + CHAR_QUERY_END; //end of update report.
}