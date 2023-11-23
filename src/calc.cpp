
#include "lib/mParser.h"
#include "lib/ASTNodes.h" 
#include "lib/lex.h"
#include <iostream>
#include <string>
#include <unordered_map>
#include "lib/ScryptComponents.h"
#include <iomanip>
#include <cmath>

using namespace std;

std::string indentString(int indentLevel);
void formatAST(std::ostream& os, const std::unique_ptr<ASTNode>& node, int indent, bool isOutermost = true);
void formatBinaryOpNode(std::ostream& os, const BinaryOpNode* node, int indent);
void formatNumberNode(std::ostream& os, const NumberNode* node, int indent);
void formatBooleanNode(std::ostream& os, const BooleanNode* node, int indent);
void formatVariableNode(std::ostream& os, const VariableNode* node, int indent);
void formatAssignmentNode(std::ostream& os, const AssignmentNode* node, int indent);
void formatBlockNode(std::ostream& os, const BlockNode* node, int indent);
void formatNullNode(std::ostream& os, const NullNode* node, int indent);
void formatCallNode(std::ostream& os, const CallNode* node, int indent, bool isOutermost);
void formatArrayLiteralNode(std::ostream& os, const ArrayLiteralNode* node, int indent, bool isOutermost);
void formatArrayLookupNode(std::ostream& os, const ArrayLookupNode* node, int indent, bool isOutermost);

Value evaluateVariable(const VariableNode* variableNode, std::shared_ptr<Scope> currentScope);
Value evaluateBinaryOperation(const BinaryOpNode* binaryOpNode, std::shared_ptr<Scope> currentScope);
Value evaluateAssignment(const AssignmentNode* assignmentNode, std::shared_ptr<Scope> currentScope);
Value evaluateExpression(const ASTNode* node, std::shared_ptr<Scope> currentScope);
Value evaluateFunctionCall(const CallNode* callNode, std::shared_ptr<Scope> currentScope);

Value lenFunction(const std::vector<Value>& args);
Value popFunction(std::vector<Value>& args);
Value pushFunction(std::vector<Value>& args);

std::shared_ptr<Scope> globalScope = std::make_shared<Scope>();

// function to create an indentation string
std::string indentString(int indentLevel) {
    return std::string(indentLevel * 4, ' '); // 4 spaces per indent level
}

void formatNullNode(std::ostream& os, const NullNode* node, int indent) {
    os << indentString(indent) << "null";
}

// function to format operation types
void formatBinaryOpNode(std::ostream& os, const BinaryOpNode* node, int indent) {
    os << '(';
    formatAST(os, node->left, 0, false);
    os << ' ' << node->op.value << ' ';
    formatAST(os, node->right, 0, false);
    os << ')';
}

// function to format numbers (especially doubles)
void formatNumberNode(std::ostream& os, const NumberNode* node, int indent) {
    double value = std::stod(node->value.value);
    double intPart;
    double fracPart = modf(value, &intPart);
    
    if (fracPart == 0.0) {
        os << indentString(indent) << static_cast<long>(intPart);
    } else {
        if (abs(value) < 1e-4 || abs(value) > 1e4) {
            std::ostringstream tempStream;
            tempStream << std::scientific << std::setprecision(0) << value;
            std::string str = tempStream.str();
            size_t ePos = str.find('e');
            size_t lastNonZeroPos = str.find_last_not_of('0', ePos - 1);
            if (lastNonZeroPos != std::string::npos && lastNonZeroPos + 1 < ePos) {
                str.erase(lastNonZeroPos + 1, ePos - lastNonZeroPos - 1);
            }
            os << indentString(indent) << str;
        } else {
            std::ostringstream tempStream;
            tempStream << std::fixed << std::setprecision(1) << value;
            std::string str = tempStream.str();
            str.erase(str.find_last_not_of('0') + 1, std::string::npos);
            if (str.back() == '.') {
                str.pop_back();
            }
            os << indentString(indent) << str;
        }
    }
}



// function to format Booleans
void formatBooleanNode(std::ostream& os, const BooleanNode* node, int indent) {
    os << indentString(indent) << node->value.value;
}

// function to format Variables
void formatVariableNode(std::ostream& os, const VariableNode* node, int indent) {
    os << indentString(indent) << node->identifier.value;
}

// function to format assignment nodes
void formatAssignmentNode(std::ostream& os, const AssignmentNode* node, int indent) {
    os << indentString(indent) << "(";
    formatAST(os, node->lhs, 0, false);

    os << " = ";
    formatAST(os, node->rhs, 0, false);

    os << ")";
}


// function to format block nodes
void formatBlockNode(std::ostream& os, const BlockNode* node, int indent) {
    bool isFirstStatement = true;
    for (const auto& stmt : node->statements) {
        if (!isFirstStatement) {
            os << "\n";
        }
        formatAST(os, stmt, indent);
        isFirstStatement = false;
    }
}

// main format function
void formatAST(std::ostream& os, const std::unique_ptr<ASTNode>& node, int indent, bool isOutermost)  {
    if (!node) return;

    switch (node->getType()) {
        case ASTNode::Type::BinaryOpNode:
            formatBinaryOpNode(os, static_cast<const BinaryOpNode*>(node.get()), indent);
            break;
        case ASTNode::Type::NumberNode:
            formatNumberNode(os, static_cast<const NumberNode*>(node.get()), indent);
            break;
        case ASTNode::Type::BooleanNode:
            formatBooleanNode(os, static_cast<const BooleanNode*>(node.get()), indent);
            break;
        case ASTNode::Type::VariableNode:
            formatVariableNode(os, static_cast<const VariableNode*>(node.get()), indent);
            break;
        case ASTNode::Type::AssignmentNode:
            formatAssignmentNode(os, static_cast<const AssignmentNode*>(node.get()), indent);
            break;
        case ASTNode::Type::BlockNode:
            formatBlockNode(os, static_cast<const BlockNode*>(node.get()), indent);
            break;
        case ASTNode::Type::NullNode:
            formatNullNode(os, static_cast<const NullNode*>(node.get()), indent);
            break;
        case ASTNode::Type::CallNode:
            formatCallNode(os, static_cast<const CallNode*>(node.get()), indent, isOutermost);
        break;
        case ASTNode::Type::ArrayLiteralNode:
            formatArrayLiteralNode(os, static_cast<const ArrayLiteralNode*>(node.get()), indent, isOutermost);
            break;
        case ASTNode::Type::ArrayLookupNode:
            formatArrayLookupNode(os, static_cast<const ArrayLookupNode*>(node.get()), indent, isOutermost);
            break;
        default:
            os << indentString(indent) << "/* Unknown node type */";
            break;
    }
}

void formatCallNode(std::ostream& os, const CallNode* node, int indent, bool isOutermost) {
    formatAST(os, node->callee, indent, false);
    os << '(';
    for (size_t i = 0; i < node->arguments.size(); ++i) {
        formatAST(os, node->arguments[i], 0, false);
        if (i < node->arguments.size() - 1) {
            os << ", ";
        }
    }
    os << ")";
}

// Function to format FunctionNode (function definitions)
void formatFunctionNode(std::ostream& os, const FunctionNode* node, int indent) {
    os << indentString(indent) << "def " << node->name.value << "(";
    for (size_t i = 0; i < node->parameters.size(); ++i) {
        os << node->parameters[i].value;
        if (i < node->parameters.size() - 1) {
            os << ", ";
        }
    }
    os << ") {";
    
    const BlockNode* blockNode = dynamic_cast<const BlockNode*>(node->body.get());
    if (blockNode && !blockNode->statements.empty()) {
        os << "\n";
        formatAST(os, node->body, indent + 1);
        os << "\n" << indentString(indent);
    } else {
        os << "\n" << indentString(indent);
    }
    os << "}";
}






// Function to format CallNode (function calls)

void formatArrayLiteralNode(std::ostream& os, const ArrayLiteralNode* node, int indent, bool isOutermost = true) {;
    os << indentString(indent) << "[";
    for (size_t i = 0; i < node->elements.size(); ++i) {
        formatAST(os, node->elements[i], 0, false); 
        if (i < node->elements.size() - 1) os << ", ";
    }
    os << "]";
}

// Function to format ArrayLookupNode (array access)
void formatArrayLookupNode(std::ostream& os, const ArrayLookupNode* node, int indent, bool isOutermost) {
    formatAST(os, node->array, indent, false);

    os << "[";
    formatAST(os, node->index, 0, false);
    os << "]";
}

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

void formatAndEvaluateAST(const std::unique_ptr<ASTNode>& ast, std::shared_ptr<Scope> scope) {
    std::ostringstream formattedOutput;
    formatAST(formattedOutput, ast, 0, true);
    std::cout << formattedOutput.str() << std::endl;
    try {
        Value result = evaluateExpression(ast.get(), scope);
        printValue(result);
        std::cout << std::endl;
    } catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
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
            case ASTNode::Type::BlockNode: {
                auto blockNode = static_cast<const BlockNode*>(node);
                Value lastValue;
                for (const auto& stmt : blockNode->statements) {
                    lastValue = evaluateExpression(stmt.get(), currentScope);
                }
                return lastValue;
            }
            case ASTNode::Type::NullNode: {
                return Value();
            }
            case ASTNode::Type::CallNode: {
                return evaluateFunctionCall(static_cast<const CallNode*>(node), currentScope);
            }
            case ASTNode::Type::ArrayLiteralNode: {
                auto arrayLiteralNode = static_cast<const ArrayLiteralNode*>(node);
                std::vector<Value> arrayValues;
                for (const auto& element : arrayLiteralNode->elements) {
                    Value copiedElement = evaluateExpression(element.get(), currentScope).deepCopy();
                    arrayValues.push_back(copiedElement);
                }
                return Value(arrayValues);
            }
            case ASTNode::Type::ArrayLookupNode: {
                auto arrayLookupNode = static_cast<const ArrayLookupNode*>(node);
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
            default:
                throw std::runtime_error("Unknown expression node type");
        }
    } catch (...) {
        throw;
    }
}

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
                throw std::runtime_error("Runtime error: division by zero.");
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
        case TokenType::LOGICAL_XOR: 
            return Value(left.asBool() != right.asBool());
        case TokenType::LOGICAL_OR:
            return Value(left.asBool() || right.asBool());
        case TokenType::ASSIGN:
            if (binaryOpNode->left->getType() == ASTNode::Type::VariableNode) {
                const auto* variableNode = static_cast<const VariableNode*>(binaryOpNode->left.get());
                currentScope->setVariable(variableNode->identifier.value, right);
                return right;
            } else {
                throw std::runtime_error("Runtime error: invalid assignee.");
            }
        default:
            throw std::runtime_error("Unsupported binary operator in evaluateBinaryOperation");
    }
}

Value evaluateFunctionCall(const CallNode* callNode, std::shared_ptr<Scope> currentScope) {
    if (!callNode) {
        throw std::runtime_error("Null CallNode passed to evaluateFunctionCall");
    }

    auto functionName = static_cast<const VariableNode*>(callNode->callee.get())->identifier.value;

    std::vector<Value> evaluatedArgs;
    for (const auto& arg : callNode->arguments) {
        evaluatedArgs.push_back(evaluateExpression(arg.get(), currentScope));
    }
    if (functionName == "push") {
        return pushFunction(evaluatedArgs);
    } else if (functionName == "pop") {
        return popFunction(evaluatedArgs);
    } else if (functionName == "len") {
        return lenFunction(evaluatedArgs);
    } else {
        throw std::runtime_error("Unknown function name: " + functionName);
    }
}

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

Value lenFunction(const std::vector<Value>& args) {
    if (args.size() != 1 || !args[0].isArray()) {
        throw std::runtime_error("Runtime error: incorrect argument count.");
    }
    return Value(static_cast<double>(args[0].asArray().size()));
}

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

Value pushFunction(std::vector<Value>& args) {
    if (args.size() != 2 || !args[0].isArray()) {
        throw std::runtime_error("Runtime error: incorrect argument count.");
    }
    args[0].asArray().push_back(args[1]);
    return Value();
}



int main() {
    std::shared_ptr<Scope> globalScope = std::make_shared<Scope>();
    std::string line;
    std::ostream& os = std::cout;

    globalScope->setVariable("len", Value(Value::FunctionPtr(lenFunction)));
    globalScope->setVariable("pop", Value(Value::FunctionPtr(popFunction)));
    globalScope->setVariable("push", Value(Value::FunctionPtr(pushFunction)));

    while (true) { 
        if (!std::getline(std::cin, line)) {
            if (std::cin.eof()) {
                break;
            } else {
                return 1;
            }
        }

        try {
            Lexer lexer(line);
            auto tokens = lexer.tokenize();
            if (lexer.isSyntaxError(tokens)) {
                continue; 
            }
            Parser parser(tokens);
            auto ast = parser.parse();

            formatAndEvaluateAST(ast, globalScope);
        } catch (const std::exception& e) {
            os << e.what() << std::endl;
        }
    }

    return 0;
}