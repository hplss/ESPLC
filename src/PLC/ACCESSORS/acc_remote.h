#ifndef PLC_REMOTE_CLIENT
#define PLC_REMOTE_CLIENT

#include "../PLC_IO.h"
#include "../PLC_Main.h"

//the PLC_Remote_Client class represents another ESP32 device that processes its own ladder logic operations, and shares data between the current device and itself, thereby enabling tethering/cluster operations
class PLC_Remote_Client : public Ladder_OBJ_Accessor
{
	public:
	PLC_Remote_Client( const String &, const IPAddress &, uint16_t, uint32_t = 2000, uint32_t = 1000, uint8_t = 10 );
	~PLC_Remote_Client() //deconstructor
	{
		nodeClient.stop();
	}

	//This updates an individual remote object, if an update is requested.
	bool updateRemoteObject();
	//This function checks to see if there are any updates that need to be processed.
	virtual void updateObject();
	//Send an update request to the client node for the corresponding local Ladder_OBJ.
	bool sendUpdateRequest( shared_ptr<Ladder_OBJ_Logical> );
	//This virtual function is called when searching for an object that is supposed to be managed by this accessor object.
	virtual shared_ptr<Ladder_OBJ_Logical> findAccessorVarByID( const String & );
	//Returns the stored IP address pertaining to the remote host.
	const IPAddress &getHostAddress(){ return ip_hostAddress; }
	//Send a specific update request command to the host device, and returns the data String that is received.
    String requestFromHost(const vector<String> &);
	String requestFromHost( const String &);
	//Returns the port that the remote update server is accepting requests on.
	//Performs a simple check to make sure that we are still capable of talking to a remote host.
	bool checkNetworkConnection();

	const uint16_t iHostPort;
	
	private: 
	uint32_t i_timeout;
    uint32_t i_nextUpdate,
             i_updateFreq;
	uint8_t i_numRetries;

	WiFiClient nodeClient;
	IPAddress ip_hostAddress; //This is the address for the remote server.

	bool b_enabled;
};

#endif