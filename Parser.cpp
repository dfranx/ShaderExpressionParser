#include "Parser.h"

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
		m_opPrecedence[TokenType_LessEqual] = 4;
		m_opPrecedence[TokenType_GreaterEqual] = 4;

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
		/*
			expr   : term ((PLUS | MINUS) term)*
			term   : factor ((MUL | DIV | MOD) factor)*
			factor : (PLUS | MINUS) factor | INTEGER | LPAREN expr RPAREN (member) | identf
			extidnt: (array | member) (INCREMENT | DECREMENT)
			identf : (INCREMENT | DECREMENT) IDENTIFIER (fcall | extidnt)
			fcall  : LPAREN (expr (COMMA expr (COMMA expr ...))) RPAREN (extind)
			array  : LBRACKET expr RBRACKET (extidnt)
			member : DOT IDENTIFIER (extidnt)
		*/

		m_token.Next();
		Node* ret = m_expr();
		if (m_token.GetTokenType() >= 0) {
			m_error = "Not fully parsed.";
			m_hasError = true;
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
		NodeType nt = (NodeType)nodeType;
		return (nt == NodeType::ArrayAccess || nt == NodeType::MemberAccess || nt == NodeType::Identifier);
	}
	Node* Parser::m_fcall(char* fname)
	{
		FunctionCallNode* node = (FunctionCallNode*)m_allocateNode<FunctionCallNode>();
		memcpy(node->Name, fname, 256);

		/* LPAREN (expr (COMMA expr (COMMA expr ...))) RPAREN */
		m_eat('(');

		Node* arg = m_expr();
		while (arg != nullptr) {
			node->Arguments.push_back(arg);

			if (m_isToken(',')) {
				m_eat(',');
				arg = m_expr();

				if (arg == nullptr) {
					// throw error
				}
			}
			else arg = nullptr;
		}

		m_eat(')');

		Node* ret = node;
		Node* ext = m_extidnt(node);
		if (ext != nullptr) ret = ext;

		return ret;
	}
	Node* Parser::m_array(Node* parent)
	{
		ArrayAccessNode* node = (ArrayAccessNode*)m_allocateNode<ArrayAccessNode>();
		node->Object = parent;

		while (m_isToken('[')) {
			m_eat('[');
			node->Indices.push_back(m_expr());
			m_eat(']');
		}

		Node* ret = node;
		Node* ext = m_extidnt(node);
		if (ext != nullptr) ret = ext;

		return ret;
	}
	Node* Parser::m_member(Node* parent)
	{
		m_eat('.');

		MemberAccessNode* node = (MemberAccessNode*)m_allocateNode<MemberAccessNode>();
		node->Object = parent;
		memcpy(node->Field, m_token.GetIdentifier(), 256);

		m_eat(TokenType_Identifier);

		Node* ret = node;
		Node* ext = m_extidnt(node);
		if (ext != nullptr) ret = ext;

		return ret;
	}
	Node* Parser::m_extidnt(Node* parent)
	{
		if (m_isToken('.'))
			return m_member(parent);
		if (m_isToken('['))
			return m_array(parent);

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
	Node* Parser::m_factor()
	{
		Node* ret = nullptr;
		if (m_isToken('+') || m_isToken('-')) {
			int operatorType = m_token.GetTokenType();
			m_eat(operatorType);
			UnaryExpressionNode* uOp = (UnaryExpressionNode*)m_allocateNode<UnaryExpressionNode>();
			uOp->Operator = operatorType;
			uOp->Child = m_factor();
			uOp->IsPost = false;
			ret = uOp;
		} else if (m_isToken(TokenType_IntegerLiteral)) {
			IntegerLiteralNode* node = (IntegerLiteralNode*)m_allocateNode<IntegerLiteralNode>();
			node->Value = m_token.GetIntValue();
			m_eat(TokenType_IntegerLiteral);
			ret = node;
		} else if (m_isToken(TokenType_Identifier) || m_isToken(TokenType_Increment) || m_isToken(TokenType_Decrement)) {
			ret = m_identf();
		} else if (m_isToken('(')) {
			m_eat('(');
			ret = m_expr();
			m_eat(')');

			if (m_isToken('.'))
				ret = m_member(ret);
		}

		return ret;
	}
	Node* Parser::m_term()
	{
		Node* node = m_factor();
		
		while (m_isToken('*') || m_isToken('/') || m_isToken('%')) {
			int operatorType = m_token.GetTokenType();
			m_eat(operatorType);
			
			BinaryExpressionNode* binNode = (BinaryExpressionNode*)m_allocateNode<BinaryExpressionNode>();
			binNode->Left = node;
			binNode->Operator = operatorType;
			binNode->Right = m_factor();

			node = (Node*)binNode;
		}

		return node;
	}
	Node* Parser::m_expr()
	{
		Node* node = m_term();

		while (m_isToken('+') || m_isToken('-')) {
			int operatorType = m_token.GetTokenType();

			if (operatorType == '+')
				m_eat('+');
			else if (operatorType == '-')
				m_eat('-');

			BinaryExpressionNode* binNode = (BinaryExpressionNode*)m_allocateNode<BinaryExpressionNode>();
			binNode->Left = node;
			binNode->Operator = operatorType;
			binNode->Right = m_term();

			node = (Node*)binNode;
		}

		return node;
	}
	Node* Parser::m_identf()
	{
		// prefix increment / decrement
		int operatorType = -1;
		if (m_isToken(TokenType_Increment) || m_isToken(TokenType_Decrement)) {
			operatorType = m_token.GetTokenType();
			m_eat(operatorType);
		}

		// cache identifier
		char identifier[256];
		memcpy(identifier, m_token.GetIdentifier(), 256);
		m_eat(TokenType_Identifier);

		// function call
		if (m_isToken('(')) {
			if (operatorType > 0) {
				m_error = "lvalue required";
				m_hasError = true;
			}

			return m_fcall(identifier);
		}

		// extended
		IdentifierNode* node = (IdentifierNode*)m_allocateNode<IdentifierNode>();
		memcpy(node->Name, identifier, 256);
		Node* ret = node;
		Node* ext = m_extidnt(node);
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