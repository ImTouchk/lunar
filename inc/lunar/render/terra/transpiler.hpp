#pragma once
#include <lunar/api.hpp>
#include <lunar/file/filesystem.hpp>
#include <lunar/render/terra/statement.hpp>
#include <lunar/render/terra/parser.hpp>
#include <vector>

namespace Terra
{
	enum class LUNAR_API TranspilerOutput
	{
		eVulkanGLSL
	};

	bool LUNAR_API transpileCode(const Fs::Path& path, TranspilerOutput output);

	namespace imp
	{
		template<typename T, typename... U>
		concept IsAnyOf = (std::same_as<T, U> || ...);

		template<typename T> 
		concept IsNamedStatementType = 
			std::derived_from<T, Statement_T> && 
			requires(T t) {
				{ t.name } -> IsAnyOf<Token, Token&>;
			};

		inline size_t getTypeHash(const Expression& expr) { return typeid(*expr).hash_code(); }
		inline size_t getTypeHash(const Statement& stmt) { return typeid(*stmt).hash_code(); }

		template<typename T> 
		inline size_t getTypeHash() { return typeid(T).hash_code(); }

		template<typename T>
		inline void getAllStatementsOfType(const std::vector<Statement>& input, std::vector<Statement>& output)
		{
			for (auto& stmt : input)
				if (getTypeHash(stmt) == getTypeHash<T>())
					output.emplace_back(stmt);
		}

		template<typename T> requires std::derived_from<T, Expression_T>
		inline const T& doTypeCast(const Expression& expr)
		{
			return *static_cast<const T*>(expr.get());
		}

		template<typename T> requires std::derived_from<T, Expression_T>
		inline T& doTypeCast(Expression& expr)
		{
			return *static_cast<T*>(expr.get());
		}

		template<typename T> requires std::derived_from<T, Statement_T>
		inline const T& doTypeCast(const Statement& stmt)
		{
			return *static_cast<const T*>(stmt.get());
		}

		template<typename T> requires std::derived_from<T, Statement_T>
		inline T& doTypeCast(Statement& stmt)
		{
			return *static_cast<T*>(stmt.get());
		}

		struct LUNAR_API TranspilerData
		{
			TranspilerData(const std::vector<Statement>& rootScope);
			
			size_t                        charsSinceLastBr = 0;
			std::string                   output           = "";
			size_t                        inLayoutCount    = 0;
			size_t                        outLayoutCount   = 0;
			size_t                        currentDepth     = 0;

			const std::vector<Statement>& rootScope;

			std::vector<Statement>        globalFunctions  = {};
			std::vector<Statement>        globalStructures = {};
			std::vector<Statement>        globalVariables  = {};

			std::vector<Statement>        scopeFunctions   = {};
			std::vector<Statement>        scopeStructures  = {};
			std::vector<Statement>        scopeVariables   = {};

			void beginScope();
			void endScope();
			void writeToOutput(const std::string_view& text);

			StructStmt* getStructure(const std::string_view& name);
			FnStmt* getFunction(const std::string_view& name);
			VarStmt* getVariable(const std::string_view& name);
			std::string_view getIdentifierReturnType(const std::string_view& name);
			std::string_view getIdentifierType(const std::string_view& name);
			std::string_view getVariableType(const VarStmt& var);
			std::string_view getFnReturnType(const FnStmt& fn);
			std::string_view getExpressionType(const Expression& expr);

			void loadLocalScope(const std::vector<Statement>& localScope);
			void clearLocalScope();

			template<typename T> requires IsNamedStatementType<T>
			T* searchInScopes
			(
				const std::string_view& name,
				const std::vector<Statement>& global,
				const std::vector<Statement>& local
			)
			{
				for (auto& stmt : local)
				{
					auto* casted_stmt = static_cast<T*>(stmt.get());
					if (casted_stmt->name == name)
						return casted_stmt;
				}

				for (auto& stmt : global)
				{
					auto* casted_stmt = static_cast<T*>(stmt.get());
					if (casted_stmt->name == name)
						return casted_stmt;
				}

				return nullptr;
			}
		};

		LUNAR_API bool isNativeType(const std::string_view& name);
		LUNAR_API bool _vkGlsl_transpileCode(TranspilerData& data);
	}
}
