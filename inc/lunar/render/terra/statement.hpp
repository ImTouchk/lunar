#pragma once
#include <lunar/api.hpp>
#include <lunar/exp/utils/token.hpp>
#include <lunar/render/terra/expression.hpp>

namespace Terra::imp
{
	struct LUNAR_API Statement_T
	{
		virtual std::string toString() const = 0;

		template<typename T> requires std::derived_from<T, Statement_T>
		inline bool isOfType()
		{
			return typeid(this).hash_code() == typeid(T).hash_code();
		}
	};

	using Statement = std::shared_ptr<Statement_T>;

	struct LUNAR_API ExpressionStmt : Statement_T
	{
		ExpressionStmt(Expression& expr) : expr(expr) {}

		Expression expr;

		std::string toString() const override { return expr->toString(); }
	};

	struct LUNAR_API VarStmt : Statement_T
	{
		VarStmt(const Token& type, const Token& name, Expression& initializer, bool isMut)
			: type(type), name(name), initializer(initializer), isMutable(isMut) {}

		Token type;
		Token name;
		Expression initializer;
		bool isMutable;

		std::string toString() const override
		{
			return std::format("{} {} = {} (mutable: {})", type.toString(), name.toString(), initializer->toString(), isMutable);
		}
	};

	struct LUNAR_API BlockStmt : Statement_T
	{
		BlockStmt() : stmts() {}

		std::vector<Statement> stmts;

		std::string toString() const override
		{
			std::string str = "BLOCK-BEGIN\n";
			for (const auto& stmt : stmts)
				str += stmt->toString() + "\n";
			str += "BLOCK-END\n";
			return str;
		}
	};

	struct LUNAR_API IfStmt : Statement_T
	{
		IfStmt(Expression& cond, Statement& thenBranch, Statement& elseBranch)
			: condition(cond), thenBranch(thenBranch), elseBranch(elseBranch) {}

		Expression condition;
		Statement thenBranch;
		Statement elseBranch;

		std::string toString() const override
		{
			return std::format("if ({}) then {} else {}", condition->toString(), thenBranch->toString(), elseBranch->toString());
		}
	};

	struct LUNAR_API WhileStmt : Statement_T
	{
		WhileStmt(Expression& cond, Statement& body) : condition(cond), body(body) {}

		Expression condition;
		Statement body;

		std::string toString() const override
		{
			return std::format("while({}) {{ {} }}", condition->toString(), body->toString());
		}
	};

	struct LUNAR_API FnStmt : Statement_T
	{
		struct Parameter
		{
			Token name;
			Token type;
		};

		FnStmt(const Token& name, const std::string_view& retType) : name(name), retType(retType), params(), body() {}

		Token name;
		std::string_view retType;
		std::vector<Parameter> params;
		Statement body;

		std::string toString() const override
		{
			return "";
		}
	};

	struct LUNAR_API ReturnStmt : Statement_T
	{
		ReturnStmt(Expression& expr) : expr(expr) {}

		Expression expr;

		std::string toString() const override
		{
			return std::format("return {};", expr->toString());
		}
	};

	struct LUNAR_API StructStmt : Statement_T
	{
		struct Variable
		{
			Token name;
			Token type;
		};

		StructStmt(const Token& name) : name(name), variables(), methods() {}

		Token name;
		std::vector<Variable> variables;
		std::vector<Statement> methods;

		std::string toString() const override
		{
			std::string str = std::format("struct {} {{\n", name.toString());
			for (auto& var : variables)
				str += std::format("  {}: {}\n", var.name.toString(), var.type.toString());

			str += "------ Methods:\n";
			for (auto& method : methods)
				str += method->toString() + "\n";

			str += "}\n";
			return str;
		}
	};
}
