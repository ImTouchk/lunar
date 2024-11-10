#pragma once
#include <lunar/api.hpp>
#include <lunar/render/terra/token.hpp>
#include <lunar/render/terra/expression.hpp>
#include <lunar/render/terra/statement.hpp>

namespace Terra::imp
{
	class LUNAR_API Parser
	{
	public:
		Parser(const std::vector<Token>& tokens);

		[[nodiscard]] const std::vector<Statement>& run();

	private:
		Expression term();
		Expression expression();
		Expression unary();
		Expression factor();
		Expression primary();
		Expression equality();
		Expression assignment();
		Expression comparison();
		Expression orExpr();
		Expression andExpr();
		Expression finishCall(Expression& callee);
		Expression callExpr();

		Statement expressionStatement();
		Statement blockStatement();
		Statement returnStatement();
		Statement ifStatement();
		Statement whileStatement();
		Statement forStatement();
		Statement statement();

		Statement variableDeclaration(const Token& type);
		Statement functionDeclaration(const Token* _name = nullptr);
		Statement structDeclaration();
		Statement declaration();

		bool isAtEnd();
		const Token& peek();
		const Token& previous();
		const Token& advance();
		bool check(TokenType type);
		bool match(const std::initializer_list<TokenType>& types);
		const Token& consume(TokenType type, const char* message);


	private:
		const std::vector<Token>& tokens;
		std::vector<Statement> stmts;
		size_t current = 0;
	};
}
