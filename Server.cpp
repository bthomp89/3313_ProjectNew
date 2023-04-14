#include "thread.h"
#include "socketserver.h"
#include <stdlib.h>
#include <time.h>
#include <list>
#include <vector>
#include <algorithm>

using namespace Sync;
// This thread handles each client connection
class SocketThread : public Thread
{
private:
    
    // The data we are receiving
    ByteArray data;

    // End the thread
    bool& endThread;

public:
    // Reference to our connected socket
    Socket& socket;

    SocketThread(Socket& socket, bool& endThread)
    : socket(socket), endThread(endThread) {}

    ~SocketThread()
    {}

    Socket& GetSocket()
    {
        return socket;
    }

    virtual long ThreadMain()
    {
        while(!endThread)
        {
            try
            {
                if (endThread) {
                    socket.Close();
                    delete this;
                }
                while(socket.Read(data) > 0){
                    std::string response = data.ToString();
                    // convert string to upper case
                    
                   //Notifies client has closed
                    std::string done = "DONE";
                    if (response == done) {
                        std::cout<<"Client has closed...\n";
                    }

                    //Closing server command entered
                    std::string close = "CLOSE";
                    if (response == close) {
                        std::cout<<"Terminating server...\n";
                        endThread = true;
                    }

                    socket.Write(response); //send back 

                }

            }
            catch (...)
            {
                // ???
                endThread = true;
            }
        }
		
	// ???

        return 0;
    }
};

// This thread handles the server operations
class ServerThread : public Thread
{
private:
    std::vector<SocketThread*>  socketThreads;
    bool endThread = false;
public:
    SocketServer& server;
    ServerThread(SocketServer& server, std::vector<SocketThread*> socketThreads)
    : server(server), socketThreads(socketThreads)
    {}


    ~ServerThread()
    {
        //Cleanup
        for (auto thread : socketThreads)
        {
            try
            {
                // Close the socket
                Socket& toClose = thread->GetSocket();
                toClose.Close();
                delete thread;
            }
            catch (...){
                endThread = true;
            }
        }

        // endThread the thread loops
        endThread = true;
    }

    virtual long ThreadMain()
    {
        while(!endThread){
            try{
                // Wait for a client socket connection
                Socket* newConnection = new Socket(server.Accept());

                // Pass a reference to this pointer into a new socket thread
                Socket& socketReference = *newConnection;
                socketThreads.push_back(new SocketThread(socketReference, endThread));
            }
            catch(...)
            {
                endThread = 1;
            }
        }
    }
};


int main(void)
{
    // Seed the random number generator with the current time
  std::srand(static_cast<unsigned>(std::time(0)));

  // Generate a random number between 1024 and 65535
  int serverPort = std::rand() % (65535 - 1024 + 1) + 1024;
    std::cout << "I am a server." << std::endl;
	std::cout << "Press enter to terminate the server...\n";
    std::cout.flush();

	 std::cout << "Your Match Code: " + std::to_string(serverPort) << std::endl;
    // Create our server
    SocketServer server(serverPort);   

    std::vector<SocketThread*> socketThreads; 

    // Need a thread to perform server operations
    ServerThread serverThread(server, socketThreads);
	
    // This will wait for input to shutdown the server
    FlexWait cinWaiter(1, stdin);
    cinWaiter.Wait();
    std::cin.get();

    // Shut down and clean up the server
    server.Shutdown();

}
