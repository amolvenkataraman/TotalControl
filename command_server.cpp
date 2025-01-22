#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <string>
#include <cstring>
#include <unistd.h>       // close()
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

const int PORT = 3142;
const int BUFFER_SIZE = 1024;

std::mutex connectionMutex; // Mutex to synchronize access to shared resources
bool running = true;

// Function to handle a single client connection
void handleClient(int clientSocket, sockaddr_in clientAddress) {
    char buffer[BUFFER_SIZE];
    std::string clientIP = inet_ntoa(clientAddress.sin_addr);
    int clientPort = ntohs(clientAddress.sin_port);

    std::cout << "Client connected: " << clientIP << ":" << clientPort << std::endl;

    while (true) {
        memset(buffer, 0, BUFFER_SIZE);
        ssize_t bytesRead = recv(clientSocket, buffer, BUFFER_SIZE - 1, 0);

        if (bytesRead <= 0) {
            // Connection closed or error
            std::cout << "Client disconnected: " << clientIP << ":" << clientPort << std::endl;
            close(clientSocket);
            break;
        }

        // Print received message
        std::cout << "Message from " << clientIP << ":" << clientPort << ": " << buffer << std::endl;

        // Echo the message back to the client
        std::string response = "Server received: " + std::string(buffer);
        send(clientSocket, response.c_str(), response.length(), 0);
    }
}

int main() {
    // Create a socket
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        std::cerr << "Error creating socket!" << std::endl;
        return 1;
    }

    // Allow address reuse
    int opt = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // Configure server address
    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(PORT);

    // Bind the socket to the address
    if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        std::cerr << "Error binding socket!" << std::endl;
        close(serverSocket);
        return 1;
    }

    // Start listening for connections
    if (listen(serverSocket, 10) < 0) {
        std::cerr << "Error listening on socket!" << std::endl;
        close(serverSocket);
        return 1;
    }

    std::cout << "Server is listening on port " << PORT << "..." << std::endl;

    // Vector to store threads for all client connections
    std::vector<std::thread> threads;

    while (running) {
        sockaddr_in clientAddress{};
        socklen_t clientAddressLen = sizeof(clientAddress);

        // Accept a new connection
        int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientAddressLen);
        if (clientSocket < 0) {
            std::cerr << "Error accepting connection!" << std::endl;
            continue;
        }

        // Create a new thread to handle the client
        std::lock_guard<std::mutex> lock(connectionMutex);
        threads.emplace_back(std::thread(handleClient, clientSocket, clientAddress));
    }

    // Join all threads before exiting
    for (auto& thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }

    // Close the server socket
    close(serverSocket);
    std::cout << "Server shutting down..." << std::endl;

    return 0;
}
