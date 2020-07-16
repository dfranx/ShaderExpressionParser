#pragma once
#include "Tokenizer.h"
#include "Node.h"

#include <vector>
#include <string>
#include <unordered_map>

namespace expr
{
	class Parser
	{
	public:
		Parser(const char* buffer, size_t bufLength);
		Node* Parse();

		void Clear();
		std::vector<Node*>& GetList() { return m_list; }

		inline bool Error() { return m_hasError; }
		inline const std::string& ErrorMessage() { return m_error; }

	private:
		Node* m_parseValue();
		Node* m_parseTernaryExpression();
		Node* m_parseExpression(int precedence);
		Node* m_parseIdentifier();
		Node* m_parseExtIdentifier(Node* parent);
		Node* m_parseFunctionCall(char* fname, int tokType);
		Node* m_parseArrayAccess(Node* parent);
		Node* m_parseMemberAccess(Node* parent);
		void m_parseArguments(std::vector<Node*>& args);

	private:
		bool m_isType(int tokenType);
		bool m_isLValue(NodeType nodeType);
		bool m_eat(int tokenType);
		bool m_isToken(int tokenType);

		template<typename T>
		Node* m_allocateNode() {
			m_list.push_back((Node*)new T());
			return m_list.back();
		}

		Tokenizer m_token;

		std::vector<Node*> m_list;

		bool m_hasError;
		std::string m_error;

		std::unordered_map<int, unsigned char> m_opPrecedence;
	};
}