#include "ScryptComponents.h"
#include <stdexcept>


// Value class implementation
Value::Value() : type(Type::Null) {}

Value::Value(double value) : type(Type::Double), doubleValue(value) {}

Value::Value(bool value) : type(Type::Bool), boolValue(value) {}

Value::Value(Function function)
    : type(Type::Function) {
    new (&functionValue) Function(std::move(function));
}

Value::~Value() {
    cleanUp();
}

Value::Value(const Value& other) {
    copyFrom(other);
}

Value::Value(Value&& other) noexcept {
    moveFrom(std::move(other));
}

Value& Value::operator=(const Value& other) {
    if (this != &other) {
        cleanUp();
        copyFrom(other);
    }
    return *this;
}

Value& Value::operator=(Value&& other) noexcept {
    if (this != &other) {
        cleanUp();
        moveFrom(std::move(other));
    }
    return *this;
}

void Value::cleanUp() {
    if (type == Type::Function) {
        functionValue.~Function();
    }
}

void Value::copyFrom(const Value& other) {
    type = other.type;
    switch (type) {
        case Type::Double:
            doubleValue = other.doubleValue;
            break;
        case Type::Bool:
            boolValue = other.boolValue;
            break;
        case Type::Function:
            new (&functionValue) Function(other.functionValue);
            break;
        case Type::Null:
            break;
    }
}

void Value::moveFrom(Value&& other) {
    type = other.type;
    switch (type) {
        case Type::Double:
            doubleValue = other.doubleValue;
            break;
        case Type::Bool:
            boolValue = other.boolValue;
            break;
        case Type::Function:
            new (&functionValue) Function(std::move(other.functionValue));
            break;
        case Type::Null:
            break;
    }
}

Value::Type Value::getType() const {
    return type;
}

double Value::asDouble() const {
    if (type != Type::Double) {
        throw std::runtime_error("Value is not a double.");
    }
    return doubleValue;
}

bool Value::asBool() const {
    if (type != Type::Bool) {
        throw std::runtime_error("Runtime error: condition is not a bool.");
    }
    return boolValue;
}

const Value::Function& Value::asFunction() const {
    if (type != Type::Function) {
        throw std::runtime_error("Value is not a function.");
    }
    return functionValue;
}

bool Value::isNull() const {
    return type == Type::Null;
}

// Scope class implementations
Scope::Scope(std::shared_ptr<Scope> parent) : parentScope(std::move(parent)) {}

void Scope::setVariable(const std::string& name, const Value& value) {
    variables[name] = value;
}

// Get a variable from this scope or parent scopes
Value* Scope::getVariable(const std::string& name) {
    auto it = variables.find(name);
    if (it != variables.end()) {
        return &it->second;
    } else if (parentScope) {
        return parentScope->getVariable(name);
    }
    return nullptr;  // Variable not found in any scope
}

const std::unordered_map<std::string, Value>& Scope::getVariables() const{
        return variables;
    }

std::shared_ptr<Scope> Scope::getParent() const { return parentScope; }

std::shared_ptr<Scope> Scope::copyScope() const {
    auto newScope = std::make_shared<Scope>(parentScope); // Copy the parent scope reference
    for (const auto& var : variables) {
        newScope->variables[var.first] = var.second; // Copy each variable
    }
    return newScope;
}

// ReturnException implementations
ReturnException::ReturnException(Value returnValue) : returnValue(std::move(returnValue)) {}

const Value& ReturnException::getValue() const { retu