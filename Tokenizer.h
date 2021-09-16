#pragma once
#include <unordered_map>

namespace expr
{
    enum TokenType
    {
        TokenType_FloatLiteral = 256,
        TokenType_IntegerLiteral,
        TokenType_BooleanLiteral,
        TokenType_Identifier,

        TokenType_Float,
        TokenType_Float2,
        TokenType_Float3,
        TokenType_Float4,
        TokenType_Float2x2,
        TokenType_Float3x3,
        TokenType_Float4x4,
        TokenType_Int,
        TokenType_Int2,
        TokenType_Int3,
        TokenType_Int4,
        TokenType_Uint,
        TokenType_Uint2,
        TokenType_Uint3,
        TokenType_Uint4,
        TokenType_Bool,
        TokenType_Bool2,
        TokenType_Bool3,
        TokenType_Bool4,

        TokenType_Equal,
        TokenType_NotEqual,
        TokenType_LessThanEqual,
        TokenType_GreaterThanEqual,
        TokenType_LogicAnd,
        TokenType_LogicOr,
        TokenType_BitshiftLeft,
        TokenType_BitshiftRight,
        TokenType_Increment,
        TokenType_Decrement,

        TokenTypeCount
    };

	class Tokenizer
	{
	public:
        Tokenizer(const char* buffer, unsigned int bufLength);

        bool Next();
		void Undo();

        inline int GetTokenType() { return m_curType; }
        inline float GetFloatValue() { return m_floatValue; }
        inline int GetIntValue() { return m_intValue; }

        inline const char* GetIdentifier() { return m_curIdentifier; }

    private:
        bool m_isSymbol(char c);
        bool m_isValidNumberEnd(char c);
        bool m_readNumber();

	private:
        std::unordered_map<const char*, TokenType> m_keywords;

        char m_curIdentifier[256];
        int m_curType;

        char m_prevIdentifier[256];
		int m_prevType;
		const char* m_tokenStart;

        float m_floatValue;
        int m_intValue;

        const char* m_buffer;
        const char* m_bufferEnd;
	};
}