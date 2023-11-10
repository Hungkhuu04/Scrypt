#include "ASTNodes.h"
#include <stdexcept>

double Value::asDouble() const {
    if (type != Type::Double) throw std::runtime_error("Not a double value.");
    return doubleValue;
}

bool Value::asBool() const {
    if (type != Type::Bool) throw std::runtime_error("Not a boolean value.");
    return boolValue;
}