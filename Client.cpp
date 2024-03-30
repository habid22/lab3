#include "thread.h"
#include "socket.h"
#include <iostream>
#include <stdlib.h>
#include <time.h>

using namespace Sync;

class ClientWorker : public Thread {
private:
    Socket& connectionSocket;
    bool &isActive;
    ByteArray inputData;
    std::string inputString;

public:
    ClientWorker(Socket& connectionSocket, bool &isActive)
    : connectionSocket(connectionSocket), isActive(isActive) {}

    ~ClientWorker() {}

    virtual long ThreadMain() {
        while(true) {
            try {
                std::cout << "Enter your data (done to exit): ";
                std::cout.flush();

                std::getline(std::cin, inputString);
                inputData = ByteArray(inputString);

                if(inputString == "done") {
                    isActive = false;
                    break;
                }

                connectionSocket.Write(inputData);

                int connectionStatus = connectionSocket.Read(inputData);

                if(connectionStatus == 0) {
                    isActive = false;
                    break;
                }

                std::cout<<"Server Response: " << inputData.ToString() << std::endl;
            } catch (std::string err) {
                std::cout<<err<<std::endl;
            }
        }

        return 0;
    }
};

int main(void) {
    std::cout << "SE3313 Lab 3 Client" << std::endl;
    bool isActive = true;

    Socket connectionSocket("127.0.0.1", 3000);
    ClientWorker clientWorker(connectionSocket, isActive);
    connectionSocket.Open();

    while(isActive) {
        sleep(1);
    }

    connectionSocket.Close();
   
    return 0;
}
