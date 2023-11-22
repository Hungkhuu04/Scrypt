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
void formatArrayLiteralNode(std::ostream& os, const ArrayLiteralNode* node, int indent, bool isOutermost);
void formatArrayLookupNode(std::ostream& os, const ArrayLookupNode* node, int indent, bool isOutermost);

Value evaluateVariable(const VariableNode* variableNode, std::shared_ptr<Scope> currentScope);
Value evaluateBinaryOperation(const BinaryOpNode* binaryOpNode, std::shared_ptr<Scope> currentScope);
Value evaluateAssignment(const AssignmentNode* assignmentNode, std::shared_ptr<Scope> currentScope);
Value evaluateExpression(const ASTNode* node, std::shared_ptr<Scope> currentScope);

std::shared_ptr<Scope> globalScope = std::make_shared<Scope>();

//FROM HERE DOWN IS FORMAT.CPP
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
    formatAST(os, node->left, 0, false); // Passing false for isOutermost
    os << ' ' << node->op.value << ' ';
    formatAST(os, node->right, 0, false); // Passing false for isOutermost
    os << ')';
}

// function to format numbers (especially doubles)
void formatNumberNode(std::ostream& os, const NumberNode* node, int indent) {
    double value = std::stod(node->value.value);
    
    // Check for a non-zero fractional part
    double intPart;
    double fracPart = modf(value, &intPart);
    
    if (fracPart == 0.0) {
        // No fractional part, treat as integer
        os << indentString(indent) << static_cast<long>(intPart);
    } else {
        // Use scientific notation for very small values
        if (abs(value) < 1e-4 || abs(value) > 1e4) {
            std::ostringstream tempStream;
            tempStream << std::scientific << std::setprecision(0) << value;
            std::string str = tempStream.str();
            // Remove trailing zeros and decimal point from exponent part
            size_t ePos = str.find('e');
            size_t lastNonZeroPos = str.find_last_not_of('0', ePos - 1);
            if (lastNonZeroPos != std::string::npos && lastNonZeroPos + 1 < ePos) {
                str.erase(lastNonZeroPos + 1, ePos - lastNonZeroPos - 1);
            }
            os << indentString(indent) << str;
        } else {
            // Regular formatting for other cases
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

    // Format left-hand side (LHS)
    formatAST(os, node->lhs, 0, false);  // Format LHS regardless of its type

    os << " = ";

    // Format right-hand side (RHS)
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
        os << "\n" << indentString(indent); // Add newline and indentation for empty body
    }
    os << "}"; // Closing brace
}






// Function to format CallNode (function calls)

void formatArrayLiteralNode(std::ostream& os, const ArrayLiteralNode* node, int indent, bool isOutermost = true) {;
    os << indentString(indent) << "[";
    for (size_t i = 0; i < node->elements.size(); ++i) {
        formatAST(os, node->elements[i], 0, false); // Passing false for isOutermost
        if (i < node->elements.size() - 1) os << ", ";
    }
    os << "]";

    // Add a semicolon only if it's the outermost array literal
}
// Function to format ArrayLookupNode (array access)
void formatArrayLookupNode(std::ostream& os, const ArrayLookupNode* node, int indent, bool isOutermost) {
    // Format the array part
    formatAST(os, node->array, indent, false); // Pass false to isOutermost

    // Format the index part
    os << "[";
    formatAST(os, node->index, 0, false); // Pass false to isOutermost
    os << "]";

    // Append a semicolon if it's a standalone array lookup expression
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
                printValue(array[i]);  // Recursive call
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
    // Format the AST
    std::ostringstream formattedOutput;
    formatAST(formattedOutput, ast, 0, true);
    std::cout << formattedOutput.str() << std::endl;

    // Evaluate the AST
    try {
        Value result = evaluateExpression(ast.get(), scope);
        printValue(result);  // Use the helper function to print the result
        std::cout << std::endl;
    } catch (const std::exception& e) {
        std::cout << e.what() << std::endl;  // Outputting the error message directly
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
                return lastValue; // Or decide how you want to handle the value of a block
            }
            case ASTNode::Type::NullNode: {
                return Value();
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

Value evaluateAssignment(const AssignmentNode* assignmentNode, std::shared_ptr<Scope> currentScope) {
    if (!assignmentNode) {
        throw std::runtime_error("Null assignment node passed to evaluateAssignment");
    }

    // Evaluate the right-hand side (rhs) expression first
    Value rhsValue = evaluateExpression(assignmentNode->rhs.get(), currentScope);

    // Handle assignment based on the type of the left-hand side (lhs)
    if (assignmentNode->lhs->getType() == ASTNode::Type::VariableNode) {
        // Variable assignment
        auto variableNode = static_cast<const VariableNode*>(assignmentNode->lhs.get());
        currentScope->setVariable(variableNode->identifier.value, rhsValue);
    } else if (assignmentNode->lhs->getType() == ASTNode::Type::ArrayLookupNode) {
        auto arrayLookupNode = static_cast<const ArrayLookupNode*>(assignmentNode->lhs.get());

        // Evaluate the array part to get the array
        Value arrayValue = evaluateExpression(arrayLookupNode->array.get(), currentScope);
        if (arrayValue.getType() != Value::Type::Array) {
            throw std::runtime_error("Runtime error: not an array.");
        }
        std::vector<Value>& array = arrayValue.asArray();

        // Evaluate the index expression
        Value indexValue = evaluateExpression(arrayLookupNode->index.get(), currentScope);
        if (indexValue.getType() != Value::Type::Double || modf(indexValue.asDouble(), nullptr) != 0.0) {
            throw std::runtime_error("Runtime error: index is not an integer.");
        }

        int index = static_cast<int>(indexValue.asDouble());
        if (index < 0 || index >= static_cast<int>(array.size())) {
            throw std::runtime_error("Runtime error: index out of bounds.");
        }

        // Perform the assignment
        array[index] = rhsValue;

        // Return the assigned value
        return rhsValue;
    }
    else {
        // If lhs is neither a variable nor an array lookup, throw an error
        throw std::runtime_error("Runtime error: invalid assignee.");
    }

    // Return the rhs value, which is the result of the assignment expression
    return rhsValue;
}



int main() {
    std::shared_ptr<Scope> globalScope = std::make_shared<Scope>();
    std::string line;
    std::ostream& os = std::cout;

    while (true) {  // Infinite loop
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
                continue;  // Go to the next iteration of the loop
            }
            Parser parser(tokens);
            auto ast = parser.parse();

            // Format and evaluate the AST
            formatAndEvaluateAST(ast, globalScope);
        } catch (const std::exception& e) {
            os << e.what() << std::endl;
        }
    }

    return 0;
}
