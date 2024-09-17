#pragma once
#include <iostream>
#include <sstream>
#include <string>
#include <vector>


namespace strings {
    // No split operation in C++, so...
    std::vector<std::string> split(const std::string &str, const char delimiter);

    std::string join(const std::vector<std::string>& tokens, const std::string& delimiter);
    
} // namespace strings
