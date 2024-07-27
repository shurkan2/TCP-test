#include <iostream>
#include <iomanip>
#include <string>
#include <chrono>
#include <thread>
#include <ctime>
#include <sstream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

class TCPClient {
public:
    TCPClient(const std::string& name, int port, int interval);
    void start();

private:
    std::string getCurrentTime();
    void sendMessage();

    std::string name;
    int port;
    int interval;
    int clientSocket;
    struct sockaddr_in serverAddr;
};

TCPClient::TCPClient(const std::string& name, int port, int interval) 
    : name(name), port(port), interval(interval) {
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket < 0) {
        throw std::runtime_error("Error opening socket");
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(clientSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        close(clientSocket);
        throw std::runtime_error("Error connecting to server");
    }
}

void TCPClient::start() {
    std::cout << "Connected" << std::endl;
    while (true) {
        sendMessage();
        std::this_thread::sleep_for(std::chrono::seconds(interval));
    }
}

void TCPClient::sendMessage() {
    std::string message = getCurrentTime() + " " + name;
    send(clientSocket, message.c_str(), message.size(), 0);
}

std::string TCPClient::getCurrentTime() {
    auto now = std::chrono::system_clock::now();
    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
    auto currentTime = std::chrono::system_clock::to_time_t(now);
    std::tm tm = *std::localtime(&currentTime);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S") << '.' << std::setfill('0') << std::setw(3) << milliseconds.count();
    return oss.str();
}

int main(int argc, char *argv[]) {
    if (argc < 4) {
        std::cerr << "Usage: client <name> <port> <interval>" << std::endl;
        return 1;
    }

    std::string name = argv[1];
    int port = std::stoi(argv[2]);
    int interval = std::stoi(argv[3]);

    try {
        TCPClient client(name, port, interval);
        client.start(); 
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}
