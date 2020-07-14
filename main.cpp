#include <iostream>
#include <vector>

#include "Parser.h"

void printOp(int op)
{
	std::cout << " ";
	if (op >= 256) {
		switch (op) {
			case expr::TokenType_Equal: std::cout << "=="; break;
			case expr::TokenType_NotEqual: std::cout << "!="; break;
			case expr::TokenType_LessEqual: std::cout << "<="; break;
			case expr::TokenType_GreaterEqual: std::cout << ">="; break;
			case expr::TokenType_LogicAnd: std::cout << "&&"; break;
			case expr::TokenType_LogicOr: std::cout << "||"; break;
			case expr::TokenType_BitshiftLeft: std::cout << "<<"; break;
			case expr::TokenType_BitshiftRight: std::cout << ">>"; break;
			case expr::TokenType_Increment: std::cout << "++"; break;
			case expr::TokenType_Decrement: std::cout << "--"; break;
		}
	} else
		std::cout << (char)op;
	std::cout << " ";
}

void visit(expr::Node* node)
{
	if (node == nullptr)
		return;

	switch (node->GetNodeType()) {
	case expr::NodeType::BinaryExpression: {
		expr::BinaryExpressionNode* bexpr = ((expr::BinaryExpressionNode*)node);

		std::cout << "(";
		visit(bexpr->Left);
		printOp(bexpr->Operator);
		visit(bexpr->Right);
		std::cout << ")";
	} break;
	case expr::NodeType::FunctionCall: {
		expr::FunctionCallNode* fcall = ((expr::FunctionCallNode*)node);
		std::cout << fcall->Name << "(";
		for (int i = 0; i < fcall->Arguments.size(); i++) {
			visit(fcall->Arguments[i]);
			if (i != fcall->Arguments.size()-1)
				std::cout << ", ";
		}
		std::cout << ")";

	} break;
	case expr::NodeType::UnaryExpression: {
		expr::UnaryExpressionNode* uexpr = ((expr::UnaryExpressionNode*)node);
		if (uexpr->IsPost) {
			visit(uexpr->Child);
			printOp(uexpr->Operator);
		} else {
			printOp(uexpr->Operator);
			visit(uexpr->Child);
		}
	} break;
	case expr::NodeType::IntegerLiteral:
		std::cout << ((expr::IntegerLiteralNode*)node)->Value;
		break;
	case expr::NodeType::Identifier:
		std::cout << ((expr::IdentifierNode*)node)->Name;
		break;
	case expr::NodeType::ArrayAccess: {
		expr::ArrayAccessNode* a_access = ((expr::ArrayAccessNode*)node);
		visit(a_access->Object);
		for (int i = 0; i < a_access->Indices.size(); i++) {
			std::cout << "[";
			visit(a_access->Indices[i]);
			std::cout << "]";
		}
	} break;
	case expr::NodeType::MemberAccess: {
		expr::MemberAccessNode* m_access = ((expr::MemberAccessNode*)node);
		std::cout << "(";
		visit(m_access->Object);
		std::cout << ")";
		std::cout << '.' << m_access->Field;
	} break;
	}
}

int main()
{
	std::string e = "myFunc() % 3";
	expr::Parser parser(e.c_str(), e.size());
	expr::Node* root = parser.Parse();

	if (parser.Error()) {
		std::cout << parser.ErrorMessage() << std::endl;
	} else visit(root);
	
	parser.Clear();

	return 0;
}
