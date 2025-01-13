#include <lunar/render/terra/parser.hpp>
#include <lunar/render/terra/transpiler.hpp>
#include <lunar/exp/utils/token.hpp>
#include <unordered_map>
#include <string>

namespace Terra::imp
{
	//void _vkGlsl_visitExpression(TranspilerData& data, const Expression& expr);
	//void _vkGlsl_visitStatement(TranspilerData& data, const Statement& stmt);

	//void _vkGlsl_visitLiteralExpr(TranspilerData& data, const Expression& expr)
	//{
	//	auto& literal_expr = doTypeCast<LiteralExpr>(expr);
	//	data.writeToOutput(literal_expr.value.toString());
	//}

	//void _vkGlsl_visitUnaryExpr(TranspilerData& data, const Expression& expr)
	//{
	//	auto& unary_expr = doTypeCast<UnaryExpr>(expr);
	//	data.writeToOutput(unary_expr.op.toString());
	//	_vkGlsl_visitExpression(data, unary_expr.right);
	//}

	//void _vkGlsl_visitAssignmentExpr(TranspilerData& data, const Expression& expr)
	//{
	//	auto& assignment_expr = doTypeCast<AssignmentExpr>(expr);
	//	data.writeToOutput(std::format("{} = ", assignment_expr.name.toString()));
	//	_vkGlsl_visitExpression(data, assignment_expr.value);
	//}

	//void _vkGlsl_visitLogicalExpr(TranspilerData& data, const Expression& expr)
	//{
	//	auto& logical_expr = doTypeCast<LogicalExpr>(expr);
	//	_vkGlsl_visitExpression(data, logical_expr.left);
	//	data.writeToOutput(std::format(" {} ", logical_expr.op.toString()));
	//	_vkGlsl_visitExpression(data, logical_expr.right);
	//}

	//void _vkGlsl_visitCallExpr(TranspilerData& data, const Expression& expr)
	//{
	//	auto& call_expr = doTypeCast<CallExpr>(expr);
	//	_vkGlsl_visitExpression(data, call_expr.callee);
	//	data.writeToOutput("(");
	//	for (size_t i = 0; i < call_expr.args.size(); i++)
	//	{
	//		_vkGlsl_visitExpression(data, call_expr.args[i]);
	//		if (i != call_expr.args.size() - 1)
	//			data.writeToOutput(", ");
	//	}
	//	data.writeToOutput(")");
	//}

	//void _vkGlsl_visitGroupingExpr(TranspilerData& data, const Expression& expr)
	//{
	//	auto& grouping_exp = doTypeCast<GroupingExpr>(expr);
	//	data.writeToOutput("(");
	//	_vkGlsl_visitExpression(data, grouping_exp.expr);
	//	data.writeToOutput(")");
	//}

	//void _vkGlsl_visitBinaryExpr(TranspilerData& data, const Expression& expr)
	//{
	//	auto& binary_expr = doTypeCast<BinaryExpr>(expr);
	//	_vkGlsl_visitExpression(data, binary_expr.left);
	//	data.writeToOutput(std::format(" {} ", binary_expr.op.toString()));
	//	_vkGlsl_visitExpression(data, binary_expr.right);
	//}

	//void _vkGlsl_visitExpression(TranspilerData& data, const Expression& expr)
	//{
	//	static const std::unordered_map<size_t, std::function<void(TranspilerData&, const Expression&)>> visitors =
	//	{
	//		{ getTypeHash<BinaryExpr>(),     _vkGlsl_visitBinaryExpr     },
	//		{ getTypeHash<GroupingExpr>(),   _vkGlsl_visitGroupingExpr   },
	//		{ getTypeHash<LiteralExpr>(),    _vkGlsl_visitLiteralExpr    },
	//		{ getTypeHash<CallExpr>(),       _vkGlsl_visitCallExpr       },
	//		{ getTypeHash<LogicalExpr>(),    _vkGlsl_visitLogicalExpr    },
	//		{ getTypeHash<AssignmentExpr>(), _vkGlsl_visitAssignmentExpr },
	//		{ getTypeHash<UnaryExpr>(),      _vkGlsl_visitUnaryExpr      },
	//	};

	//	auto type_hash = getTypeHash(expr);
	//	if (visitors.contains(type_hash))
	//		visitors.at(type_hash)(data, expr);
	//}

	//void _vkGlsl_visitVarStmt(TranspilerData& data, const Statement& stmt)
	//{
	//	auto& var_stmt       = doTypeCast<VarStmt>(stmt);
	//	auto  var_name       = var_stmt.name.toString();
	//	auto& var_type       = var_stmt.type;
	//	auto  var_value_type = data.getVariableType(var_stmt);

	//	data.writeToOutput(std::format("{} {}", var_value_type, var_name));
	//	if (var_stmt.initializer)
	//	{
	//		data.writeToOutput(" = ");
	//		_vkGlsl_visitExpression(data, var_stmt.initializer);
	//	}
	//	data.writeToOutput(";\n");
	//}

	//void _vkGlsl_visitExprStmt(TranspilerData& data, const Statement& stmt)
	//{
	//	auto& expr_stmt = doTypeCast<ExpressionStmt>(stmt);
	//	_vkGlsl_visitExpression(data, expr_stmt.expr);
	//	data.writeToOutput(";\n");
	//}

	//void _vkGlsl_visitFnStmt(TranspilerData& data, const Statement& stmt)
	//{
	//	auto& fn_stmt     = doTypeCast<FnStmt>(stmt);
	//	auto  fn_name     = fn_stmt.name.toString();
	//	auto  fn_ret_type = data.getFnReturnType(fn_stmt);

	//	data.beginScope();
	//	data.writeToOutput(std::format("{} {}()\n", fn_ret_type, fn_name));
	//	
	//	_vkGlsl_visitStatement(data, fn_stmt.body);

	//	data.endScope();
	//}

	//void _vkGlsl_visitBlockStmt(TranspilerData& data, const Statement& stmt)
	//{
	//	data.beginScope();
	//	data.writeToOutput("{\n");

	//	auto& block_stmt = doTypeCast<BlockStmt>(stmt);
	//	for (auto& stmt : block_stmt.stmts)
	//		_vkGlsl_visitStatement(data, stmt);

	//	data.writeToOutput("}\n");
	//	data.endScope();
	//}

	//void _vkGlsl_visitIfStmt(TranspilerData& data, const Statement& stmt)
	//{
	//	auto& if_stmt = doTypeCast<IfStmt>(stmt);

	//	data.writeToOutput("if (");
	//	_vkGlsl_visitExpression(data, if_stmt.condition);
	//	data.writeToOutput(")");

	//	data.beginScope();
	//	data.writeToOutput("{\n");
	//	_vkGlsl_visitStatement(data, if_stmt.thenBranch);
	//	data.writeToOutput("}\n");
	//	data.endScope();

	//	if (if_stmt.elseBranch != nullptr)
	//	{
	//		data.writeToOutput("else\n");
	//		data.beginScope();
	//		data.writeToOutput("{\n");

	//		_vkGlsl_visitStatement(data, if_stmt.elseBranch);

	//		data.writeToOutput("}\n");
	//		data.endScope();
	//	}
	//}

	//void _vkGlsl_visitStatement(TranspilerData& data, const Statement& stmt)
	//{
	//	static const std::unordered_map<size_t, std::function<void(TranspilerData&, const Statement&)>> visitors =
	//	{
	//		{ getTypeHash<VarStmt>(),        _vkGlsl_visitVarStmt   },
	//		{ getTypeHash<ExpressionStmt>(), _vkGlsl_visitExprStmt  },
	//		{ getTypeHash<FnStmt>(),         _vkGlsl_visitFnStmt    },
	//		{ getTypeHash<BlockStmt>(),      _vkGlsl_visitBlockStmt },
	//		{ getTypeHash<IfStmt>(),         _vkGlsl_visitIfStmt    }
	//	};

	//	auto type_hash = getTypeHash(stmt);
	//	if (visitors.contains(type_hash))
	//		visitors.at(type_hash)(data, stmt);
	//}

	bool _vkGlsl_transpileCode(TranspilerData& data)
	{
		return true;
	}
}