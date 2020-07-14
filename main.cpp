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
	}
	else
		std::cout << (char)op;
	std::cout << " ";
}
void printType(int op)
{
	std::cout << " ";
	if (op >= 256) {
		switch (op) {
		case expr::TokenType_Float: std::cout << "float"; break;
		case expr::TokenType_Float2: std::cout << "vec2"; break;
		case expr::TokenType_Float3: std::cout << "vec3"; break;
		case expr::TokenType_Float4: std::cout << "vec4"; break;
		case expr::TokenType_Float2x2: std::cout << "mat2"; break;
		case expr::TokenType_Float3x3: std::cout << "mat3"; break;
		case expr::TokenType_Float4x4: std::cout << "mat4"; break;
		case expr::TokenType_Int: std::cout << "int"; break;
		case expr::TokenType_Int2: std::cout << "ivec2"; break;
		case expr::TokenType_Int3: std::cout << "ivec3"; break;
		case expr::TokenType_Int4: std::cout << "ivec4"; break;
		case expr::TokenType_Uint: std::cout << "uint"; break;
		case expr::TokenType_Uint2: std::cout << "uvec2"; break;
		case expr::TokenType_Uint3: std::cout << "uvec3"; break;
		case expr::TokenType_Uint4: std::cout << "uvec4"; break;
		case expr::TokenType_Bool: std::cout << "bool"; break;
		case expr::TokenType_Bool2: std::cout << "bvec2"; break;
		case expr::TokenType_Bool3: std::cout << "bvec3"; break;
		case expr::TokenType_Bool4: std::cout << "bvec4"; break;
		}
	}
	else
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
	case expr::NodeType::FloatLiteral:
		std::cout << ((expr::FloatLiteralNode*)node)->Value;
		break;
	case expr::NodeType::BooleanLiteral:
		std::cout << ((expr::BooleanLiteralNode*)node)->Value;
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
	case expr::NodeType::Cast: {
		expr::CastNode* cast = ((expr::CastNode*)node);
		std::cout << "(";
		printType(cast->Type);
		std::cout << ")";
		std::cout << "(";
		visit(cast->Object);
		std::cout << ")";
	} break;
	}
}

int main()
{
	std::string e = "a * b || + c";
	expr::Parser parser(e.c_str(), e.size());
	expr::Node* root = parser.Parse();

	if (parser.Error()) {
		std::cout << parser.ErrorMessage() << std::endl;
	} else visit(root);
	
	parser.Clear();
	
	return 0;
}
