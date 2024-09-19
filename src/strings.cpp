#include "strings.hpp"
#include <iostream>
#include <string>
#include <algorithm>

std::vector<std::string> strings::split(const std::string &str, const char delimiter){
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;

    while (std::getline(ss, token, delimiter)) {
        tokens.push_back(token);
    }

    return tokens;
}

std::string strings::join(const std::vector<std::string> &tokens, const std::string &delimiter){
    std::string result;
    for (size_t i = 0; i < tokens.size(); ++i) {
        result += tokens[i];
        if (i != tokens.size() - 1) {
            result += delimiter;
        }
    }
    return result;
}

//Removes leading and trailing whitespace characters
std::string strings::TrimSpace(std::string str) {    
    // Find the first non-whitespace character
    auto start = std::find_if_not(str.begin(), str.end(), [](unsigned char ch) {
        return std::isspace(ch);
    });

    // Find the last non-whitespace character
    auto end = std::find_if_not(str.rbegin(), str.rend(), [](unsigned char ch) {
        return std::isspace(ch);
    }).base();

    // If the string is all spaces, end will be before start
    if (start >= end) {
        return "";
    }

    return std::string(start, end);

}
