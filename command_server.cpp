#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

int main() {
    // Create socket
    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0) {
        std::cerr << "Socket creation failed\n";
        return 1;
    }

    // Bind the socket to a port
    struct sockaddr_in server_addr, client_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);  // Port 8080
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Bind failed\n";
        close(server_sock);
        return 1;
    }

    // Listen for incoming connections
    listen(server_sock, 5);

    // Accept client connection
    socklen_t client_len = sizeof(client_addr);
    int client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &client_len);
    if (client_sock < 0) {
        std::cerr << "Accept failed\n";
        close(server_sock);
        return 1;
    }

    // Receive message from client
    char buffer[128];
    ssize_t bytesReceived = recv(client_sock, buffer, sizeof(buffer) - 1, 0);
    if (bytesReceived > 0) {
        buffer[bytesReceived] = '\0';
        std::cout << "Received from client: " << buffer << std::endl;

        // Send response to client
        const char* response = "Hello from background process!";
        send(client_sock, response, strlen(response), 0);
    }

    // Clean up
    close(client_sock);
    close(server_sock);
    return 0;
}
