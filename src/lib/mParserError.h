#ifndef PARSE_ERROR_H
#define PARSE_ERROR_H

#include <stdexcept>
#include <string>

// A simple parse error exception type that contains an error message
class ParseError : public std::runtime_error {
public:
    
    explicit ParseError(const std::string& message) : std::runtime_error(message) {}
};

#endif 
