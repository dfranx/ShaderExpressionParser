#include "Parser.h"
#include <string.h>

namespace expr
{
	Parser::Parser(const char* buffer, size_t bufLength) :
		m_token(buffer, bufLength)
	{
		m_hasError = false;
		m_error = "";

		m_opPrecedence['*'] = 1;
		m_opPrecedence['/'] = 1;
		m_opPrecedence['%'] = 1;

		m_opPrecedence['+'] = 2;
		m_opPrecedence['-'] = 2;

		m_opPrecedence[TokenType_BitshiftLeft] = 3;
		m_opPrecedence[TokenType_BitshiftRight] = 3;

		m_opPrecedence['<'] = 4;
		m_opPrecedence['>'] = 4;
		m_opPrecedence[TokenType_LessThanEqual] = 4;
		m_opPrecedence[TokenType_GreaterThanEqual] = 4;

		m_opPrecedence[TokenType_Equal] = 5;
		m_opPrecedence[TokenType_NotEqual] = 5;

		m_opPrecedence['&'] = 6;
		m_opPrecedence['^'] = 7;
		m_opPrecedence['|'] = 8;
		m_opPrecedence[TokenType_LogicAnd] = 9;
		m_opPrecedence[TokenType_LogicOr] = 10;

		m_opPrecedence[TokenTypeCount] = 10; // store max here
	}

	Node* Parser::Parse()
	{
		m_token.Next();
		Node* ret = m_parseTernaryExpression();
		
		// leftover tokens
		if (m_token.GetTokenType() >= 0) {
			m_error = "Not fully parsed.";
			m_hasError = true;
		}

		// check for nulls
		for (auto& node : m_list) {
			if (m_hasError) break;

			switch (node->GetNodeType()) {
			case expr::NodeType::BinaryExpression: {
				expr::BinaryExpressionNode* bexpr = ((expr::BinaryExpressionNode*)node);
				if (bexpr->Left == nullptr || bexpr->Right == nullptr)
					m_hasError = true;
			} break;
			case expr::NodeType::FunctionCall: {
				expr::FunctionCallNode* fcall = ((expr::FunctionCallNode*)node);
				for (int i = 0; i < fcall->Arguments.size(); i++) {
					if (fcall->Arguments[i] == nullptr) {
						m_hasError = true;
						break;
					}
				}
			} break;
			case expr::NodeType::UnaryExpression: {
				expr::UnaryExpressionNode* uexpr = ((expr::UnaryExpressionNode*)node);
				if (uexpr->Child == nullptr)
					m_hasError = true;
			} break;
			case expr::NodeType::ArrayAccess: {
				expr::ArrayAccessNode* a_access = ((expr::ArrayAccessNode*)node);
				if (a_access->Object == nullptr)
					m_hasError = true;

				for (int i = 0; i < a_access->Indices.size(); i++) {
					if (a_access->Indices[i] == nullptr) {
						m_hasError = true;
						break;
					}
				}
			} break;
			case expr::NodeType::MemberAccess: {
				expr::MemberAccessNode* m_access = ((expr::MemberAccessNode*)node);
				if (m_access->Object == nullptr)
					m_hasError = true;
			} break;
			case expr::NodeType::MethodCall: {
				expr::MethodCallNode* mcall = ((expr::MethodCallNode*)node);
				if (mcall->Object == nullptr)
					m_hasError = true;
				for (int i = 0; i < mcall->Arguments.size(); i++)
					if (mcall->Arguments[i] == nullptr) {
						m_hasError = true;
						break;
					}
			} break;
			case expr::NodeType::Cast: {
				expr::CastNode* cast = ((expr::CastNode*)node);
				if (cast->Object == nullptr)
					m_hasError = true;
			} break;
			case expr::NodeType::TernaryExpression: {
				expr::TernaryExpressionNode* texpr = ((expr::TernaryExpressionNode*)node);
				if (texpr->Condition == nullptr || texpr->OnTrue == nullptr || texpr->OnFalse == nullptr)
					m_hasError = true;
			} break;
			}

			if (m_hasError)
				m_error = "error occured - null child";
		}

		return ret;
	}
	void Parser::Clear()
	{
		for (auto& el : m_list)
			delete el;
		m_list.clear();
	}
	bool Parser::m_eat(int tokenType)
	{
		if (m_token.GetTokenType() == tokenType)
			m_token.Next();
		else {
			if (!m_hasError)
				m_error = "Expected 'tokenType' got 'm_token.GetTokenType()'";
			m_hasError = true;

			return false;
		}
		return true;
	}
	bool Parser::m_isToken(int tokenType)
	{
		return (m_token.GetTokenType() == tokenType);
	}
	bool Parser::m_isLValue(NodeType nodeType)
	{
		return (nodeType == NodeType::ArrayAccess || nodeType == NodeType::MemberAccess || nodeType == NodeType::Identifier);
	}
	bool Parser::m_isType(int tokenType)
	{
		return (tokenType == TokenType_Bool || tokenType == TokenType_Bool2 || tokenType == TokenType_Bool3 || tokenType == TokenType_Bool4) ||
			(tokenType == TokenType_Int || tokenType == TokenType_Int2 || tokenType == TokenType_Int3 || tokenType == TokenType_Int4) ||
			(tokenType == TokenType_Float || tokenType == TokenType_Float2 || tokenType == TokenType_Float3 || tokenType == TokenType_Float4) ||
			(tokenType == TokenType_Uint || tokenType == TokenType_Uint2 || tokenType == TokenType_Uint3 || tokenType == TokenType_Uint4) ||
			(tokenType == TokenType_Float2x2 || tokenType == TokenType_Float3x3 || tokenType == TokenType_Float4x4);
	}
	Node* Parser::m_parseFunctionCall(char* fname, int tokType)
	{
		FunctionCallNode* node = (FunctionCallNode*)m_allocateNode<FunctionCallNode>();
		memcpy(node->Name, fname, 256);
		node->TokenType = tokType;

		m_eat('(');
		m_parseArguments(node->Arguments);
		m_eat(')');

		Node* ret = node;
		Node* ext = m_parseExtIdentifier(node);
		if (ext != nullptr) ret = ext;

		return ret;
	}
	Node* Parser::m_parseArrayAccess(Node* parent)
	{
		ArrayAccessNode* node = (ArrayAccessNode*)m_allocateNode<ArrayAccessNode>();
		node->Object = parent;

		while (m_isToken('[')) {
			m_eat('[');
			node->Indices.push_back(m_parseTernaryExpression());
			m_eat(']');
		}

		Node* ret = node;
		Node* ext = m_parseExtIdentifier(node);
		if (ext != nullptr) ret = ext;

		return ret;
	}
	Node* Parser::m_parseMemberAccess(Node* parent)
	{
		m_eat('.');

		char identifier[256] = { 0 };
		memcpy(identifier, m_token.GetIdentifier(), 256);
		m_eat(TokenType_Identifier);

		Node* ret = nullptr;

		if (m_isToken('(')) {
			MethodCallNode* node = (MethodCallNode*)m_allocateNode<MethodCallNode>();
			memcpy(node->Name, identifier, 256);
			node->Object = parent;
			node->TokenType = TokenType_Identifier;

			m_eat('(');
			m_parseArguments(node->Arguments);
			m_eat(')');

			ret = (Node*)node;
		}
		else {
			MemberAccessNode* node = (MemberAccessNode*)m_allocateNode<MemberAccessNode>();
			memcpy(node->Field, identifier, 256);
			node->Object = parent;
			ret = (Node*)node;
		}

		Node* ext = m_parseExtIdentifier(ret);
		if (ext != nullptr) ret = ext;

		return ret;
	}
	void Parser::m_parseArguments(std::vector<Node*>& args)
	{
		Node* arg = m_parseTernaryExpression();
		while (arg != nullptr) {
			args.push_back(arg);

			if (m_isToken(',')) {
				m_eat(',');
				arg = m_parseTernaryExpression();

				if (arg == nullptr)
					m_hasError = true;
			}
			else arg = nullptr;
		}
	}
	Node* Parser::m_parseExtIdentifier(Node* parent)
	{
		if (m_isToken('.'))
			return m_parseMemberAccess(parent);
		if (m_isToken('['))
			return m_parseArrayAccess(parent);

		// postfix increment / decrement
		if (m_isToken(TokenType_Increment) || m_isToken(TokenType_Decrement)) {
			if (m_isLValue(parent->GetNodeType())) {
				int operatorType = m_token.GetTokenType();
				m_eat(operatorType);
				UnaryExpressionNode* uOp = (UnaryExpressionNode*)m_allocateNode<UnaryExpressionNode>();
				uOp->Operator = operatorType;
				uOp->Child = parent;
				uOp->IsPost = true;
				return uOp;
			} else {
				m_hasError = true;
				m_error = "lvalue required";
			}
		}

		return nullptr; // return parent?
	}
	Node* Parser::m_parseValue()
	{
		Node* ret = nullptr;
		if (m_isToken('+') || m_isToken('-') || m_isToken('!') || m_isToken('~')) {
			int operatorType = m_token.GetTokenType();
			m_eat(operatorType);
			UnaryExpressionNode* uOp = (UnaryExpressionNode*)m_allocateNode<UnaryExpressionNode>();
			uOp->Operator = operatorType;
			uOp->Child = m_parseValue();
			uOp->IsPost = false;
			ret = uOp;
		} else if (m_isToken(TokenType_IntegerLiteral)) {
			IntegerLiteralNode* node = (IntegerLiteralNode*)m_allocateNode<IntegerLiteralNode>();
			node->Value = m_token.GetIntValue();
			m_eat(TokenType_IntegerLiteral);
			ret = node;
		} else if (m_isToken(TokenType_FloatLiteral)) {
			FloatLiteralNode* node = (FloatLiteralNode*)m_allocateNode<FloatLiteralNode>();
			node->Value = m_token.GetFloatValue();
			m_eat(TokenType_FloatLiteral);
			ret = node;
		} else if (m_isToken(TokenType_BooleanLiteral)) {
			BooleanLiteralNode* node = (BooleanLiteralNode*)m_allocateNode<BooleanLiteralNode>();
			node->Value = m_token.GetIntValue();
			m_eat(TokenType_BooleanLiteral);
			ret = node;
		} else if (m_isToken(TokenType_Identifier) || m_isToken(TokenType_Increment) || m_isToken(TokenType_Decrement)) {
			ret = m_parseIdentifier();
		} else if (m_isToken('(')) {
			m_eat('(');
			bool canHaveMember = false;
			if (m_isType(m_token.GetTokenType())) {
				int castType = m_token.GetTokenType();

				m_eat(castType);

				if (!m_isToken(')')) {
					m_token.Undo();

					canHaveMember = true;
					ret = m_parseTernaryExpression();
					m_eat(')');
				} else {
					m_eat(')');

					CastNode* node = (CastNode*)m_allocateNode<CastNode>();
					node->Object = m_parseValue();
					node->Type = castType;

					ret = node;
				}
			} else {
				canHaveMember = true;
				ret = m_parseTernaryExpression();
				m_eat(')');
			}

			if (m_isToken('.')) {
				if (canHaveMember)
					ret = m_parseMemberAccess(ret);
				else {
					m_hasError = true;
					m_error = "invalid member access";
				}
			}
		}
		else if (m_isType(m_token.GetTokenType())) {
			// cache identifier
			int tokType = m_token.GetTokenType();
			char identifier[256];
			memcpy(identifier, m_token.GetIdentifier(), 256);

			m_eat(m_token.GetTokenType());

			ret = m_parseFunctionCall(identifier, tokType);
		}

		return ret;
	}
	Node* Parser::m_parseTernaryExpression()
	{
		Node* node = m_parseExpression(m_opPrecedence[TokenTypeCount]);

		if (m_isToken('?')) {
			m_eat('?');

			TernaryExpressionNode* ternNode = (TernaryExpressionNode*)m_allocateNode<TernaryExpressionNode>();
			ternNode->Condition = node;
			ternNode->OnTrue = m_parseTernaryExpression();
			m_eat(':');
			ternNode->OnFalse = m_parseTernaryExpression();

			node = (Node*)ternNode;
		}

		return node;
	}
	Node* Parser::m_parseExpression(int prec)
	{
		if (prec == 0)
			return m_parseValue();

		Node* node = m_parseExpression(prec - 1);

		while (true) {
			bool shouldContinue = false;
			for (const auto& op : m_opPrecedence)
				if (op.second == prec && m_isToken(op.first)) {
					shouldContinue = true;
					break;
				}

			if (!shouldContinue)
				break;

			int operatorType = m_token.GetTokenType();
			m_eat(operatorType);

			BinaryExpressionNode* binNode = (BinaryExpressionNode*)m_allocateNode<BinaryExpressionNode>();
			binNode->Left = node;
			binNode->Operator = operatorType;
			binNode->Right = m_parseExpression(prec - 1);

			node = (Node*)binNode;
		}

		return node;
	}
	Node* Parser::m_parseIdentifier()
	{
		// prefix increment / decrement
		int operatorType = -1;
		if (m_isToken(TokenType_Increment) || m_isToken(TokenType_Decrement)) {
			operatorType = m_token.GetTokenType();
			m_eat(operatorType);
		}

		// cache identifier
		int identTokType = m_token.GetTokenType();
		char identifier[256];
		memcpy(identifier, m_token.GetIdentifier(), 256);
		m_eat(TokenType_Identifier);

		// function call
		if (m_isToken('(')) {
			if (operatorType > 0) {
				m_error = "lvalue required";
				m_hasError = true;
			}

			return m_parseFunctionCall(identifier, identTokType);
		}

		// extended
		IdentifierNode* node = (IdentifierNode*)m_allocateNode<IdentifierNode>();
		memcpy(node->Name, identifier, 256);
		Node* ret = node;
		Node* ext = m_parseExtIdentifier(node);
		if (ext != nullptr) ret = ext;

		// prefix increment/decrement
		if (operatorType > 0) {
			if (m_isLValue(ret->GetNodeType())) {
				UnaryExpressionNode* uOp = (UnaryExpressionNode*)m_allocateNode<UnaryExpressionNode>();
				uOp->Operator = operatorType;
				uOp->Child = ret;
				uOp->IsPost = false;
				ret = uOp;
			} else {
				m_hasError = true;
				m_error = "lvalue required";
			}
		}
		
		return ret;
	}
}