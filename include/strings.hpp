#pragma once
#include <iostream>
#include <sstream>
#include <string>
#include <vector>


namespace strings {
    // No split operation in C++, so...
    std::vector<std::string> split(const std::string &str, const char delimiter) {
        std::vector<std::string> tokens;
        std::stringstream ss(str);
        std::string token;

        while (std::getline(ss, token, delimiter)) {
            tokens.push_back(token);
        }

        return tokens;
    }

    std::string join(const std::vector<std::string>& tokens, const std::string& delimiter) {
        std::string result;
        for (size_t i = 0; i < tokens.size(); ++i) {
            result += tokens[i];
            if (i != tokens.size() - 1) {
                result += delimiter;
            }
        }
        return result;
    }
    
} // namespace strings
