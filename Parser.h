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

		inline bool Error() { return m_hasError; }
		inline const std::string& ErrorMessage() { return m_error; }

	private:
		Node* m_factor();
		Node* m_term();
		Node* m_expr();
		Node* m_identf();
		Node* m_extidnt(Node* parent);
		Node* m_fcall(char* fname);
		Node* m_array(Node* parent);
		Node* m_member(Node* parent);

		// ExpressionType m_evaluateType(Node*);

	private:
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