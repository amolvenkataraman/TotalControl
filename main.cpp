#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <iostream>
#include <chrono>
#include <thread>
#include <vector>

#define BGCOLOUR 185, 185, 185, 255
#define FGCOLOUR 20, 20, 20, 255

const int FPS = 30;
const int FRAME_DELAY = 1000 / FPS; // Milliseconds per frame

const int WIN_WIDTH = 400;
const int WIN_HEIGHT = 600;

const char* FONT_PATH = "/usr/share/fonts/opentype/fira/FiraMono-Medium.otf";


SDL_Texture* RenderText(SDL_Renderer* renderer, TTF_Font* font, const std::string& text, SDL_Color color, SDL_Rect& destRect, int x_pos = 0, int y_pos = 0, bool hcenter = false, bool vcenter = false) {
    // Render text surface
    SDL_Surface* textSurface = TTF_RenderText_Blended(font, text.c_str(), color);
    if (!textSurface) {
        std::cerr << "TTF_RenderText_Blended Error: " << TTF_GetError() << std::endl;
        return nullptr;
    }

    // Convert surface to texture
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    if (!textTexture) {
        std::cerr << "SDL_CreateTextureFromSurface Error: " << SDL_GetError() << std::endl;
        SDL_FreeSurface(textSurface);
        return nullptr;
    }

    // Set destination rect dimensions
    destRect.w = textSurface->w;
    destRect.h = textSurface->h;

    if (hcenter) destRect.x = (WIN_WIDTH - destRect.w) / 2;
    else destRect.x = x_pos;

    if (vcenter) destRect.y = (WIN_HEIGHT - destRect.h) / 2;
    else destRect.y = y_pos;

    SDL_FreeSurface(textSurface); // Free the surface after creating the texture
    return textTexture;
}

int main() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    if (TTF_Init() == -1) {
        std::cerr << "TTF_Init Error: " << TTF_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }



    int client_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (client_sock < 0) {
        std::cerr << "Socket creation failed\n";
        SDL_Quit();
        return 1;
    }

    // Server details
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);  // Port 8080
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Connect to server
    if (connect(client_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Connection failed\n";
        close(client_sock);
        SDL_Quit();
        return 1;
    }

    // Send message to background process
    const char* message = "Hello from SDL2 GUI App!";
    send(client_sock, message, strlen(message), 0);

    // Receive response from background process
    char buffer[128];
    ssize_t bytesReceived = recv(client_sock, buffer, sizeof(buffer) - 1, 0);
    if (bytesReceived > 0) {
        buffer[bytesReceived] = '\0';
        std::cout << "Received from background process: " << buffer << std::endl;
    }


    SDL_Window* window = SDL_CreateWindow("TotalControl", 
                                          SDL_WINDOWPOS_CENTERED, 
                                          SDL_WINDOWPOS_CENTERED, 
                                          WIN_WIDTH, WIN_HEIGHT, 
                                          SDL_WINDOW_SHOWN | SDL_WINDOW_BORDERLESS);

    if (!window) {
        std::cerr << "Failed to create window: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return -1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "Failed to create renderer: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    // Load a font
    TTF_Font* font_small = TTF_OpenFont(FONT_PATH, 12);
    if (!font_small) {
        std::cerr << "TTF_OpenFont Error: " << TTF_GetError() << std::endl;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }
    TTF_Font* font_med = TTF_OpenFont(FONT_PATH, 28);
    TTF_Font* font_large = TTF_OpenFont(FONT_PATH, 36);

    SDL_Rect textRect = {0, 0, 0, 0};

    std::string header = "Type out a command and press ENTER to run";
    std::string command = "";
    bool special_cmd = false;
    std::string special_txt = "";

    std::vector<std::pair<std::string, std::string> > available_commands = {
        {"se", "Types out university email"},
        {"sem", "Opens university email"},
        {"ssh", "Opens SSH into personal server"},
        {"sshstu", "Opens SSH into UW student server"},
        {"sshfcs", "Opens SSH into UW CSC server"},
    };

    bool running = true;
    SDL_Event event;

    while (running) {
        auto frameStart = SDL_GetTicks(); // Get the start time of the frame

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }

            else if (event.type == SDL_KEYDOWN) {
                SDL_Keycode keyPressed = event.key.keysym.sym;
                std::string keyName = SDL_GetKeyName(keyPressed);

                // Handle specific key events
                if (keyPressed == SDLK_ESCAPE) {
                    running = false;  // Quit when Escape is pressed
                }
                else if (keyPressed == SDLK_BACKSPACE) {
                    if (command.size()) {
                        command.pop_back();
                    }
                }
                else if (keyPressed == SDLK_RETURN || keyPressed == SDLK_KP_ENTER) {

                }

                else if (keyName.size() == 1) {
                    if (keyName[0] >= 48 && keyName[0] <= 57) {

                    }
                    else if (keyName[0] >= 65 && keyName[0] <= 90) {
                        command += keyName[0] + 32;
                    }
                }
            }
        }

        SDL_SetRenderDrawColor(renderer, BGCOLOUR); // Background color
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, FGCOLOUR);
        int line_y = 140;
        SDL_RenderDrawLine(renderer, 0, line_y, WIN_WIDTH, line_y);


        SDL_Texture* textTexture = RenderText(renderer, font_small, header, {FGCOLOUR}, textRect, 0, 20, true, false);
        SDL_RenderCopy(renderer, textTexture, NULL, &textRect);

        if (command.size()) {
            textTexture = RenderText(renderer, font_large, command, {FGCOLOUR}, textRect, 0, 60, true, false);
            SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
        }

        if (special_cmd) {
            textTexture = RenderText(renderer, font_small, special_txt, {FGCOLOUR}, textRect, 0, 240, true, false);
            SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
        }
        else {
            for (int i = 0; i < available_commands.size(); ++i) {
                textTexture = RenderText(renderer, font_med, available_commands[i].first, {FGCOLOUR}, textRect, 40, 160 + (i * 75), false, false);
                SDL_RenderCopy(renderer, textTexture, NULL, &textRect);

                textTexture = RenderText(renderer, font_small, available_commands[i].second, {FGCOLOUR}, textRect, 40, 200 + (i * 75), false, false);
                SDL_RenderCopy(renderer, textTexture, NULL, &textRect);

                textTexture = RenderText(renderer, font_large, std::to_string(i + 1), {FGCOLOUR}, textRect, WIN_WIDTH - 60, 160 + (i * 75), false, false);
                SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
            }
        }

        SDL_RenderPresent(renderer);

        // Frame rate limiting
        int frameTime = SDL_GetTicks() - frameStart; // Time taken for this frame
        if (frameTime < FRAME_DELAY) {
            SDL_Delay(FRAME_DELAY - frameTime); // Delay to maintain consistent FPS
        }
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    close(client_sock);
    return 0;
}
