#include "acc_remote.h"

const String PROGMEM &connection = PSTR("Connection to: ");

PLC_Remote_Client::PLC_Remote_Client( const String &id, const IPAddress &addr, uint16_t port, uint32_t timeout, uint32_t updateFreq, uint8_t retries) 
    : Ladder_OBJ_Accessor( id, OBJ_TYPE::TYPE_REMOTE )
{
    i_hostPort = port;
    i_timeout = timeout;
    i_updateFreq = updateFreq;
    ip_hostAddress = addr;
    
    i_numRetries = retries;

    setState(true); //default to enabled -- maybe make a new ENUM for states tat can be used across all object types... TODO

    i_nextUpdate = millis();
}

String PLC_Remote_Client::requestFromHost(const vector<String> &cmdVector)
{
    String cmd;
    for ( uint16_t x = 0; x < cmdVector.size(); x++ )
    {
        if (!x)
            cmd += cmdVector[x];
        else
        {
            cmd += CHAR_UPDATE_RECORD + cmdVector[x];
        }
        
    }

    return requestFromHost(cmd);
}

String PLC_Remote_Client::requestFromHost(const String &cmd)
{
    String recvdData;
    uint8_t retries = 0;
    WiFiClient nodeClient;
    while ( retries < i_numRetries ) //were we able to connect to the inputted IP address on the given port?
    {
        if( nodeClient.connect(getHostAddress(), getHostPort(), i_timeout) )
        {
            uint32_t storedTime = millis();
            nodeClient.setNoDelay(true); //Send immediately (don't wait for significant packet size unless epcifically told to do so)
            nodeClient.setTimeout(0); 
            nodeClient.print(cmd + CHAR_TRANSMIT_END); //send some message
            //Wait to receive a reply...

            while( !nodeClient.available() && ((millis() - storedTime) < i_timeout) ){} //loop until the conditions are met

            if ( nodeClient.available() ) //Did we receive anything for realz?
            {
                nodeClient.setTimeout(0); 
                recvdData += nodeClient.readStringUntil(CHAR_TRANSMIT_END); //just remove these now
                Core.sendMessage( PSTR("TX Bytes: ") + String(cmd.length() + 1) + PSTR("RX Bytes: ") + String(recvdData.length()) + PSTR(" Latency: ") + String(millis() - storedTime) + " RSSI: " + WiFi.RSSI() + "dBm" ); //some stat
                //Should also tell number of bytes sent/received.
            }

            if (!recvdData.length())
                Core.sendMessage( PSTR("No valid response from host at: ") + getHostAddress().toString() );

            nodeClient.stop();
            return recvdData;
        }

        retries++; //increment before we try again.
    }

    Core.sendMessage(connection + getHostAddress().toString() + PSTR(" failed.") );
    setState(false); //Connection is no good, so disable this accessor

    return recvdData;
}

void PLC_Remote_Client::updateObject()
{
    if (!checkNetworkConnection() || !getState() )
        return; //end here if failed.

    if ( millis() > i_nextUpdate ) //time to update?
    {
        uint16_t numObjects = getObjectVARs().size();
        if ( numObjects ) //must have some objects initialized in order to reqest updates.
        {
            String newRequest(CMD_REQUEST_UPDATE); //init with update request
            for ( uint16_t x = 0; x < numObjects; x++ )
            {
                if ( x == numObjects )
                    newRequest += getObjectVARs()[x]->getID();
                else
                    newRequest += getObjectVARs()[x]->getID() + CHAR_UPDATE_RECORD; //split the requested object ID's up by the record char
            }
            newRequest += CHAR_QUERY_END;

            handleUpdates(requestFromHost(newRequest));
        }
        
        //Perform update logic here -- where all the magic happens
        i_nextUpdate = millis() + i_updateFreq;
    }

    Ladder_OBJ_Accessor::updateObject();
}

bool PLC_Remote_Client::checkNetworkConnection()
{
    if ( !WiFi.isConnected() ) //looks like the connection died? 
    {
        if ( getState() )
        {
            setState(false); //disabled now
            Core.sendMessage(connection + getHostAddress().toString() + PSTR(" interrupted."));
        }

        return false; //not connected to a network, so we can't do anything. It's that simple.
    }

    return true;
}

shared_ptr<Ladder_OBJ_Logical> PLC_Remote_Client::findLadderObjByID( const String &id )
{
    //shared_ptr<Ladder_OBJ_Logical> accObj = Ladder_OBJ_Accessor::findLadderObjByID(id); //is it already locally stored?
    shared_ptr<Ladder_OBJ_Logical> accObj = getObjectVAR(id);
    if ( !accObj ) //guess not, so we'll poll the remote host for it and initialize it as necessary.
    {
        //Request to initialize from the host
        accObj = handleInit(requestFromHost(CMD_REQUEST_INIT + id + CHAR_QUERY_END));
    }

    return accObj;
}