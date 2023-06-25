#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <iterator>
#include <Winsock2.h>
#pragma comment(lib, "ws2_32.lib")

#define MAX_BUF_SIZE 1024

// Function to send VISCA command to the specified IP address and port
bool sendVISCACommand(const std::string& ipAddress, int port, const std::string& command) {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Failed to initialize Winsock." << std::endl;
        return false;
    }

    SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == INVALID_SOCKET) {
        std::cerr << "Failed to create socket." << std::endl;
        WSACleanup();
        return false;
    }

    sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = inet_addr(ipAddress.c_str());

    if (connect(sock, (struct sockaddr*)&server, sizeof(server)) < 0) {
        std::cerr << "Failed to connect to the remote server." << std::endl;
        closesocket(sock);
        WSACleanup();
        return false;
    }

    // Convert the command string to a byte array
    std::vector<uint8_t> commandBytes;
    std::string hexCommand = command.substr(2); // Remove the leading '81'
    for (size_t i = 0; i < hexCommand.length(); i += 2) {
        std::string byteString = hexCommand.substr(i, 2);
        uint8_t byte = static_cast<uint8_t>(std::stoi(byteString, nullptr, 16));
        commandBytes.push_back(byte);
    }

    if (send(sock, reinterpret_cast<const char*>(commandBytes.data()), static_cast<int>(commandBytes.size()), 0) < 0) {
        std::cerr << "Failed to send the command." << std::endl;
        closesocket(sock);
        WSACleanup();
        return false;
    }

    closesocket(sock);
    WSACleanup();

    return true;
}

int main(int argc, char* argv[]) {
    std::string ipAddress;
    int port = 0;
    std::string command;
    std::string slot;

    // Check if required arguments are provided
    if (argc != 8) {
        std::cerr << "Missing required arguments." << std::endl;
        std::cerr << "Usage: cameracontrol -i <IP address> -p <port number> -m <slot number> -s (or -l)" << std::endl;
        return 1;
    }

    // Parse command line arguments
    for (int i = 1; i < argc; i += 1) {
        std::string arg = argv[i];
        if (arg == "-i" && (i + 1) < argc) {
            ipAddress = argv[i + 1];
            std::cout << ipAddress << std::endl;
        }
        else if (arg == "-p" && (i + 1) < argc) {
            try {
                port = std::stoi(argv[i + 1]);
                std::cout << port << std::endl;
            }
            catch (const std::exception& e) {
                std::cerr << "Invalid port number argument." << std::endl;
                std::cerr << "Usage: cameracontrol -i <IP address> -p <port number> -m <slot number> -s (or -l)" << std::endl;
                return 1;
            }
        }
        else if (arg == "-m" && (i + 1) < argc) {
            slot = argv[i + 1];
        }
        else if (arg == "-s" || arg == "-l") {
            command = arg == "-s" ? "8101043F01FF" : "8101043F02FF";
        }
    }
    std::cout << slot << std::endl;
    std::cout << command << std::endl;

    // Insert slot value into command
    if (!command.empty()) {
        command.insert(10, slot);
    }

    // Check if required arguments are provided
    if (ipAddress.empty()) {
        std::cerr << "Missing IP address argument." << std::endl;
        std::cerr << "Usage: cameracontrol -i <IP address> -p <port number> -m <slot number> -s (or -l)" << std::endl;
        return 1;
    }

    if (port == 0) {
        std::cerr << "Missing port number argument." << std::endl;
        std::cerr << "Usage: cameracontrol -i <IP address> -p <port number> -m <slot number> -s (or -l)" << std::endl;
        return 1;
    }

    if (command.empty()) {
        std::cerr << "Missing save/load command argument." << std::endl;
        std::cerr << "Usage: cameracontrol -i <IP address> -p <port number> -m <slot number> -s (or -l)" << std::endl;
        return 1;
    }

    // Send the VISCA command
    if (!sendVISCACommand(ipAddress, port, command)) {
        std::cerr << "Failed to send the VISCA command." << std::endl;
        return 1;
    }

    std::cout << "VISCA command sent successfully." << std::endl;
    return 0;
}
