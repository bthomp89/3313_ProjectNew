#include "thread.h"
#include "socket.h"
#include <iostream>
#include <stdlib.h>
#include <time.h>

using namespace Sync;

// This thread handles the connection to the server
class ClientThread : public Thread
{
private:
	// Reference to our connected socket
	Socket& socket;

	// Reference to end the thread
	bool& endThread;

	// Are we connected?
	bool connected = false;

	// Data to send to server
	ByteArray data;
	std::string data_str;

	//To prevent sending nothing
	int dataLength = 0;

public:
	ClientThread(Socket& socket, bool& endThread)
	: socket(socket), endThread(endThread)
	{}

	~ClientThread()
	{}

	bool isConnected() {
		return connected;
	}
	
	Socket& GetSocket()
    {
        return socket;
    }

	void ConnectServer()
	{
		try
		{
			std::cout << "Connecting...";
			std::cout.flush();
			socket.Open(); //To Open the socket
			connected = true;
			std::cout << "OK" << std::endl;
		}
		catch (...)
		{
			std::cout << "Waiting..." << std::endl;
			return;
		}
	}

	virtual long ThreadMain()
	{
		// Loop to ensure connetion
		int i = 0;
		while (true)
		{
			// Attempt to connect
			ConnectServer();

			// If exiting or connection established...
			if (endThread || connected)
			{
				break;
			}

			// Pause attempt to connect
			std::cout << "Reconnecting..." << std::endl;
			sleep(4);
			i++;
			if(i == 4) {
				endThread = true;
				std::cout<<"Connection timed out!\n";
				break;
			}

		}

		//Loop to ensure we haven't terminated program
		while (!endThread)
		{
			std::cout<<"Enter anything (done to exit): \n";
			// We are connected, perform our operations
			std::cout.flush();

			// Get the data
			data_str.clear();
			std::getline(std::cin, data_str);
			data = ByteArray(data_str);

			// Must have data to send
			dataLength = data_str.size();
			if (dataLength == 0)
			{
				std::cout << "<--Not Sent | Empty Data-->" << std::endl;
				continue;
			}
			else if (data_str == "done")
			{
				std::cout << "Closing the client..." << std::endl;
				endThread = true;
				socket.Write(data);
				break;
			}



			// If no bytes received
			if (socket.Write(data) <= 0)
			{
				std::cout << "Server failed to respond. Closing client..." << std::endl;
				endThread = true;
			}

			//DISPLAY RESPONSE
			socket.Read(data);
			if(data.ToString().size() == 0)
			{
				std::cout<<"Server failed to respond... Terminating client\n";
				endThread = true;
				break;
			}
			std::cout<< "Server Response: " + data.ToString() << std::endl;
			
		}

		return 0;
	}
};

int main(void)
{
	int gamePort = 0;
	// Welcome the user and try to initialize the socke

	std::cout<<"Enter a Match Code"<<std::endl;
	std::cin>>gamePort;

	// Create our socket
	Socket socket("127.0.0.1", gamePort);
	bool endThread = false;

	// Scope to kill thread
	{
		// Thread to perform socket operations on
		ClientThread clientThread(socket, endThread);

		//Loop for clientThread
		while(!endThread)
		{
			sleep(1);
		}
		
		// Wait to make sure the thread is cleaned up
		std::cout<<"Waiting for thread to close...\n";
	}
	
	// Attempt to close the socket
	try
	{
		std::cout<<"Closing socket...\n";
		socket.Close();
	}
	catch (...)
	{}

	return 0;
}