#include "parser.hpp"
#include "diagnostic.hpp"
#include "lexer.hpp"
#include <tl/expected.hpp>

using namespace ast;

namespace de_double_bracket {
    struct Node;
    using Debracketed = diagnostic::WithInfo<std::variant<token::OpenBracket,    //
                                                          token::CloseBracket,   //
                                                          token::Number,         //
                                                          token::Assign,         //
                                                          token::OperatorBinary, //
                                                          token::OperatorUnary,  //
                                                          Node>>;
    struct Node {
        std::vector<std::vector<Debracketed>> elements;
    };

    diagnostic::Expected<std::pair<Node, int>> parse(std::span<token::Token> tokens, bool is_root) {
        std::vector<std::vector<Debracketed>> elements{{}};
        for (int i{0}; i < tokens.size(); i++) {
            auto r = std::visit(
                [&]<typename T>(
                    const T& t) -> std::optional<diagnostic::Expected<std::pair<Node, int>>> {
                    if constexpr (std::is_same_v<T, token::OpenBracket2>) {
                        auto node_i = parse(tokens.subspan(i + 1), false);
                        if (!node_i) {
                            return tl::unexpected(node_i.error());
                        }
                        auto node = node_i.value().first;
                        auto new_i = node_i.value().second;
                        elements.back().push_back(
                            {.range{diagnostic::Range{.start{tokens[i].range.start},
                                                      .end{tokens[new_i].range.end}}},
                             .t{node}});
                        i += new_i + 1;
                    } else if constexpr (std::is_same_v<T, token::CloseBracket2>) {
                        if (is_root) {
                            return tl::unexpected(
                                diagnostic::Diagnostic{.range{.start{tokens.back().range.start},
                                                              .end{tokens.back().range.end}},
                                                       .message{"Extra \"))\" without matching \"((\""}});
                        } else {
                            return std::make_pair(Node{.elements{std::move(elements)}}, i);
                        }
                    } else if constexpr (std::is_same_v<T, token::Semicolon>) {
                        elements.emplace_back();
                    } else if constexpr (std::is_same_v<T, token::Comment>) {
                        // do nothing
                    } else if constexpr (std::is_same_v<T, token::OpenBracket> ||
                                         std::is_same_v<T, token::CloseBracket> ||
                                         std::is_same_v<T, token::Number> ||
                                         std::is_same_v<T, token::Assign> ||
                                         std::is_same_v<T, token::OperatorBinary> ||
                                         std::is_same_v<T, token::OperatorUnary>) {
                        elements.back().push_back({.range{tokens[i].range}, .t{t}});
                    } else {
                        static_assert(false, "Not exhaustive");
                    }
                    return std::nullopt;
                },
                tokens[i].t);
            if (r) {
                auto& elements = (*r)->first.elements;
                if (elements.back().empty()) {
                    elements.pop_back();
                }
                return *r;
            }
        }
        if (!is_root) {
            return tl::unexpected(diagnostic::Diagnostic{
                .range{.start{tokens.back().range.end}, .end{tokens.back().range.end}},
                .message{"Missing \"))\""}});
        } else {
            if (elements.back().empty()) {
                elements.pop_back();
            }
            return std::make_pair(Node{.elements{elements}}, tokens.size());
        }
    }
} // namespace de_double_bracket

diagnostic::Expected<Array> parse(std::span<token::Token> tokens) {
    de_double_bracket::parse(tokens, true);
    return Array{};
}
