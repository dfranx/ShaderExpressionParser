#include "Tokenizer.h"
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

namespace expr
{
    Tokenizer::Tokenizer(const char* buffer, size_t bufLength)
    {
        m_curIdentifier[0] = 0;
        m_curType = 0;
        m_floatValue = 0.0f;
        m_intValue = 0;
        m_buffer = buffer;
        m_bufferEnd = buffer + bufLength;
    }

    bool Tokenizer::Next()
    {
        if (m_buffer >= m_bufferEnd || *m_buffer == '\0') {
            m_curType = -1;
            return false;
        }

        // skip white space
        while (m_buffer != m_bufferEnd && isspace(m_buffer[0]))
            m_buffer++;

        // two character operators
        if (m_buffer[0] == '=' && m_buffer[1] == '=') {
            m_curType = TokenType_Equal;
            m_buffer += 2;
            return true;
        } else if (m_buffer[0] == '!' && m_buffer[1] == '=') {
            m_curType = TokenType_NotEqual;
            m_buffer += 2;
            return true;
        } else if (m_buffer[0] == '<') {
            if (m_buffer[1] == '=') {
                m_curType = TokenType_LessEqual;
                m_buffer += 2;
                return true;
            }
            else if (m_buffer[1] == '<') {
                m_curType = TokenType_BitshiftLeft;
                m_buffer += 2;
                return true;
            }
        } else if (m_buffer[0] == '>') {
            if (m_buffer[1] == '=') {
                m_curType = TokenType_GreaterEqual;
                m_buffer += 2;
                return true;
            }
            else if (m_buffer[1] == '>') {
                m_curType = TokenType_BitshiftRight;
                m_buffer += 2;
                return true;
            }
        } else if (m_buffer[0] == '+' && m_buffer[1] == '+') {
            m_curType = TokenType_Increment;
            m_buffer += 2;
            return true;
        } else if (m_buffer[0] == '-' && m_buffer[1] == '-')  {
            m_curType = TokenType_Decrement;
            m_buffer += 2;
            return true;
        } else if (m_buffer[0] == '&' && m_buffer[1] == '&') {
            m_curType = TokenType_LogicAnd;
            m_buffer += 2;
            return true;
        } else if (m_buffer[0] == '|' && m_buffer[1] == '|') {
            m_curType = TokenType_LogicOr;
            m_buffer += 2;
            return true;
        }

        // numbers
        if (m_readNumber())
            return true;

        // symbols
        if (m_isSymbol(m_buffer[0])) {
            m_curType = (int)m_buffer[0];
            m_buffer++;
            return true;
        }

        // identifier
        const char* identifierStart = m_buffer;
        while (m_buffer < m_bufferEnd && m_buffer[0] != 0 && !isspace(m_buffer[0]) && !m_isSymbol(m_buffer[0]))
            m_buffer++;

        size_t identifierLength = m_buffer - identifierStart;
        memcpy(m_curIdentifier, identifierStart, identifierLength);
        m_curIdentifier[identifierLength] = 0;
        m_curType = TokenType_Identifier;

        if (strcmp(m_curIdentifier, "true") == 0) {
            m_curType = TokenType_BooleanLiteral;
            m_intValue = 1;
            m_floatValue = 1.0f;
        } else if (strcmp(m_curIdentifier, "false") == 0) {
            m_curType = TokenType_BooleanLiteral;
            m_intValue = 0;
            m_floatValue = 0.0f;
        }

        return true;
    }
    bool Tokenizer::m_isSymbol(char c)
    {
        switch (c)
        {
        case '+': case '-':
        case '*': case '/':
        case '%':
        case '<': case '>':
        case '(': case ')':
        case '[': case ']':
        case '{': case '}':
        case ';': case '!':
        case ',': case '.':
        case '|': case '&': case '^': case '~':
            return true;
        }
        return false;
    }
    bool Tokenizer::m_isValidNumberEnd(char c)
    {
        return c == 0 || isspace(c) || m_isSymbol(c);
    }
    bool Tokenizer::m_readNumber()
    {
        if (m_buffer[0] == '+' || m_buffer[0] == '-')
            return false;

        // hex value
        if (m_bufferEnd - m_buffer > 2 && m_buffer[0] == '0' && m_buffer[1] == 'x') {
            char* literalEnd = nullptr;
            m_intValue = strtol(m_buffer + 2, &literalEnd, 16);
            if (m_isValidNumberEnd(*literalEnd)) {
                m_curType = TokenType_IntegerLiteral;
                m_buffer = literalEnd;
                return true;
            }
        }

        // float value
        char* fEnd = nullptr;
        m_floatValue = strtof(m_buffer, &fEnd);
        if (m_buffer == fEnd) return false;
        if (fEnd[0] == 'f')
            ++fEnd;

        // int value
        char* iEnd = nullptr;
        m_intValue = strtol(m_buffer, &iEnd, 10);


        if (fEnd > iEnd && m_isValidNumberEnd(fEnd[0])) {
            m_curType = TokenType_FloatLiteral;
            m_buffer = fEnd;
            return true;
        } else if (iEnd > m_buffer && m_isValidNumberEnd(iEnd[0])) {
            m_curType = TokenType_IntegerLiteral;
            m_buffer = iEnd;
            return true;
        }

        return false;
    }
}