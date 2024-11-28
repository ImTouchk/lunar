#include <lunar/exp/utils/token.hpp>
#include <lunar/render/terra/transpiler.hpp>

namespace Terra::imp
{
	std::string_view toString(NativeType type)
	{
		switch (type)
		{
		case NativeType::eInt:   return "Int";
		case NativeType::eFloat: return "Float";
		case NativeType::eBool:  return "Bool";
		case NativeType::eVec3:  return "Vec3";
		case NativeType::eVec4:  return "Vec4";
		default:                 return "Unknown";
		}
	}

	NativeType toNativeType(const std::string_view& name)
	{
		auto   hash_fn   = std::hash<std::string_view>{};
		size_t name_hash = hash_fn(name);

		static const std::unordered_map<size_t, NativeType> types =
		{
			{ hash_fn("Int"),   NativeType::eInt   },
			{ hash_fn("Float"), NativeType::eFloat },
			{ hash_fn("Bool"),  NativeType::eBool  },
			{ hash_fn("Vec3"),  NativeType::eVec3  },
			{ hash_fn("Vec4"),  NativeType::eVec4  }
		};
		
		if (types.contains(name_hash))
			return types.at(name_hash);
		else
			return NativeType::eUnknown;
	}

	namespace type_checker
	{
		using Utils::Exp::Token;
		using Utils::Exp::TokenType;

		typedef std::string_view (*PFN_ExpressionTypeChecker)(TranspilerData&, Expression&);

		void visitStatement(TranspilerData&, Statement&);
		std::string_view visitExpression(TranspilerData&, Expression&);
		
		NativeType tryTypeUpcast(NativeType a, NativeType b)
		{
			static const std::initializer_list<std::pair<NativeType, NativeType>> compat_types =
			{
				{ NativeType::eInt,   NativeType::eFloat  },
				{ NativeType::eBool,  NativeType::eInt  },
				{ NativeType::eVec3,  NativeType::eVec4 }
			};

			if (a == b)
				return a;

			for (const auto& compat_type : compat_types)
			{
				if(
					(a == compat_type.first && b == compat_type.second) ||
					(b == compat_type.first && a == compat_type.second)
				)
				{
					return compat_type.second;
				}
			}

			return NativeType::eUnknown;
		}

		std::string_view visitLiteralExpression(TranspilerData& data, Expression& expr)
		{
			auto& literal_expr = doTypeCast<LiteralExpr>(expr);
			switch (literal_expr.value.type)
			{
			case TokenType::eInteger: return "Int";
			case TokenType::eReal:    return "Float";
			case TokenType::eTrue:
			case TokenType::eFalse:
									  return "Bool";
			default:				  return "unknown";
			}
		}

		std::string_view visitUnaryExpression(TranspilerData& data, Expression& expr)
		{
			auto& unary_expr = doTypeCast<UnaryExpr>(expr);
			return visitExpression(data, unary_expr.right);
		}

		std::string_view visitGroupingExpression(TranspilerData& data, Expression& expr)
		{
			auto& grouping_expr = doTypeCast<GroupingExpr>(expr);
			return visitExpression(data, grouping_expr.expr);
		}

		std::string_view visitBinaryExpression(TranspilerData& data, Expression& expr)
		{
			auto& binary_expr  = doTypeCast<BinaryExpr>(expr);
			auto  left_ty      = visitExpression(data, binary_expr.left);
			auto  right_ty     = visitExpression(data, binary_expr.right);
			auto  left_native  = toNativeType(left_ty);
			auto  right_native = toNativeType(right_ty);

			if (left_native != NativeType::eUnknown || right_native != NativeType::eUnknown)
			{
				auto upcast_ty = tryTypeUpcast(left_native, right_native);
				if (upcast_ty == NativeType::eUnknown)
				{
					data.writeError(
						expr,
						std::format
						(
							"Cannot perform operation '{}' on types '{}' and '{}'.",
							binary_expr.op.toStringView(), left_ty, right_ty
						)
					);

					return left_ty;
				}
				else
					return toString(upcast_ty);
			}
			else if (left_ty != right_ty)
			{
				data.writeError(
					expr,
					std::format
					(
						"Cannot perform operation '{}' on custom types '{}' and '{}'.",
						binary_expr.op.toStringView(), left_ty, right_ty
					)
				);
			}

			return left_ty;
		}

		std::string_view visitLogicalExpression(TranspilerData& data, Expression& expr)
		{
			auto& logical_expr = doTypeCast<LogicalExpr>(expr);
			auto  left_ty      = visitExpression(data, logical_expr.left);
			auto  right_ty     = visitExpression(data, logical_expr.right);
			// TODO
			return "Bool";
		}

		std::string_view visitCallExpression(TranspilerData& data, Expression& expr)
		{
			auto& call_expr = doTypeCast<CallExpr>(expr);
			return "";
		}

		std::string_view visitExpression(TranspilerData& data, Expression& expr)
		{
			static const VisitorMap<PFN_ExpressionTypeChecker> visitors =
			{
				{ getTypeHash<LiteralExpr>(), visitLiteralExpression },
				{ getTypeHash<UnaryExpr>(),   visitUnaryExpression   },
				{ getTypeHash<GroupingExpr>(),visitGroupingExpression},
				{ getTypeHash<BinaryExpr>(),  visitBinaryExpression  },
				{ getTypeHash<LogicalExpr>(), visitLogicalExpression },
				{ getTypeHash<CallExpr>(),    visitCallExpression    }
			};

			size_t expr_ty = getTypeHash(expr);
			if (visitors.contains(expr_ty))
			{
				auto value = visitors.at(expr_ty)(data, expr);
				DEBUG_LOG("Expression type value: {}", value);
				return value;
			}

			return "not_implemented";
		}

		//void visitFnStatement(TranspilerData& data, Statement& statement)
		//{
		//	auto& fn_stmt  = doTypeCast<FnStmt>(statement);
		//	auto& ret_type = fn_stmt.retType;
		//	
		//	if (ret_type.compare("unknown") == 0)
		//	{
		//		auto& fn_body = doTypeCast<BlockStmt>(fn_stmt.body);
		//		for (auto& stmt : fn_body.stmts)
		//		{
		//			if (stmt->isOfType<ReturnStmt>())
		//			{
		//				auto& ret_stmt = doTypeCast<ReturnStmt>(stmt);
		//				ret_type = data.getExpressionType(ret_stmt.expr);
		//				return;
		//			}
		//		}

		//		throw;
		//	}

		//	
		//}

		void visitExprStatement(TranspilerData& data, Statement& statement)
		{
			auto& expr_stmt = doTypeCast<ExpressionStmt>(statement);
			visitExpression(data, expr_stmt.expr);
		}

		void visitStatement(TranspilerData& data, Statement& statement)
		{
			static const VisitorMap<PFN_StatementVisitor> visitors =
			{
				{ getTypeHash<ExpressionStmt>(), visitExprStatement } 
			};

			size_t statement_ty = getTypeHash(statement);
			if (visitors.contains(statement_ty))
				visitors.at(statement_ty)(data, statement);
		}
	}

	StaticAnalyzer::StaticAnalyzer(TranspilerData& data)
		: data(data)
	{
	}

	StaticAnalyzer& StaticAnalyzer::run()
	{
		for (auto& stmt : data.rootScope)
			type_checker::visitStatement(data, stmt);

		return *this;
	}
}
