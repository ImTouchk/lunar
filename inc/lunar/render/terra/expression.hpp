#pragma once
#include <lunar/api.hpp>
#include <lunar/render/terra/token.hpp>
#include <concepts>

namespace Terra::imp
{
	struct LUNAR_API Expression_T
	{
		virtual std::string toString() const = 0;

		template<typename T> requires std::derived_from<T, Expression_T>
		inline bool isOfType()
		{
			return typeid(*this).hash_code() == typeid(T).hash_code();
		}
	};

	using Expression = std::shared_ptr<Expression_T>;

	struct LUNAR_API BinaryExpr : Expression_T
	{
		BinaryExpr(Expression& left, const Token& op, Expression& right) : left(left), op(op), right(right) {}

		Expression left;
		Token op;
		Expression right;


		std::string toString() const override
		{
			return std::format("{} {} {}", left->toString(), op.toString(), right->toString());
		}
	};

	struct LUNAR_API GroupingExpr : Expression_T
	{
		GroupingExpr(Expression& expr) : expr(expr) {}

		Expression expr;

		std::string toString() const override { return std::format("({})", expr->toString()); }
	};

	struct LUNAR_API LiteralExpr : Expression_T
	{
		LiteralExpr(const Token& value) : value(value) {}

		Token value;

		std::string toString() const override { return value.toString(); }
	};

	struct LUNAR_API UnaryExpr : Expression_T
	{
		UnaryExpr(const Token& op, Expression& right) : op(op), right(right) {}

		Token op;
		Expression right;

		std::string toString() const override { return std::format("{}{}", op.toString(), right->toString()); }
	};

	struct LUNAR_API AssignmentExpr : Expression_T
	{
		AssignmentExpr(const Token& name, Expression& value) : name(name), value(value) {}

		Token name;
		Expression value;

		std::string toString() const override { return std::format("{} = {}", name.toString(), value->toString()); }
	};

	struct LUNAR_API LogicalExpr : Expression_T
	{
		LogicalExpr(Expression left, const Token& op, Expression right) : left(left), op(op), right(right) {}

		Expression left;
		Token op;
		Expression right;

		std::string toString() const override { return std::format("{} {} {}", left->toString(), op.toString(), right->toString()); }
	};

	struct LUNAR_API CallExpr : Expression_T
	{
		CallExpr() {}

		Expression callee;
		std::vector<Expression> args;

		std::string toString() const override
		{
			std::string str = std::format("{}(", callee->toString());
			for (auto& expr : args)
				str += expr->toString() + ",";
			str += ")";
			return str;
		}
	};

	struct LUNAR_API GetExpr : Expression_T
	{
		GetExpr(Expression& obj, const Token& name) : object(obj), name(name) {}

		Expression object;
		Token name;

		std::string toString() const override
		{
			return std::format("{}.{}", object->toString(), name.toString());
		}
	};

	struct LUNAR_API SetExpr : Expression_T
	{
		SetExpr(Expression& obj, const Token& name, Expression& value) : object(obj), name(name), value(value) {}

		Expression object;
		Token name;
		Expression value;

		std::string toString() const override
		{
			return std::format("{}.{} = {}", object->toString(), name.toString(), value->toString());
		}
	};
}
