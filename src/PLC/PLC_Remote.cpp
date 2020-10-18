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
            client.println(handleRequest(client.readStringUntil(CHAR_NEWLINE))); //Figure out what the client wants and then write the reply
        }
        
        client.stop(); //end this connection after sending the reply
    }
}

String PLC_Remote_Server::handleRequest( const String &request )
{
    String tempStr = removeFromStr(request, {CHAR_NEWLINE, CHAR_CARRIAGE} );

    if ( strBeginsWith(tempStr, CMD_REQUEST_UPDATE) ) //Requesting update info for an object that is already initialized on the remote client.
    {
        tempStr = removeFromStr( tempStr, CMD_REQUEST_UPDATE );
    }
    else if ( strBeginsWith(tempStr, CMD_REQUEST_INIT) ) //requesting enough information to initialize a new object on the client
    {
        tempStr = removeFromStr( tempStr, CMD_REQUEST_INIT );
    }
    else
    {
        tempStr = CMD_REQUEST_INVALID; //the request does not match any of the valid request types.
    }

    return tempStr;
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

/*
*
* Below here are functions that are declared in PLC_Main that pertain to remote communications (via wifi)
*
*/

bool PLC_Main::createRemoteServer( uint16_t port )
{
	if ( getRemoteServer() && getRemoteServer()->getPort() == port )
		return false; //already initialized on that port. Do nothing 

	getRemoteServer() = unique_ptr<PLC_Remote_Server>( new PLC_Remote_Server(port) );
	return true;
}

vector<IPAddress> PLC_Main::scanForRemoteNodes( uint16_t port, uint8_t low, uint8_t high, uint16_t timeout )
{
    vector<IPAddress> ipAddrs;
    if ( low >= high )
        return ipAddrs; //nothing to do

    for ( uint8_t x = low; x < high; x++ )
    {
        IPAddress addr(192,168,0,x); //could use some tweaking
        shared_ptr<WiFiClient> newClient = make_shared<WiFiClient>();
        if ( newClient->connect( addr, port, timeout ) ) //were we able to connect to the inputted IP address on the given port?
        {
            Core.sendMessage(PSTR("Found node at: ") + addr.toString() );
            ipAddrs.push_back( newClient->remoteIP() );
        }
    }

    return ipAddrs;
}
