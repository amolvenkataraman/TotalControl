#pragma once

#include <cstdlib>
#include <fstream>
#include <string>

// Shared secret between the launcher and the command server.
// Read from $TOTALCONTROL_TOKEN, or else the first line of ~/.config/totalcontrol/token.
// Returns an empty string if neither is set.
inline std::string load_token() {
    if (const char* env = std::getenv("TOTALCONTROL_TOKEN"); env && *env) {
        return env;
    }
    const char* home = std::getenv("HOME");
    if (!home) return "";

    std::ifstream file(std::string(home) + "/.config/totalcontrol/token");
    std::string token;
    std::getline(file, token);
    return token;
}

// Compare in constant time so the token can't be guessed from response timing.
inline bool token_matches(const std::string& given, const std::string& expected) {
    if (given.size() != expected.size()) return false;
    unsigned char diff = 0;
    for (std::size_t i = 0; i < given.size(); ++i) {
        diff |= static_cast<unsigned char>(given[i] ^ expected[i]);
    }
    return diff == 0;
}
