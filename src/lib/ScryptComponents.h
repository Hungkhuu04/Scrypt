#ifndef SCRIPT_COMPONENTS_H
#define SCRIPT_COMPONENTS_H

#include <string>
#include <unordered_map>
#include <exception>
#include <memory>
#include <vector>
#include "ASTNodes.h"
#include <functional>

class Scope;

// Value class to represent different types of values in your script
class Value {
public:
    using FunctionPtr = std::function<Value(std::vector<Value>&)>;
    enum class Type { Double, Bool, Function, Null, Array, BuiltinFunction};

    struct Function {
        std::shared_ptr<FunctionNode> definition; 
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
    Value(std::vector<Value> array);
    Value(FunctionPtr func);

    ~Value();

    Type getType() const;
    double asDouble() const;
    bool asBool() const;
    const Function& asFunction() const;
    bool isNull() const;
    bool equals(const Value& other) const;

    bool isArray() const;
    bool isInteger() const;
    std::vector<Value>& asArray();
    const std::vector<Value>& asArray() const;
    Value deepCopy() const;
    Value executeFunction(std::vector<Value>& args) const;
private:
    Type type;
    union {
        double doubleValue;
        bool boolValue;
        Function functionValue;
        std::shared_ptr<std::vector<Value>> arrayValue;
        FunctionPtr builtinFunction; 
    };

    void cleanUp();
    void copyFrom(const Value& other);
    void moveFrom(Value&& other);
};
// Scope class for variable scoping
class Scope {
public:
    Scope(std::shared_ptr<Scope> parent = nullptr) : parentScope(parent) {}

    void setVariable(const std::string& name, const Value& value);
    Value* getVariable(const std::string& name);
    const std::unordered_map<std::string, Value>& getVariables() const;

    std::shared_ptr<Scope> getParent() const;
    std::shared_ptr<Scope> copyScope() const;
    std::shared_ptr<Scope> deepCopy() const;
    bool hasVariable(const std::string& name);

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