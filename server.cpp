#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <thread>
#include <mutex>
#include <chrono>
#include <ctime>
#include <sstream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

class TCPServer {
public:
    TCPServer(int port);
    void start();

private:
    void handleClient(int clientSocket);

    int port;
    int serverSocket;
    std::mutex logMutex;
};

TCPServer::TCPServer(int port) : port(port) {
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        throw std::runtime_error("Error opening socket");
    }

    struct sockaddr_in serverAddr = {0};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        throw std::runtime_error("Error binding socket");
    }

    listen(serverSocket, 5);
}

void TCPServer::start() {
    while (true) {
        struct sockaddr_in clientAddr;
        socklen_t clientLen = sizeof(clientAddr);
        int clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientLen);
        if (clientSocket >= 0) {
            std::thread clientThread(&TCPServer::handleClient, this, clientSocket);
            clientThread.detach();
        }
    }
}

void TCPServer::handleClient(int clientSocket) {
    char buffer[256];
    int nread;
    std::ofstream logFile("log.txt", std::ios_base::app);

    while ((nread = read(clientSocket, buffer, 255)) > 0) {
        buffer[nread] = '\0';
        std::lock_guard<std::mutex> guard(logMutex);
        logFile << buffer << std::endl;
    }

    logFile.close();
    close(clientSocket);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: server <port>" << std::endl;
        return 1;
    }

    int port = std::stoi(argv[1]);
    try {
        TCPServer server(port);
        server.start();
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}
