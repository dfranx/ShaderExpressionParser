#pragma once
#include <vector>

namespace expr
{
	enum class NodeType
	{
		None,
		FloatLiteral,
		IntegerLiteral,
		BooleanLiteral,
		Identifier,
		BinaryExpression,
		UnaryExpression,
		Cast,
		FunctionCall,
		MemberAccess,
		ArrayAccess
	};
	enum class ValueType
	{
		Float,
		Float2,
		Float3,
		Float4,
		Float2x2,
		Float3x3,
		Float4x4,
		Float4x3,
		Float4x2,
		Int,
		Int2,
		Int3,
		Int4,
		Uint,
		Uint2,
		Uint3,
		Uint4,
		Bool,
		Bool2,
		Bool3,
		Bool4,
	};
	
	class Node
	{
	public:
		inline virtual NodeType GetNodeType() { return NodeType::None; }
	};
	class FloatLiteralNode : public Node
	{
	public:
		inline virtual NodeType GetNodeType() { return NodeType::FloatLiteral; }
		float Value;
	};
	class IntegerLiteralNode : public Node
	{
	public:
		inline virtual NodeType GetNodeType() { return NodeType::IntegerLiteral; }
		int Value;
	};
	class BooleanLiteralNode : public Node
	{
	public:
		inline virtual NodeType GetNodeType() { return NodeType::BooleanLiteral; }
		bool Value;
	};
	class IdentifierNode : public Node
	{
	public:
		inline virtual NodeType GetNodeType() { return NodeType::Identifier; }
		char Name[256];
	};
	class BinaryExpressionNode : public Node
	{
	public:
		inline virtual NodeType GetNodeType() { return NodeType::BinaryExpression; }
		int Operator;
		Node *Left, *Right;
	};
	class UnaryExpressionNode : public Node
	{
	public:
		inline virtual NodeType GetNodeType() { return NodeType::UnaryExpression; }
		int Operator;
		Node* Child;
		bool IsPost; // for ++ and --
	};
	class FunctionCallNode : public Node
	{
	public:
		inline virtual NodeType GetNodeType() { return NodeType::FunctionCall; }
		char Name[256];
		std::vector<Node*> Arguments;
	};
	class ArrayAccessNode : public Node
	{
	public:
		inline virtual NodeType GetNodeType() { return NodeType::ArrayAccess; }

		Node* Object;
		std::vector<Node*> Indices;
	};
	class MemberAccessNode : public Node
	{
	public:
		inline virtual NodeType GetNodeType() { return NodeType::MemberAccess; }

		Node* Object;
		char Field[256];
	};
	class CastNode : public Node
	{
	public:
		inline virtual NodeType GetNodeType() { return NodeType::Cast; }
		Node* Object;
		int Type;
	};
}