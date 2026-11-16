#include <iostream>
#include <thread>
#include <vector>
#include <map>
#include <mutex>
#include <string>
#include <cstring>
#include <cstdlib>
#include <unistd.h>       // close()
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "token.h"

const int PORT = 3142;
const int BUFFER_SIZE = 1024;

std::mutex connectionMutex; // Mutex to synchronize access to shared resources
bool running = true;
std::string AUTH_TOKEN; // Every message must start with this, followed by a newline

enum Command_Type {
    OS_COMMAND,
    XDG_OPEN,
    TERMINAL,
    KEYSTROKE,
};

std::map<std::string, Command_Type> COMMAND_TYPE_MAP = {
    {"os", OS_COMMAND},
    {"xdg", XDG_OPEN},
    {"key", KEYSTROKE},
};

void call_thread(std::string param) {
    std::thread t(system, param.c_str());
    t.join();
}

struct Command {
    std::string code;
    std::string description;

    Command_Type type;
    std::string params;
    bool multiple = false;

    // Command(const std::string c, const std::string d, const Command_Type t, const std::string p):
    //     code{c}, description{d}, type{t}, params{p} {}
    // Command(std::string code, std::string desc, std::string type, std::string param):
    //     code{code}, description{desc}, type{COMMAND_TYPE_MAP[type]}, params{param} {}

    void execute(std::string custom_params = "") {
        if (type == OS_COMMAND) {
            call_thread(params);
        }
        else if (type == XDG_OPEN) {
            call_thread("xdg-open " + params);
        }
        else if (type == TERMINAL) {
            call_thread("gnome-terminal -- " + params);
        }
    }
};

std::map<std::string, Command> COMMANDS = {
    {"uw", Command{"uw", "Open UW Folder", XDG_OPEN, "~/Desktop/UWaterloo/3A_Winter_2025"}},
    {"l", Command{"l", "Open Learn", XDG_OPEN, "https://learn.uwaterloo.ca/d2l/home"}},
    {"cm", Command{"cm", "Open Crowdmark", XDG_OPEN, "https://app.crowdmark.com/student/courses"}},
    {"ww", Command{"ww", "Open WaterlooWorks", XDG_OPEN, "https://waterlooworks.uwaterloo.ca/home.htm"}},
    {"res", Command{"res", "Open my Resume", XDG_OPEN, "~/Documents/Resume/Amol_Venkataraman-RESUME-2025.pdf"}},
    {"ts", Command{"ts", "Open my Transcript", XDG_OPEN, "~/Documents/Resume/SSR_TSRPT.pdf"}},
    {"t", Command{"t", "Open terminal", OS_COMMAND, "gnome-terminal"}},
    {"hs", Command{"hs", "Open Housesigma", XDG_OPEN, "https://housesigma.com/on/map/"}},
    {"map", Command{"map", "Open Google Maps", XDG_OPEN, "https://maps.google.com/"}},
    {"chat", Command{"chat", "Open ChatGPT", XDG_OPEN, "https://chatgpt.com/"}},
    {"htop", Command{"htop", "Run htop", TERMINAL, "htop"}},
    {"mc", Command{"mc", "Launch Minecraft", TERMINAL, "~/Games/UltimMC/bin/UltimMC"}},
    {"as", Command{"as", "Launch Android Studio", TERMINAL, "~/Android/android-studio/bin/studio.sh"}},
    {"auda", Command{"auda", "Launch Audacity", TERMINAL, "~/Downloads/audacity-linux-3.7.1-x64-20.04.AppImage"}},
    {"idea", Command{"idea", "Launch IntelliJ IDEA", TERMINAL, "/opt/ideaIC-2024.3.1.1/idea-IC-243.22562.218/bin/idea.sh"}},
};

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

        // Messages are "<token>\n<command>". Drop the connection on a missing or wrong token,
        // since commands can run arbitrary shell code (see the '.' prefix below).
        std::string message = buffer;
        std::size_t newline = message.find('\n');
        if (newline == std::string::npos || !token_matches(message.substr(0, newline), AUTH_TOKEN)) {
            std::cout << "Rejected unauthenticated message from " << clientIP << ":" << clientPort << std::endl;
            send(clientSocket, "Unauthorized", 12, 0);
            close(clientSocket);
            break;
        }
        std::string cmd = message.substr(newline + 1);
        if (cmd.empty()) continue;

        std::cout << "Command from " << clientIP << ":" << clientPort << ": " << cmd << std::endl;

        if (cmd[0] == '.') {
            call_thread(cmd.substr(1));
        }
        else if (cmd[0] == '-') {
            call_thread("gnome-terminal -- " + cmd.substr(1));
        }
        else if (COMMANDS.count(cmd)) {
            COMMANDS[cmd].execute();
        }

        // Echo the message back to the client
        std::string response = "Server received: " + cmd;
        send(clientSocket, response.c_str(), response.length(), 0);
    }
}

int main() {
    AUTH_TOKEN = load_token();
    if (AUTH_TOKEN.empty()) {
        std::cerr << "No token: set TOTALCONTROL_TOKEN or write one to ~/.config/totalcontrol/token" << std::endl;
        return 1;
    }

    // Create a socket
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        std::cerr << "Error creating socket!" << std::endl;
        return 1;
    }

    // Allow address reuse
    int opt = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // Configure server address: localhost only, never other machines on the network
    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
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
