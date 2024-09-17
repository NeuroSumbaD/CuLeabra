#include "strings.hpp"

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
