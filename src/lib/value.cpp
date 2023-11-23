
#include "ScryptComponents.h"
#include <stdexcept>
#include <cmath>


// Value class implementation
Value::Value() : type(Type::Null) {}

Value::Value(double value) : type(Type::Double), doubleValue(value) {}

Value::Value(bool value) : type(Type::Bool), boolValue(value) {}

Value::Value(Function function)
    : type(Type::Function) {
    new (&functionValue) Function(std::move(function));
}

Value::Value(std::vector<Value> array)
    : type(Type::Array), arrayValue(std::make_shared<std::vector<Value>>(std::move(array))) {}


Value::Value(FunctionPtr func) : type(Type::BuiltinFunction), builtinFunction(func) {}

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
    if (type != Type::Null) {
        switch (type) {
            case Type::Double:
            case Type::Bool:
                // These types don't require special handling.
                break;
            case Type::Function:
                // Explicitly call the destructor for Function
                functionValue.~Function();
                break;
            case Type::Array:
                // Reset the shared pointer
                arrayValue.reset();
                break;
            case Type::BuiltinFunction:
                // Explicitly call the destructor for FunctionPtr
                builtinFunction.~FunctionPtr();
                break;
            case Type::Null:
                // Null doesn't require special handling.
                break;
        }
        // Set type to Null after cleanup
        type = Type::Null;
    }
}




void Value::copyFrom(const Value& other) {
    // Clean up existing content
    cleanUp();

    type = other.type;
    switch (type) {
        case Type::Double:
            doubleValue = other.doubleValue;
            break;
        case Type::Bool:
            boolValue = other.boolValue;
            break;
        case Type::Function:
            new (&functionValue) Function(other.functionValue); // Placement new
            break;
        case Type::Array:
            new (&arrayValue) std::shared_ptr<std::vector<Value>>(other.arrayValue); // Placement new for shared_ptr
            break;
        case Type::Null:
            break;
        case Type::BuiltinFunction:
            new (&builtinFunction) FunctionPtr(other.builtinFunction); // Placement new
            break;
    }
}


Value Value::deepCopy() const {
    switch (type) {
        case Type::Double:
            return Value(doubleValue);
        case Type::Bool:
            return Value(boolValue);
        case Type::Array: {
            auto copiedArray = std::make_shared<std::vector<Value>>();
            for (const auto& element : *arrayValue) {
                copiedArray->push_back(element.deepCopy());
            }
            return Value(*copiedArray);
        }
        case Type::Null:
            return Value();
        // Add cases for other types as necessary

        default:
            // Handle unexpected type, perhaps by throwing an exception
            throw std::runtime_error("Unknown or unsupported type for deepCopy");
    }
}

void Value::moveFrom(Value&& other) {
    // Clean up existing content
    cleanUp();

    type = other.type;
    switch (type) {
        case Type::Double:
            doubleValue = other.doubleValue;
            break;
        case Type::Bool:
            boolValue = other.boolValue;
            break;
        case Type::Function:
            new (&functionValue) Function(std::move(other.functionValue)); // Placement new for move
            break;
        case Type::Array:
            new (&arrayValue) std::shared_ptr<std::vector<Value>>(std::move(other.arrayValue)); // Placement new for move
            break;
        case Type::Null:
            break;
        case Type::BuiltinFunction:
            new (&builtinFunction) FunctionPtr(std::move(other.builtinFunction)); // Placement new for move
            break;
    }
    other.type = Type::Null; // Set the moved-from object to null state
}



Value::Type Value::getType() const {
    return type;
}

double Value::asDouble() const {
    if (type != Type::Double) {
        throw std::runtime_error("Runtime error: invalid operand type.");
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
        throw std::runtime_error("Runtime error: not a function.");
    }
    return functionValue;
}

bool Value::equals(const Value& other) const {
    // First, check if the types of the two values are the same
    if (this->type != other.type) return false;

    // Compare based on type
    switch (this->type) {
        case Type::Null:
            return true; // If both are Null, they are equal
        case Type::Bool:
            return this->boolValue == other.boolValue;
        case Type::Double:
            return this->doubleValue == other.doubleValue;
        case Type::Array: {
            const auto& thisArray = this->asArray();
            const auto& otherArray = other.asArray();
            if (thisArray.size() != otherArray.size()) {
                return false;
            }
            for (size_t i = 0; i < thisArray.size(); ++i) {
                if (!thisArray[i].equals(otherArray[i])) {
                    return false;
                }
            }
            return true;
        }
        case Type::BuiltinFunction:
            return this->builtinFunction.target<Value::FunctionPtr>() == other.builtinFunction.target<Value::FunctionPtr>();
        // Add cases for other types your Value class supports
        default:
            throw std::runtime_error("Unsupported type in Value::equals");
    }
}


bool Value::isNull() const {
    return type == Type::Null;
}

bool Value::isArray() const {
    return type == Type::Array;
}

bool Value::isInteger() const {
    if (type != Type::Double) {
        return false;
    }
    double intPart;
    return std::modf(doubleValue, &intPart) == 0.0;
}


std::vector<Value>& Value::asArray() {
    if (type != Type::Array) {
        throw std::runtime_error("Runtime error: not an array.");
    }
    return *arrayValue;
}

const std::vector<Value>& Value::asArray() const {
    if (type != Type::Array) {
        throw std::runtime_error("Runtime error: not an array.");
    }
    return *arrayValue;
}

void Scope::setVariable(const std::string& name, const Value& value) {
    if (parentScope && parentScope->hasVariable(name)) {
        parentScope->setVariable(name, value);
    } else {
        variables[name] = value;
    }
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

bool Scope::hasVariable(const std::string& name) {
    if (variables.count(name) > 0) {
        return true;
    } else if (parentScope) {
        return parentScope->hasVariable(name);
    }
    return false;
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

std::shared_ptr<Scope> Scope::deepCopy() const {
    auto copiedScope = std::make_shared<Scope>(nullptr);

    // Copy variables
    for (const auto& variable : this->variables) {
        const std::string& name = variable.first;
        const Value& value = variable.second;
        copiedScope->variables[name] = value;
    }

    // Recursively copy parent scope if it exists
    if (this->parentScope) {
        copiedScope->parentScope = this->parentScope->deepCopy();
    }

    return copiedScope;
}

// ReturnException implementations
ReturnException::ReturnException(Value returnValue) : returnValue(std::move(returnValue)) {}

const Value& ReturnException::getValue() const {
    return returnValue;
}