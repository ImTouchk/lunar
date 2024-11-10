#include <lunar/render/terra/parser.hpp>
#include <lunar/debug.hpp>

namespace Terra::imp
{
	Parser::Parser(const std::vector<Token>& tokens)
		: tokens(tokens),
		stmts(),
		current(0)
	{
	}

	/*
		UTILITY FUNCTIONS
		----------------------
	*/

	const Token& Parser::peek() 
	{ 
		return tokens[current]; 
	}

	const Token& Parser::previous() 
	{
		return tokens[current - 1]; 
	}

	bool Parser::isAtEnd() 
	{ 
		return peek().type == TokenType::eEof; 
	}

	const Token& Parser::advance()
	{
		if (!isAtEnd()) current++;
		return previous();
	}

	bool Parser::check(TokenType type)
	{
		if (isAtEnd()) return false;
		return peek().type == type;
	}

	bool Parser::match(const std::initializer_list<TokenType>& types)
	{
		for (auto& type : types)
		{
			if (check(type))
			{
				advance();
				return true;
			}
		}

		return false;
	}

	const Token& Parser::consume(TokenType type, const char* message)
	{
		if (check(type)) return advance();
		else
		{
			DEBUG_ERROR("Shader parsing error: {}", message);
			throw;
		}
	}

	const std::vector<Statement>& Parser::run()
	{
		try {
			while (!isAtEnd())
				stmts.push_back(declaration());
		}
		catch (...) {
			DEBUG_ERROR("Failed to parse shader file.");
		}

		return stmts;
	}

	/* 
		EXPRESSION PARSING
		-------------------------
	*/

	Expression Parser::expression()
	{
		return assignment();
	}

	Expression Parser::andExpr()
	{
		auto expr = equality();
		while (match({ TokenType::eAnd }))
		{
			const auto& op = previous();
			auto right = equality();
			expr = std::make_shared<LogicalExpr>(expr, op, right);
		}

		return expr;
	}

	Expression Parser::orExpr()
	{
		auto expr = andExpr();
		while (match({ TokenType::eOr }))
		{
			const auto& op = previous();
			auto right = andExpr();
			expr = std::make_shared<LogicalExpr>(expr, op, right);
		}
		return expr;
	}

	Expression Parser::assignment()
	{
		auto expr = orExpr();
		if (match({ TokenType::eEqual }))
		{
			const auto& eq = previous();
			auto value = assignment();

			if (expr->isOfType<GetExpr>())
			{
				auto* get = static_cast<GetExpr*>(expr.get());
				return std::make_shared<SetExpr>(get->object, get->name, value);
			}
			else if (expr->isOfType<LiteralExpr>())
			{
				const auto& name = static_cast<LiteralExpr*>(expr.get())->value;
				return std::make_shared<AssignmentExpr>(name, value);
			}

			DEBUG_ERROR("Invalid assignment target.");
			throw;
		}

		return expr;
	}

	Expression Parser::equality()
	{
		auto expr = comparison();
		while (match({ TokenType::eBangEqual, TokenType::eEqualEqual }))
		{
			const auto& op = previous();
			auto right = comparison();
			expr = std::make_shared<BinaryExpr>(expr, op, right);
		}

		return expr;
	}

	Expression Parser::comparison()
	{
		auto expr = term();
		while (match({ TokenType::eGreater, TokenType::eGreaterEqual, TokenType::eLess, TokenType::eLessEqual }))
		{
			const auto& op = previous();
			auto right = term();
			expr = std::make_shared<BinaryExpr>(expr, op, right);
		}

		return expr;
	}

	Expression Parser::term()
	{
		auto expr = factor();

		while (match({ TokenType::ePlus, TokenType::eMinus }))
		{
			const auto& op = previous();
			auto right = factor();
			expr = std::make_shared<BinaryExpr>(expr, op, right);
		}

		return expr;
	}

	Expression Parser::factor()
	{
		auto expr = unary();

		while (match({ TokenType::eSlash, TokenType::eStar }))
		{
			const auto& op = previous();
			auto right = unary();
			expr = std::make_shared<BinaryExpr>(expr, op, right);
		}

		return expr;
	}

	Expression Parser::unary()
	{
		if (match({ TokenType::eBang, TokenType::eMinus }))
		{
			const auto& op = previous();
			auto right = unary();
			return std::make_shared<UnaryExpr>(op, right);
		}

		return callExpr();
	}

	Expression Parser::finishCall(Expression& callee)
	{
		auto call_expr = std::make_shared<CallExpr>();

		if (!check(TokenType::eRightParen))
		{
			do
			{
				if (call_expr->args.size() >= 255)
				{
					DEBUG_ERROR("Too many arguments inside function call.");
					throw;
				}

				call_expr->args.push_back(expression());
			} while (match({ TokenType::eComma }));
		}

		consume(TokenType::eRightParen, "Expected ')' after function call arguments.");

		call_expr->callee = callee;
		return call_expr;
	}

	Expression Parser::callExpr()
	{
		auto expr = primary();
		while (true)
		{
			if (match({ TokenType::eLeftParen }))
				expr = finishCall(expr);
			else if (match({ TokenType::eDot }))
			{
				const auto& name = consume(TokenType::eIdentifier, "Expected property name after '.'");
				expr = std::make_shared<GetExpr>(expr, name);
			}
			else
				break;
		}
		return expr;
	}

	Expression Parser::primary()
	{
		if (match({ TokenType::eFalse, TokenType::eTrue, TokenType::eReal, TokenType::eIdentifier }))
			return std::make_shared<LiteralExpr>(previous());

		if (match({ TokenType::eLeftParen }))
		{
			auto expr = expression();
			consume(TokenType::eRightParen, "Expected ) after expression");
			return std::make_shared<GroupingExpr>(expr);
		}
	}

	/*
		STATEMENT PARSING
		-------------------------
	*/

	Statement Parser::expressionStatement()
	{
		auto expr = expression();
		consume(TokenType::eSemicolon, "Expected semicolon after expression");
		return std::make_shared<ExpressionStmt>(expr);
	}

	Statement Parser::blockStatement()
	{
		auto stmt = std::make_shared<BlockStmt>();

		while (!check(TokenType::eRightBrace) && !isAtEnd())
			stmt->stmts.push_back(declaration());

		consume(TokenType::eRightBrace, "Expected right brace at the end of block");
		return stmt;
	}

	Statement Parser::returnStatement()
	{
		Expression value = nullptr;
		if (!check(TokenType::eSemicolon))
			value = expression();

		consume(TokenType::eSemicolon, "Expected semicolon after return value.");
		return std::make_shared<ReturnStmt>(value);
	}

	Statement Parser::ifStatement()
	{
		consume(TokenType::eLeftParen, "Expected '(' after 'if' keyword.");
		auto cond = expression();
		consume(TokenType::eRightParen, "Expected ')' after 'if' condition.");

		auto thenBranch = statement();
		Statement elseBranch = nullptr;
		if (match({ TokenType::eElse }))
			elseBranch = statement();

		return std::make_shared<IfStmt>(cond, thenBranch, elseBranch);
	}

	Statement Parser::whileStatement()
	{
		consume(TokenType::eLeftParen, "Expected '(' after 'while' keyword.");
		auto cond = expression();
		consume(TokenType::eRightParen, "Expected ')' after 'while' condition.");
		auto body = statement();

		return std::make_shared<WhileStmt>(cond, body);
	}

	Statement Parser::forStatement()
	{
		consume(TokenType::eLeftParen, "Expected '(' after 'for' keyword.");

		Statement initializer = nullptr;
		if (match({ TokenType::eSemicolon }))
			initializer = nullptr;
		else if (match({ TokenType::eLet }))
			initializer = variableDeclaration(previous());
		else
			initializer = expressionStatement();

		Expression condition = nullptr;
		if (!check(TokenType::eSemicolon))
			condition = expression();

		consume(TokenType::eSemicolon, "Expected ';' after 'for' loop condition.");

		Expression increment = nullptr;
		if (!check(TokenType::eRightParen))
			increment = expression();

		consume(TokenType::eRightParen, "Expected ')' after 'for' loop clauses.");

		auto body = statement();

		if (increment != nullptr)
		{
			auto upper = std::make_shared<BlockStmt>();
			upper->stmts.emplace_back(body);
			upper->stmts.emplace_back(std::make_shared<ExpressionStmt>(increment));
			body = upper;
		}

		if (condition == nullptr)
			condition = std::make_shared<LiteralExpr>(Token(TokenType::eTrue, true));

		body = std::make_shared<WhileStmt>(condition, body);

		if (initializer != nullptr)
		{
			auto upper = std::make_shared<BlockStmt>();
			upper->stmts.emplace_back(body);
			upper->stmts.emplace_back(initializer);
			body = upper;
		}

		return body;
	}

	Statement Parser::statement()
	{
		if (match({ TokenType::eIf }))
			return ifStatement();

		if (match({ TokenType::eFor }))
			return forStatement();

		if (match({ TokenType::eReturn }))
			return returnStatement();

		if (match({ TokenType::eWhile }))
			return whileStatement();

		if (match({ TokenType::eLeftBrace }))
			return blockStatement();

		return expressionStatement();
	}

	Statement Parser::variableDeclaration(const Token& type)
	{
		bool isMutable = false;
		if (match({ TokenType::eMut }))
			isMutable = true;

		const auto& name = consume(TokenType::eIdentifier, "Expected variable name.");
		Expression initializer = nullptr;
		if (match({ TokenType::eEqual }))
			initializer = expression();

		consume(TokenType::eSemicolon, "Expected semicolon after variable declaration.");
		return std::make_shared<VarStmt>(type, name, initializer, isMutable);
	}

	Statement Parser::functionDeclaration(const Token* _name)
	{
		const auto& name = (_name == nullptr)
			? consume(TokenType::eIdentifier, "Expected name after 'fn' keyword.")
			: *_name;

		consume(TokenType::eLeftParen, "Expected '(' at beginning of function declaration.");

		auto fn_stmt = std::make_shared<FnStmt>(name, Token(TokenType::eEof, "void"));
		if (!check(TokenType::eRightParen))
		{
			do
			{
				if (fn_stmt->params.size() >= 255)
				{
					DEBUG_ERROR("Too many arguments inside function declaration.");
					throw;
				}
				const auto& param_name = consume(TokenType::eIdentifier, "Expected parameter name inside function declaration.");
				consume(TokenType::eDoubleColon, "Expected ':' after parameter name inside function declaration.");
				const auto& param_type = consume(TokenType::eIdentifier, "Expected parameter type inside function declaration.");
				fn_stmt->params.push_back({ param_name, param_type });
			} while (match({ TokenType::eComma }));
		}

		consume(TokenType::eRightParen, "Expected end of parameter list inside function declaration.");

		if (!check(TokenType::eLeftBrace))
		{
			consume(TokenType::eDoubleColon, "Expected function return type inside declaration.");
			fn_stmt->retType = consume(TokenType::eIdentifier, "Expected function return type inside declaration.");
		}

		consume(TokenType::eLeftBrace, "Expected beginning of function block.");

		fn_stmt->body = blockStatement();
		return fn_stmt;
	}

	Statement Parser::structDeclaration()
	{
		const auto& name = consume(TokenType::eIdentifier, "Expected struct name.");
		consume(TokenType::eLeftBrace, "Expected '{' before struct body.");

		auto stmt = std::make_shared<StructStmt>(name);
		while (!check(TokenType::eRightBrace) && !isAtEnd())
		{
			const auto& var_name = consume(TokenType::eIdentifier, "Expected variable name inside struct body.");
			consume(TokenType::eDoubleColon, "Expected variable type after name inside struct body.");
			if (match({ TokenType::eFun }))
			{
				auto fn_decl = functionDeclaration(&var_name);
				stmt->methods.push_back(fn_decl);
			}
			else
			{
				const auto& var_type = consume(TokenType::eIdentifier, "Expected variable type after name inside struct body.");
				stmt->variables.push_back({ var_name, var_type });
			}

			if (match({ TokenType::eComma }))
				continue;
			else
				break;
		}
		consume(TokenType::eRightBrace, "Expected end of struct declaration.");
		consume(TokenType::eSemicolon, "Expected end of struct declaration.");
		return stmt;
	}

	Statement Parser::declaration()
	{
		if (match({ TokenType::eStruct }))
			return structDeclaration();

		if (match({ TokenType::eFun }))
			return functionDeclaration();

		if (match({ TokenType::eLet, TokenType::eUniform, TokenType::eInput, TokenType::eOutput }))
			return variableDeclaration(previous());

		return statement();
	}
}
