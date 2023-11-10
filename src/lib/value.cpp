#include "ASTNodes.h"
#include <stdexcept>


// Retrieves the value of this Value object as a double.
double Value::asDouble() const {
    if (type != Type::Double) throw std::runtime_error("Not a double value.");
    return doubleValue;
}

// Retrieves the value of this Value object as a boolean.
bool Value::asBool() const {
    if (type != Type::Bool) throw std::runtime_error("Not a boolean value.");
    return boolValue;
}