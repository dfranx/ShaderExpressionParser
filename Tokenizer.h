#pragma once

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
        TokenType_Float4x3,
        TokenType_Float4x2,
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
        TokenType_LessEqual,
        TokenType_GreaterEqual,
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
        Tokenizer(const char* buffer, size_t bufLength);

        bool Next();

        inline int GetTokenType() { return m_curType; }
        inline float GetFloatValue() { return m_floatValue; }
        inline int GetIntValue() { return m_intValue; }

        inline const char* GetIdentifier() { return m_curIdentifier; }

    private:
        bool m_isSymbol(char c);
        bool m_isValidNumberEnd(char c);
        bool m_readNumber();

	private:
        char m_curIdentifier[256];
        int m_curType;

        float m_floatValue;
        int m_intValue;

        const char* m_buffer;
        const char* m_bufferEnd;
	};
}