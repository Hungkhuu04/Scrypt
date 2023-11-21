

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
void evaluatePrint(const PrintNode* printNode, std::shared_ptr<Scope> currentScope);
Value evaluateBinaryOperation(const BinaryOpNode* binaryOpNode, std::shared_ptr<Scope> currentScope);
Value* evaluateVariable(const ASTNode* node, std::shared_ptr<Scope> currentScope);
Value evaluateAssignment(const AssignmentNode* assignmentNode, std::shared_ptr<Scope> currentScope);
Value evaluateFunctionCall(const CallNode* node, std::shared_ptr<Scope> currentScope);
void evaluateFunctionDefinition(const FunctionNode* functionNode, std::shared_ptr<Scope> currentScope);
void evaluateReturn(const ReturnNode* returnNode, std::shared_ptr<Scope> currentScope);
void evaluateStatement(const ASTNode* stmt, std::shared_ptr<Scope> currentScope);
Value evaluateArrayLiteral(const ArrayLiteralNode* arrayNode, std::shared_ptr<Scope> currentScope);
Value evaluateArrayLookup(const ArrayLookupNode* lookupNode, std::shared_ptr<Scope> currentScope);


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

Value evaluateFunctionCall(const CallNode* node, std::shared_ptr<Scope> currentScope) {
    try {
        auto funcValue = evaluateExpression(node->callee.get(), currentScope);
        if (funcValue.getType() != Value::Type::Function) {
            throw std::runtime_error("Runtime error: not a function.");
        }

        const auto& function = funcValue.asFunction();

        // Create a new scope for the function call with the function's captured scope as the parent
        auto callScope = std::make_shared<Scope>(function.capturedScope);

        const auto& params = function.definition->parameters;
        const auto& args = node->arguments;
        if (params.size() != args.size()) {
            throw std::runtime_error("Runtime error: incorrect argument count.");
        }

        for (size_t i = 0; i < params.size(); ++i) {
            auto argValue = evaluateExpression(args[i].get(), currentScope);
            callScope->setVariable(params[i].value, argValue);
        }

        try {
            evaluateBlock(static_cast<const BlockNode*>(function.definition->body.get()), callScope);
        } catch (const ReturnException& e) {
            return e.getValue();
        }

        return Value(); // Return null if no return statement was executed
    } catch (...) {
        throw;
    }
}


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
            case ASTNode::Type::NullNode: {
                return Value();
            }
            case ASTNode::Type::ArrayLiteralNode: {
                return evaluateArrayLiteral(static_cast<const ArrayLiteralNode*>(node), currentScope);
            }
            case ASTNode::Type::ArrayLookupNode: {
                return evaluateArrayLookup(static_cast<const ArrayLookupNode*>(node), currentScope);
            }
            default:
                throw std::runtime_error("Unknown expression node type");
        }
    } catch (...) {
        throw;
    }
}


// Evaluate the if node
void evaluateIf(const IfNode* ifNode, std::shared_ptr<Scope> currentScope) {
    Value conditionValue = evaluateExpression(ifNode->condition.get(), currentScope);
    if (conditionValue.asBool()) {
        evaluateBlock(static_cast<const BlockNode*>(ifNode->trueBranch.get()), currentScope);
    } else if (ifNode->falseBranch) {
        evaluateStatement(ifNode->falseBranch.get(), currentScope);
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

            // Propagate changes back to the current scope
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



void evaluateReturn(const ReturnNode* returnNode, std::shared_ptr<Scope> currentScope) {
    if (!returnNode) {
        throw std::runtime_error("Null return node passed to evaluateReturn");
    }

    // Evaluate the expression of the return statement, if present
    Value returnValue;
    if (returnNode->value) {
        returnValue = evaluateExpression(returnNode->value.get(), currentScope);
    } else {
        throw ReturnException(Value());  // Returning null if no expression is present
    }

    // Throw a ReturnException to signal a return from the function
    throw ReturnException(std::move(returnValue));
}
void printArray(const std::vector<Value>& array) {
    
    std::cout << '[';
    bool firstElement = true;
    for (const auto& elem : array) {
        if (!firstElement) {
            std::cout << ", ";
        }
        firstElement = false;

        switch (elem.getType()) {
            case Value::Type::Double:
                std::cout << elem.asDouble();
                break;
            case Value::Type::Bool:
                std::cout << std::boolalpha << elem.asBool();
                break;
            case Value::Type::Null:
                std::cout << "null";
                break;
            case Value::Type::Array:
                printArray(elem.asArray());
                break;
            // Handle other types if necessary
            default:
                throw std::runtime_error("Unsupported type in array print");
        }
    }
    std::cout << ']';
}


// Evaluate the print node
void evaluatePrint(const PrintNode* printNode, std::shared_ptr<Scope> currentScope) {
    Value value = evaluateExpression(printNode->expression.get(), currentScope);
    switch (value.getType()) {
        case Value::Type::Double:
            std::cout << value.asDouble() << std::endl;
            break;
        case Value::Type::Bool:
            std::cout << std::boolalpha << value.asBool() << std::endl;
            break;
        case Value::Type::Null:
            std::cout << "null" << std::endl;
            break;
        case Value::Type::Array:
            printArray(value.asArray());
            std::cout << std::endl;
            break;
        default:
            throw std::runtime_error("Unsupported type in print statement");
    }
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
Value* evaluateVariable(const ASTNode* node, std::shared_ptr<Scope> currentScope) {
    if (!node) {
        throw std::runtime_error("Null VariableNode passed to evaluateVariable");
    }

    // Ensure the node is a VariableNode
    if (node->getType() != ASTNode::Type::VariableNode) {
        throw std::runtime_error("Expected VariableNode in evaluateVariable");
    }

    auto variableNode = static_cast<const VariableNode*>(node);

    // Retrieve the variable from the current scope
    Value* valuePtr = currentScope->getVariable(variableNode->identifier.value);
    if (!valuePtr) {
        throw std::runtime_error("Variable not defined: " + variableNode->identifier.value);
    }

    return valuePtr;
}


// Evaluate Assignments
Value evaluateAssignment(const AssignmentNode* assignmentNode, std::shared_ptr<Scope> currentScope) {
    if (!assignmentNode) {
        throw std::runtime_error("Null assignment node passed to evaluateAssignment");
    }

    // Evaluate the right-hand side (rhs) expression
    Value rhsValue = evaluateExpression(assignmentNode->rhs.get(), currentScope);

    if (assignmentNode->lhs->getType() == ASTNode::Type::VariableNode) {
        // Handle variable assignment
        auto variableNode = static_cast<const VariableNode*>(assignmentNode->lhs.get());
        currentScope->setVariable(variableNode->identifier.value, rhsValue);
    } else if (assignmentNode->lhs->getType() == ASTNode::Type::ArrayLookupNode) {
        // Handle array element assignment
        auto arrayLookupNode = static_cast<const ArrayLookupNode*>(assignmentNode->lhs.get());

        // Ensure the array is actually a variable
        if (arrayLookupNode->array->getType() != ASTNode::Type::VariableNode) {
            throw std::runtime_error("Array assignment requires a variable");
        }

        // Evaluate the array variable and index
        auto variableNode = static_cast<const VariableNode*>(arrayLookupNode->array.get());
        Value* arrayValuePtr = evaluateVariable(variableNode, currentScope);

        if (!arrayValuePtr || !arrayValuePtr->isArray()) {
            throw std::runtime_error("Invalid array variable");
        }

        Value indexValue = evaluateExpression(arrayLookupNode->index.get(), currentScope);
        if (!indexValue.isInteger()) {
            throw std::runtime_error("Array index is not an integer");
        }

        int index = static_cast<int>(indexValue.asDouble());
        if (index < 0 || index >= static_cast<int>(arrayValuePtr->asArray().size())) {
            throw std::runtime_error("Array index out of bounds");
        }


        arrayValuePtr->asArray()[index] = rhsValue;
    } else {
        throw std::runtime_error("Invalid left-hand side in assignment");
    }

    return rhsValue;
}



Value evaluateArrayLiteral(const ArrayLiteralNode* arrayNode, std::shared_ptr<Scope> currentScope) {
    std::vector<Value> elements;
    for (const auto& element : arrayNode->elements) {
        elements.push_back(evaluateExpression(element.get(), currentScope));
    }
    return Value(std::move(elements));
}

Value evaluateArrayLookup(const ArrayLookupNode* lookupNode, std::shared_ptr<Scope> currentScope) {
    auto arrayValue = evaluateExpression(lookupNode->array.get(), currentScope);
    auto indexValue = evaluateExpression(lookupNode->index.get(), currentScope);

    if (!indexValue.isInteger()) {
        throw std::runtime_error("Runtime error: index is not an integer.");
    }
    auto& array = arrayValue.asArray();
    int index = static_cast<int>(indexValue.asDouble());
    if (index < 0 || index >= static_cast<int>(array.size())) {
        throw std::runtime_error("Runtime error: index out of bounds.");
    }
    return array[index];
}

int main() {
    std::ostream& os = std::cout;
    std::string line;
    std::string inputCode;
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
            // Start evaluation with the global scope
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
        os << "Unknown error" << std::endl;
        exit(2);
    }
    return 0;
}