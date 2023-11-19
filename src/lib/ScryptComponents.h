#ifndef SCRIPT_COMPONENTS_H
#define SCRIPT_COMPONENTS_H

#include <string>
#include <unordered_map>
#include <exception>
#include <memory>
#include <vector>
#include "ASTNodes.h"

// Forward declaration for Scope class
class Scope;

// Value class to represent different types of values in your script
class Value {
public:
    enum class Type { Double, Bool, Function, Null };

    struct Function {
        std::shared_ptr<FunctionNode> definition; // Changed to shared_ptr
        std::shared_ptr<Scope> capturedScope;

        Function() = default;
        Function(const std::shared_ptr<FunctionNode>& def, const std::shared_ptr<Scope>& scope)
            : definition(def), capturedScope(scope) {}
        Function(const Function& other) 
            : definition(other.definition), capturedScope(other.capturedScope) {}
        Function(Function&& other) noexcept 
            : definition(std::move(other.definition)), capturedScope(std::move(other.capturedScope)) {}

        Function& operator=(const Function& other) {
            if (this != &other) {
                definition = other.definition;
                capturedScope = other.capturedScope;
            }
            return *this;
        }

        Function& operator=(Function&& other) noexcept {
            definition = std::move(other.definition);
            capturedScope = std::move(other.capturedScope);
            return *this;
        }
    };

    Value();
    Value(double value);
    Value(bool value);
    Value(Function function);
    Value(const Value& other);
    Value(Value&& other) noexcept;
    Value& operator=(const Value& other);
    Value& operator=(Value&& other) noexcept;
    ~Value();

    Type getType() const;
    double asDouble() const;
    bool asBool() const;
    const Function& asFunction() const;
    bool isNull() const;

private:
    Type type;
    union {
        double doubleValue;
        bool boolValue;
        Function functionValue; // Raw storage, will be constructed/destructed manually
    };

    void cleanUp();
    void copyFrom(const Value& other);
    void moveFrom(Value&& other);
};
// Scope class for variable scoping
class Scope {
public:
    explicit Scope(std::shared_ptr<Scope> parent = nullptr);

    void setVariable(const std::string& name, const Value& value);
    Value* getVariable(const std::string& name);
    const std::unordered_map<std::string, Value>& getVariables() const;

    std::shared_ptr<Scope> getParent() const;
    std::shared_ptr<Scope> copyScope() const;

private:
    std::unordered_map<std::string, Value> variables;
    std::shared_ptr<Scope> parentScope;
};

// Exception class for handling return statements
class ReturnException : public std::exception {
    Value returnValue;
public:
    explicit ReturnException(Value returnValue);
    const Value& getValue() const;
};

#endif // SCRIPT_COMPONENTS_H