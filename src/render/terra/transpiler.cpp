#include <lunar/render/terra/token.hpp>
#include <lunar/render/terra/parser.hpp>
#include <lunar/render/terra/transpiler.hpp>
#include <lunar/utils/stopwatch.hpp>
#include <lunar/exp/utils/lexer.hpp>
#include <lunar/debug.hpp>

namespace Terra
{
	bool transpileCode(const Fs::Path& path, TranspilerOutput output)
	{
		auto stopwatch = Utils::Stopwatch(true);

		auto lexer = Utils::Exp::LexerBuilder()
			.appendTextFile(path)
			.create();

		auto tokens = std::vector<Token> {};

		Terra::scanShaderSource(lexer, tokens);

		auto parser = imp::Parser(tokens);
		const auto& ast = parser.run();
		for (const auto& stmt : ast)
			DEBUG_LOG("{}", stmt->toString());

		stopwatch.end();
		return true;
	}
}
