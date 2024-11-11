#include <lunar/render/terra/token.hpp>
#include <lunar/render/terra/parser.hpp>
#include <lunar/render/terra/transpiler.hpp>
#include <lunar/utils/stopwatch.hpp>
#include <lunar/exp/utils/lexer.hpp>
#include <lunar/debug.hpp>
#include <regex>

namespace Terra
{
	bool transpileCode(const Fs::Path& path, TranspilerOutput output)
	{
		auto stopwatch = Utils::Stopwatch(true);

		auto lexer = Utils::Exp::LexerBuilder()
			.appendTextFile(path)
			.create();

		auto tokens = std::vector<Token>{};

		Terra::scanShaderSource(lexer, tokens);

		auto parser = imp::Parser(tokens);
		const auto& ast = parser.run();

		auto data = imp::TranspilerData(ast);
		if (output == TranspilerOutput::eVulkanGLSL)
		{
			imp::_vkGlsl_transpileCode(data);
			DEBUG_LOG("TRANSPILED OUTPUT:\n {}", data.output);
		}


		stopwatch.end();
		return true;
	}

	namespace imp
	{
		TranspilerData::TranspilerData(const std::vector<Statement>& rootScope)
			: rootScope(rootScope)
		{
			getAllStatementsOfType<FnStmt>(rootScope, globalFunctions);
			getAllStatementsOfType<StructStmt>(rootScope, globalStructures);
			getAllStatementsOfType<VarStmt>(rootScope, globalVariables);
		}

		void TranspilerData::loadLocalScope(const std::vector<Statement>& scope)
		{
			getAllStatementsOfType<FnStmt>(scope, scopeFunctions);
			getAllStatementsOfType<StructStmt>(scope, scopeStructures);
			getAllStatementsOfType<VarStmt>(scope, scopeVariables);
		}
		
		StructStmt* TranspilerData::getStructure(const std::string_view& name)
		{
			return searchInScopes<StructStmt>(name, globalStructures, scopeStructures);
		}

		VarStmt* TranspilerData::getVariable(const std::string_view& name)
		{
			return searchInScopes<VarStmt>(name, globalVariables, scopeVariables);
		}

		FnStmt* TranspilerData::getFunction(const std::string_view& name)
		{
			if (isNativeType(name))
				return nullptr;

			return searchInScopes<FnStmt>(name, globalFunctions, scopeFunctions);
		}

		void TranspilerData::clearLocalScope()
		{
			scopeFunctions.clear();
			scopeVariables.clear();
			scopeStructures.clear();
		}

		void TranspilerData::beginScope()
		{
			currentDepth++;
		}

		void TranspilerData::endScope()
		{
			currentDepth--;
		}

		void TranspilerData::writeToOutput(const std::string_view& text)
		{
			constexpr size_t MAX_CHARS_PER_LINE = 80;

			auto processed = std::string(text);
			
			/* Limit column width to MAX_CHARS_PER_LINE */

			// TODO

			/* Maintain indenting */

			auto spaces = std::string(currentDepth * 2, ' ');
			std::regex_replace(processed, std::regex("\n"), "\n" + spaces);
			output.append(processed);
		}

		static const std::vector<const char*> NATIVE_TYPES =
		{
			"Float", "Int", "Bool", "vec2", "vec3", "vec4"
		};

		std::string_view TranspilerData::getIdentifierReturnType(const std::string_view& name)
		{
			auto* var = getVariable(name);
			if (var != nullptr)
				return getVariableType(*var);

			auto* structure = getStructure(name);
			if (structure != nullptr)
				return structure->name.toStringView();

			auto* fn = getFunction(name);
			if (fn != nullptr)
				return getFnReturnType(*fn);

			if (isNativeType(name))
				return name;
		}

		std::string_view TranspilerData::getIdentifierType(const std::string_view& name)
		{
			auto* var = getVariable(name);
			if (var != nullptr)
				return getVariableType(*var);

			auto* structure = getStructure(name);
			if (structure != nullptr)
				return structure->name.toStringView();

			auto* fn = getFunction(name);
			if (fn != nullptr)
				return "fn";

			return "unknown";
		}

		std::string_view TranspilerData::getFnReturnType(const FnStmt& fn)
		{
			return fn.retType.toStringView();
		}

		std::string_view TranspilerData::getExpressionType(const Expression& expr)
		{
			static const std::unordered_map<size_t, std::function<std::string_view(const Expression&)>> visitors =
			{
				{ 
					getTypeHash<BinaryExpr>(),
					[&](const Expression& expr) -> std::string_view {
						return getExpressionType(
							doTypeCast<BinaryExpr>(expr)
								.left
						);
					} 
				},
				{ 
					getTypeHash<GroupingExpr>(),
					[&](const Expression& expr) -> std::string_view {
						return getExpressionType(
							doTypeCast<GroupingExpr>(expr)
								.expr
						);
					}
				},
				{ 
					getTypeHash<CallExpr>(),
					[&](const Expression& expr) -> std::string_view {
						auto& call_expr = doTypeCast<CallExpr>(expr);
						auto callee_type = getExpressionType(call_expr.callee);
						return getIdentifierReturnType(callee_type);
					}
				},
				{ 
					getTypeHash<LogicalExpr>(), 
					[&](const Expression& expr) -> std::string_view { return "Bool"; }
				},
				{
					getTypeHash<AssignmentExpr>(),
					[&](const Expression& expr) -> std::string_view {
						return getExpressionType(
							doTypeCast<AssignmentExpr>(expr)
								.value
						);	
					}
				},
				{
					getTypeHash<UnaryExpr>(),
					[&](const Expression& expr) -> std::string_view {
						return getExpressionType(
							doTypeCast<UnaryExpr>(expr)
								.right
						);
					}
				},
				{ 
					getTypeHash<LiteralExpr>(),
					[&](const Expression& expr) -> std::string_view {
						auto& literal_expr = doTypeCast<LiteralExpr>(expr);
						auto& literal = literal_expr.value;
						switch (literal.type)
						{
						case TokenType::eFalse:
						case TokenType::eTrue: 
							return "Bool";
						case TokenType::eReal:
							return "Float";
						case TokenType::eIdentifier: {
							auto identifier = std::get<std::string_view>(literal.value);
							if (isNativeType(identifier))
								return identifier;
							else
								return getIdentifierType(identifier);
						}
						}
					}
				},
			};

			auto hash = getTypeHash(expr);
			if(visitors.contains(hash))
				return visitors.at(hash)(expr);

			return "unknown";
		}

		std::string_view TranspilerData::getVariableType(const VarStmt& stmt)
		{
			if (!stmt.initializer)
				return "unknown";

			return getExpressionType(stmt.initializer);
		}

		bool isNativeType(const std::string_view& name)
		{
			for (auto& type : NATIVE_TYPES)
				if (name.compare(type) == 0)
					return true;

			return false;
		}
	}
}
