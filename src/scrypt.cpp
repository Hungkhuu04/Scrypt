#include "lib/mParser.h"
#include "lib/lex.h"
#include "lib/ASTNodes.h" 
#include <iostream>
#include <fstream>
#include <memory>
#include <stdexcept>
#include <unordered_map>
#include <vector>
#include <string>
#include <cmath>
#include "lib/ScryptComponents.h"

std::shared_ptr<Scope> globalScope = std::make_shared<Scope>();

Value tokenToValue(const Token& token);
Value evaluateExpression(const ASTNode* node, std::shared_ptr<Scope> currentScope);
void evaluateBlock(const BlockNode* blockNode, std::shared_ptr<Scope> currentScope);
void evaluateIf(const IfNode* ifNode, std::shared_ptr<Scope> currentScope);
void evaluateWhile(const WhileNode* whileNode, std::shared_ptr<Scope> currentScope);
void printValue(const Value& value);
void evaluatePrint(const PrintNode* printNode, std::shared_ptr<Scope> currentScope);
Value evaluateBinaryOperation(const BinaryOpNode* binaryOpNode, std::shared_ptr<Scope> currentScope);
Value evaluateVariable(const VariableNode* variableNode, std::shared_ptr<Scope> currentScope);
Value evaluateAssignment(const AssignmentNode* assignmentNode, std::shared_ptr<Scope> currentScope);
Value evaluateFunctionCall(const CallNode* node, std::shared_ptr<Scope> currentScope);
void evaluateFunctionDefinition(const FunctionNode* functionNode, std::shared_ptr<Scope> currentScope);
void evaluateReturn(const ReturnNode* returnNode, std::shared_ptr<Scope> currentScope);
void evaluateStatement(const ASTNode* stmt, std::shared_ptr<Scope> currentScope);
Value evaluateArrayLiteralNode(const ArrayLiteralNode* arrayLiteralNode, std::shared_ptr<Scope> currentScope);
Value evaluateArrayLookupNode(const ArrayLookupNode* arrayLookupNode, std::shared_ptr<Scope> currentScope);

Value lenFunction(const std::vector<Value>& args);
Value popFunction(std::vector<Value>& args);
Value pushFunction(std::vector<Value>& args);

// Checks for Boolean True and False
Value tokenToValue(const Token& token) {
    switch (token.type) {
        case TokenType::NUMBER:
            return Value(std::stod(token.value));
        case TokenType::BOOLEAN_TRUE:
            return Value(true);
        case TokenType::BOOLEAN_FALSE:
            return Value(false);
        default:
            throw std::runtime_error("Invalid token type for value conversion");
    }
}


// Evaluate the block node
void evaluateBlock(const BlockNode* blockNode, std::shared_ptr<Scope> currentScope) {
    if (!blockNode) {
        throw std::runtime_error("Null block node passed to evaluateBlock");
    }

    try {
        for (const auto& stmt : blockNode->statements) {
            evaluateStatement(stmt.get(), currentScope);
        }
    } catch (...) {
        throw;
    }
}


// Evaluate function calls
Value evaluateFunctionCall(const CallNode* node, std::shared_ptr<Scope> currentScope) {
    std::string functionName = static_cast<const VariableNode*>(node->callee.get())->identifier.value;
    std::vector<Value> args;
    for (const auto& arg : node->arguments) {
        args.push_back(evaluateExpression(arg.get(), currentScope));
    }
    if (functionName == "push") {
        return pushFunction(args);
    } else if (functionName == "pop") {
        return popFunction(args);
    } else if (functionName == "len") {
        return lenFunction(args);
    } else {
        auto funcValue = evaluateExpression(node->callee.get(), currentScope);
        if (funcValue.getType() != Value::Type::Function) {
            throw std::runtime_error("Runtime error: not a function.");
        }

        const auto& function = funcValue.asFunction();
        auto callScope = function.capturedScope;

        const auto& params = function.definition->parameters;
        if (params.size() != args.size()) {
            throw std::runtime_error("Runtime error: incorrect argument count.");
        }

        for (size_t i = 0; i < params.size(); ++i) {
            callScope->setVariable(params[i].value, args[i]);
        }

        try {
            evaluateBlock(static_cast<const BlockNode*>(function.definition->body.get()), callScope);
        } catch (...) {
            throw;
        }

        return Value();
    }
}



// Evaluate Function Definitions
void evaluateFunctionDefinition(const FunctionNode* functionNode, std::shared_ptr<Scope> currentScope) {
    if (!functionNode) {
        throw std::runtime_error("Null function node passed to evaluateFunctionDefinition");
    }

    try {
        std::shared_ptr<Scope> capturedScope = currentScope->copyScope();
        Value::Function functionValue;
        functionValue.definition = std::make_unique<FunctionNode>(*functionNode);
        functionValue.capturedScope = capturedScope;
        Value value(std::move(functionValue));
        currentScope->setVariable(functionNode->name.value, std::move(value));
    } catch (...) {
        throw;
    }
}


// Evaluate Statements
void evaluateStatement(const ASTNode* stmt, std::shared_ptr<Scope> currentScope) {
    switch (stmt->getType()) {
        case ASTNode::Type::IfNode:
            evaluateIf(static_cast<const IfNode*>(stmt), currentScope);
            break;
        case ASTNode::Type::WhileNode:
            evaluateWhile(static_cast<const WhileNode*>(stmt), currentScope);
            break;
        case ASTNode::Type::PrintNode:
            evaluatePrint(static_cast<const PrintNode*>(stmt), currentScope);
            break;
        case ASTNode::Type::AssignmentNode:
            evaluateAssignment(static_cast<const AssignmentNode*>(stmt), currentScope);
            break;
        case ASTNode::Type::BlockNode:
            evaluateBlock(static_cast<const BlockNode*>(stmt), currentScope);
            break;
        case ASTNode::Type::FunctionNode:
            evaluateFunctionDefinition(static_cast<const FunctionNode*>(stmt), currentScope);
            break;
        case ASTNode::Type::ReturnNode:
            evaluateReturn(static_cast<const ReturnNode*>(stmt), currentScope);
            break;
        case ASTNode::Type::CallNode:
            evaluateFunctionCall(static_cast<const CallNode*>(stmt), currentScope);
            break;
        default:
            throw std::runtime_error("Unknown Node Type in evaluateStatement");
    }
}

// Evaluate Expressions
Value evaluateExpression(const ASTNode* node, std::shared_ptr<Scope> currentScope) {
    if (!node) {
        throw std::runtime_error("Null expression node");
    }
    try {
        switch (node->getType()) {
            case ASTNode::Type::NumberNode: {
                auto numberNode = static_cast<const NumberNode*>(node);
                return Value(std::stod(numberNode->value.value));
            }
            case ASTNode::Type::BooleanNode: {
                auto booleanNode = static_cast<const BooleanNode*>(node);
                return Value(booleanNode->value.type == TokenType::BOOLEAN_TRUE);
            }
            case ASTNode::Type::VariableNode: {
                auto variableNode = static_cast<const VariableNode*>(node);
                return evaluateVariable(variableNode, currentScope);
            }
            case ASTNode::Type::BinaryOpNode: {
                auto binaryOpNode = static_cast<const BinaryOpNode*>(node);
                return evaluateBinaryOperation(binaryOpNode, currentScope);
            }
            case ASTNode::Type::AssignmentNode: {
                auto assignmentNode = static_cast<const AssignmentNode*>(node);
                return evaluateAssignment(assignmentNode, currentScope);
            }
            case ASTNode::Type::CallNode: {
                auto callNode = static_cast<const CallNode*>(node);
                return evaluateFunctionCall(callNode, currentScope);
            }
            case ASTNode::Type::ArrayLiteralNode:
                return evaluateArrayLiteralNode(static_cast<const ArrayLiteralNode*>(node), currentScope);
            case ASTNode::Type::ArrayLookupNode:
                return evaluateArrayLookupNode(static_cast<const ArrayLookupNode*>(node), currentScope);
            case ASTNode::Type::NullNode:
                return Value();
            default:
                throw std::runtime_error("Unknown expression node type");
        }
    } catch (...) {
        throw;
    }
}


// Evaluate the if node
void evaluateIf(const IfNode* ifNode, std::shared_ptr<Scope> currentScope) {
    try {
        Value conditionValue = evaluateExpression(ifNode->condition.get(), currentScope);
        if(!conditionValue.asBool()) {
            throw std::runtime_error("Runtime error: condition is not a bool.");
        } 
        else if (conditionValue.asBool()) {
            evaluateBlock(static_cast<const BlockNode*>(ifNode->trueBranch.get()), currentScope);
        } else if (ifNode->falseBranch) {
            evaluateStatement(ifNode->falseBranch.get(), currentScope);
        }
    } catch (...) {
        throw;
    }
}

// Evaluate the while node
void evaluateWhile(const WhileNode* whileNode, std::shared_ptr<Scope> currentScope) {
    try {
        while (true) {
            Value conditionValue = evaluateExpression(whileNode->condition.get(), currentScope);
            if (!conditionValue.asBool()) {
                break;
            }
            auto loopScope = std::make_shared<Scope>(currentScope);
            evaluateBlock(static_cast<const BlockNode*>(whileNode->body.get()), loopScope);
            for (const auto& var : loopScope->getVariables()) {
                if (currentScope->hasVariable(var.first)) {
                    currentScope->setVariable(var.first, var.second);
                }
            }
        }
    } catch (...) {
        throw;
    }
}


// Evaluate Return (functions)
void evaluateReturn(const ReturnNode* returnNode, std::shared_ptr<Scope> currentScope) {
    if (!returnNode) {
        throw std::runtime_error("Null return node passed to evaluateReturn");
    }

    Value returnValue;
    if (returnNode->value) {
        returnValue = evaluateExpression(returnNode->value.get(), currentScope);
    } else {
        throw std::runtime_error("Runtime error: unexpected return.");
    }
    throw std::runtime_error("Runtime error: unexpected return.");
}

//helper function with print
void printValue(const Value& value) {
    switch (value.getType()) {
        case Value::Type::Double:
            std::cout << value.asDouble();
            break;

        case Value::Type::Bool:
            std::cout << std::boolalpha << value.asBool();
            break;

        case Value::Type::Null:
            std::cout << "null";
            break;

        case Value::Type::Array: {
            std::cout << "[";
            const auto& array = value.asArray();
            for (size_t i = 0; i < array.size(); ++i) {
                if (i > 0) std::cout << ", ";
                printValue(array[i]);
            }
            std::cout << "]";
            break;
        }

        default:
            std::cout << "/* Unsupported type */";
            break;
    }
}

// Evaluate the print node
void evaluatePrint(const PrintNode* printNode, std::shared_ptr<Scope> currentScope) {
    Value value = evaluateExpression(printNode->expression.get(), currentScope);
    printValue(value);
    std::cout << std::endl;
}

// Evaluate Operations
Value evaluateBinaryOperation(const BinaryOpNode* binaryOpNode, std::shared_ptr<Scope> currentScope) {
    if (!binaryOpNode) {
        throw std::runtime_error("Null BinaryOpNode passed to evaluateBinaryOperation");
    }

    Value left = evaluateExpression(binaryOpNode->left.get(), currentScope);
    Value right = evaluateExpression(binaryOpNode->right.get(), currentScope);

    switch (binaryOpNode->op.type) {
        case TokenType::ADD:
            return Value(left.asDouble() + right.asDouble());
        case TokenType::SUBTRACT:
            return Value(left.asDouble() - right.asDouble());
        case TokenType::MULTIPLY:
            return Value(left.asDouble() * right.asDouble());
        case TokenType::DIVIDE:
            if (right.asDouble() == 0) {
                throw std::runtime_error("Division by zero.");
            }
            return Value(left.asDouble() / right.asDouble());
        case TokenType::MODULO:
            if (right.asDouble() == 0) {
                throw std::runtime_error("Modulo by zero.");
            }
            return Value(fmod(left.asDouble(), right.asDouble()));
        case TokenType::LESS:
            return Value(left.asDouble() < right.asDouble());
        case TokenType::LESS_EQUAL:
            return Value(left.asDouble() <= right.asDouble());
        case TokenType::GREATER:
            return Value(left.asDouble() > right.asDouble());
        case TokenType::GREATER_EQUAL:
            return Value(left.asDouble() >= right.asDouble());
        case TokenType::EQUAL:
            return Value(left.equals(right));
        case TokenType::NOT_EQUAL:
            return Value(!left.equals(right));
        case TokenType::LOGICAL_AND:
            return Value(left.asBool() && right.asBool());
        case TokenType::LOGICAL_OR:
            return Value(left.asBool() || right.asBool());
        case TokenType::LOGICAL_XOR: 
            return Value(left.asBool() != right.asBool());
        case TokenType::ASSIGN:
            if (binaryOpNode->left->getType() == ASTNode::Type::VariableNode) {
                const auto* variableNode = static_cast<const VariableNode*>(binaryOpNode->left.get());
                currentScope->setVariable(variableNode->identifier.value, right);
                return right;
            } else {
                throw std::runtime_error("Invalid left-hand side in assignment");
            }
        default:
            throw std::runtime_error("Unsupported binary operator in evaluateBinaryOperation");
    }
}
// Evaluate variables
Value evaluateVariable(const VariableNode* variableNode, std::shared_ptr<Scope> currentScope) {
    if (!variableNode) {
        throw std::runtime_error("Null VariableNode passed to evaluateVariable");
    }

    Value* valuePtr = currentScope->getVariable(variableNode->identifier.value);
    if (valuePtr) {
        return *valuePtr;
    } else {
        throw std::runtime_error("Runtime error: unknown identifier " + variableNode->identifier.value);
    }
}


// Evaluate Assignments
Value evaluateAssignment(const AssignmentNode* assignmentNode, std::shared_ptr<Scope> currentScope) {
    if (!assignmentNode) {
        throw std::runtime_error("Null assignment node passed to evaluateAssignment");
    }

    Value rhsValue = evaluateExpression(assignmentNode->rhs.get(), currentScope);

    if (assignmentNode->lhs->getType() == ASTNode::Type::ArrayLookupNode &&
        assignmentNode->rhs->getType() == ASTNode::Type::ArrayLiteralNode) {
        return rhsValue;
    }
    if (assignmentNode->lhs->getType() == ASTNode::Type::VariableNode) {
        auto variableNode = static_cast<const VariableNode*>(assignmentNode->lhs.get());
        currentScope->setVariable(variableNode->identifier.value, rhsValue);
    } else if (assignmentNode->lhs->getType() == ASTNode::Type::ArrayLookupNode) {
        auto arrayLookupNode = static_cast<const ArrayLookupNode*>(assignmentNode->lhs.get());

        if (arrayLookupNode->array->getType() != ASTNode::Type::VariableNode) {
            throw std::runtime_error("Runtime error: not an array.");
        }
        auto variableNode = static_cast<const VariableNode*>(arrayLookupNode->array.get());
        std::string arrayName = variableNode->identifier.value;
        Value* arrayValuePtr = currentScope->getVariable(arrayName);

        if (!arrayValuePtr || arrayValuePtr->getType() != Value::Type::Array) {
            throw std::runtime_error("Runtime error: not an array.");
        }
        std::vector<Value>& array = arrayValuePtr->asArray();

        Value indexValue = evaluateExpression(arrayLookupNode->index.get(), currentScope);
        if (indexValue.getType() != Value::Type::Double) {
        throw std::runtime_error("Runtime error: index is not a number.");
        }

        double intPart;
        if (modf(indexValue.asDouble(), &intPart) != 0.0) {
            throw std::runtime_error("Runtime error: index is not an integer.");
        }

        int index = static_cast<int>(intPart);
        if (index < 0 || index >= static_cast<int>(array.size())) {
            throw std::runtime_error("Runtime error: index out of bounds.");
        }

        Value rhsValue = evaluateExpression(assignmentNode->rhs.get(), currentScope);

        array[index] = rhsValue;

        return rhsValue;
    }
    else {
        throw std::runtime_error("Runtime error: invalid assignee.");
    }

    return rhsValue;
}

// Evaluate Array Literals
Value evaluateArrayLiteralNode(const ArrayLiteralNode* arrayLiteralNode, std::shared_ptr<Scope> currentScope) {
    if (!arrayLiteralNode) {
        throw std::runtime_error("Null ArrayLiteralNode passed to evaluateArrayLiteralNode");
    }

    std::vector<Value> arrayValues;
    for (const auto& element : arrayLiteralNode->elements) {
        Value copiedElement = evaluateExpression(element.get(), currentScope).deepCopy();
        arrayValues.push_back(copiedElement);
    }
    return Value(arrayValues);
}

// Evaluate and return the Array Literals
Value evaluateArrayLookupNode(const ArrayLookupNode* arrayLookupNode, std::shared_ptr<Scope> currentScope) {
    if (!arrayLookupNode) {
        throw std::runtime_error("Null ArrayLookupNode passed to evaluateArrayLookupNode");
    }

    Value arrayValue = evaluateExpression(arrayLookupNode->array.get(), currentScope);
    Value indexValue = evaluateExpression(arrayLookupNode->index.get(), currentScope);

    if (indexValue.getType() != Value::Type::Double) {
        throw std::runtime_error("Runtime error: index is not a number.");
    }

    double intPart;
    if (modf(indexValue.asDouble(), &intPart) != 0.0) {
        throw std::runtime_error("Runtime error: index is not an integer.");
    }

    int index = static_cast<int>(intPart);
    if (index < 0 || index >= static_cast<int>(arrayValue.asArray().size())) {
        throw std::runtime_error("Runtime error: index out of bounds.");
    }
    return arrayValue.asArray()[index];
}

// Len Function of Arrays
Value lenFunction(const std::vector<Value>& args) {
    if (args.size() != 1 || !args[0].isArray()) {
        throw std::runtime_error("Runtime error: incorrect argument count.");
    }
    return Value(static_cast<double>(args[0].asArray().size()));
}

// Pop function of arrays
Value popFunction(std::vector<Value>& args) {
    if (args.size() != 1 || !args[0].isArray()) {
        throw std::runtime_error("Runtime error: incorrect argument count.");
    }
    auto& array = args[0].asArray();
    if (array.empty()) {
        throw std::runtime_error("pop from an empty array.");
    }
    Value poppedValue = std::move(array.back());
    array.pop_back();
    return poppedValue;
}

// push function of arrays
Value pushFunction(std::vector<Value>& args) {
    if (args.size() != 2 || !args[0].isArray()) {
        throw std::runtime_error("Runtime error: incorrect argument count.");
    }
    args[0].asArray().push_back(args[1]);
    return Value();
}



int main() {
    std::ostream& os = std::cout;
    std::string line;
    std::string inputCode;
    globalScope->setVariable("len", Value(Value::FunctionPtr(lenFunction)));
    globalScope->setVariable("pop", Value(Value::FunctionPtr(popFunction)));
    globalScope->setVariable("push", Value(Value::FunctionPtr(pushFunction)));
    while (std::getline(std::cin, line)) {
        inputCode += line + "\n";
    }

    try {
        Lexer lexer(inputCode);
        auto tokens = lexer.tokenize();
        if (lexer.isSyntaxError(tokens)) {
            exit(1);
        }

        Parser parser(tokens);
        auto ast = parser.parse();

        if (ast->getType() == ASTNode::Type::BlockNode) {
            evaluateBlock(static_cast<const BlockNode*>(ast.get()), globalScope);
        } else {
            throw std::runtime_error("Invalid AST node type");
        }
    } catch (const std::runtime_error& e) {
        os << e.what() << std::endl;
        if (std::string(e.what()) == "Runtime error: condition is not a bool.") {
            exit(3);
        } else if (std::string(e.what()) == "Runtime error: incorrect argument count.") {
            exit(3);
        } else if (std::string(e.what()) == "Runtime error: not a function.") {
            exit(3);
        } else {
            exit(2);
        }
    } catch (...){
        os << "Runtime error: unexpected return." << std::endl;
        exit(3);
    }
    return 0;
}