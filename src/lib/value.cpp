#include "ASTNodes.h"
#include <stdexcept>

int Value::asInt() const {
    if (type != Type::Int) throw std::runtime_error("Not an integer value.");
    return intValue;
}

bool Value::asBool() const {
    if (type != Type::Bool) throw std::runtime_error("Not a boolean value.");
    return boolValue;
}