#include "thread.h"
#include "socketserver.h"
#include <stdlib.h>
#include <time.h>
#include <list>
#include <vector>
#include <algorithm>

using namespace Sync;

class ClientHandler : public Thread {
private:
    Socket& clientSocket;
    ByteArray clientData;
    bool &shouldTerminate;
    std::vector<ClientHandler*> &handlerList;

public:
    ClientHandler(Socket& clientSocket, bool &shouldTerminate, std::vector<ClientHandler*> &handlerList)
    : clientSocket(clientSocket), shouldTerminate(shouldTerminate), handlerList(handlerList) {}

    ~ClientHandler() {
        this->terminationEvent.Wait();
    }

    Socket& GetSocket() {
        return clientSocket;
    }

    virtual long ThreadMain() {
        try {
            while(!shouldTerminate) {
                clientSocket.Read(clientData);

                std::string response = clientData.ToString();
                std::for_each(response.begin(), response.end(), [](char & response){
                    response = ::toupper(response);
                });

                if (response=="DONE") {
                    handlerList.erase(std::remove(handlerList.begin(), handlerList.end(), this), handlerList.end());
                    shouldTerminate = true;
                    break;      
                }

                response.append("-received");
                clientSocket.Write(ByteArray(response));
            }
        } catch (std::string &s) {
            std::cout<<s<<std::endl;
        } catch (std::string err) {
            std::cout<<err<<std::endl;
        }

        std::cout<<"Client Disconnected" <<std::endl;
        return 0;
    }
};

class ServerWorker : public Thread {
private:
    SocketServer& serverInstance;
    bool shouldTerminate = false;
    std::vector<ClientHandler*> handlerList;

public:
    ServerWorker(SocketServer& serverInstance)
    : serverInstance(serverInstance) {}

    ~ServerWorker() {
        for(auto thread: handlerList) {
            try{
                Socket& toClose = thread->GetSocket();
                toClose.Close();
            } catch (...) {}
        }

        std::vector<ClientHandler*>().swap(handlerList);
        std::cout<<"Closing client from server"<<std::endl;
        shouldTerminate = true;
    }

    virtual long ThreadMain() {
        while (1) {
            try {
                Socket* newConnection = new Socket(serverInstance.Accept());
                ThreadSem serverBlock(1);
                Socket& socketReference = *newConnection;
                handlerList.push_back(new ClientHandler(socketReference, shouldTerminate, std::ref(handlerList)));
            } catch (std::string error) {
                std::cout << "ERROR: " << error << std::endl;
                return 1;
            } catch (TerminationException terminationException) {
                std::cout << "Server has shut down!" << std::endl;
                return terminationException;
            }
        }

        return 0;
    }
};

int main(void) {
    std::cout << "I am a server." << std::endl;
    std::cout << "Press enter to terminate the server...";
    std::cout.flush();

    SocketServer serverInstance(3000);    
    ServerWorker serverWorker(serverInstance);

    FlexWait cinWaiter(1, stdin);
    cinWaiter.Wait();
    std::cin.get();

    serverInstance.Shutdown();
}
