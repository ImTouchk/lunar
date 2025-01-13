#pragma once
#include <lunar/api.hpp>
#include <lunar/file/filesystem.hpp>
#include <lunar/render/terra/statement.hpp>
#include <lunar/render/terra/parser.hpp>
#include <lunar/exp/utils/token.hpp>
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
				{ t.name } -> IsAnyOf<Utils::Exp::Token, Utils::Exp::Token&>;
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
			TranspilerData(std::vector<Statement>& rootScope);
			
			std::vector<Statement>&       rootScope;
			size_t                        charsSinceLastBr = 0;
			std::string                   output           = "";
			std::string                   errorOutput      = "";
			size_t                        inLayoutCount    = 0;
			size_t                        outLayoutCount   = 0;
			size_t                        currentDepth     = 0;


			void beginScope();
			void endScope();
			void writeToOutput(const std::string_view& text);
			void writeError(const Statement& at, const std::string& message);
			void writeError(const Expression& at, const std::string& message);
		};

		enum class LUNAR_API NativeType
		{
			eUnknown = 0,
			eInt,
			eFloat,
			eBool,
			eVec3,
			eVec4
		};

		NativeType toNativeType(const std::string_view& name);
		

		struct LUNAR_API StaticAnalyzer
		{
			StaticAnalyzer(TranspilerData& data);

			StaticAnalyzer& run();

			TranspilerData& data;
		};

		LUNAR_API typedef void (*PFN_StatementVisitor)(TranspilerData&, Statement&);
		LUNAR_API typedef void (*PFN_ExpressionVisitor)(TranspilerData&, Expression&);

		template<typename T>
		using VisitorMap = std::unordered_map<size_t, T>;

		LUNAR_API bool isNativeType(const std::string_view& name);
		LUNAR_API bool _vkGlsl_transpileCode(TranspilerData& data);
	}
}
