#include "parser.hpp"
#include "diagnostic.hpp"
#include "lexer.hpp"
#include "macros.hpp"
#include <tl/expected.hpp>

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
        NON_COPIABLE(Node)

        std::vector<std::vector<Debracketed>> elements;
        explicit Node(std::vector<std::vector<Debracketed>>&& elements)
            : elements(std::move(elements)) {}
    };

    diagnostic::ExpectedV<std::pair<Node, int>>
    parse(std::span<token::Token> tokens, std::vector<diagnostic::Range>& open_b_ranges) {
        std::vector<std::vector<Debracketed>> elements{};
        elements.emplace_back();
        for (int i{0}; i < tokens.size(); i++) {
            auto r = std::visit(
                [&]<typename T>(
                    const T& t) -> std::optional<diagnostic::ExpectedV<std::pair<Node, int>>> {
                    if constexpr (std::is_same_v<T, token::OpenBracket2>) {
                        open_b_ranges.push_back(tokens[i].range);
                        auto node_i = parse(tokens.subspan(i + 1), open_b_ranges);
                        if (!node_i) {
                            return tl::unexpected(node_i.error());
                        } else {
                            auto node = std::move(node_i.value().first);
                            auto new_i = i + node_i.value().second + 1;
                            elements.back().push_back(
                                {.range{diagnostic::Range{.start{tokens[i].range.start},
                                                          .end{tokens[new_i].range.end}}},
                                 .t{std::move(node)}});
                            i = new_i;
                        }
                    } else if constexpr (std::is_same_v<T, token::CloseBracket2>) {
                        if (open_b_ranges.empty()) {
                            std::vector<diagnostic::Diagnostic> ds;
                            for (; i < tokens.size(); i++) {
                                std::visit(
                                    [&]<typename T2>(const T2&) {
                                        static_assert(std::is_same_v<T2, std::decay_t<T2>>);
                                        if constexpr (std::is_same_v<T2, token::CloseBracket2>) {
                                            ds.push_back(diagnostic::Diagnostic{
                                                .range{tokens[i].range},
                                                .message{R"(Can not find matching "((")"}});
                                        }
                                    },
                                    tokens[i].t);
                            }
                            return tl::unexpected(ds);
                        } else {
                            open_b_ranges.pop_back();
                            return std::make_pair(Node{std::move(elements)}, i);
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
                if (*r) {
                    auto& elements = (*r)->first.elements;
                    if (elements.back().empty()) {
                        elements.pop_back();
                    }
                }
                return std::move(*r);
            }
        }
        if (!open_b_ranges.empty()) {
            std::vector<diagnostic::Diagnostic> ds;
            ds.reserve(open_b_ranges.size());
            for (const auto& range : open_b_ranges) {
                ds.push_back(diagnostic::Diagnostic{
                    .range{range}, .message{R"range(Can not find matching "))")range"}});
            }
            return tl::unexpected(ds);
        } else {
            if (elements.back().empty()) {
                elements.pop_back();
            }
            return std::make_pair(Node{std::move(elements)}, tokens.size());
        }
    }
} // namespace de_double_bracket

namespace to_ast {
    diagnostic::ExpectedV<ast::Node<ast::Array>>
    parse_array(const diagnostic::WithInfo<de_double_bracket::Node>& node) {}
} // namespace to_ast

diagnostic::ExpectedV<ast::Array> parse(std::span<token::Token> tokens) {
    std::vector<diagnostic::Range> open_b_ranges{};
    auto d_nodes_i = de_double_bracket::parse(tokens, open_b_ranges);
    if (!d_nodes_i) {
        return tl::unexpected(d_nodes_i.error());
    }
    auto d_nodes = std::move(d_nodes_i.value().first);
    return to_ast::parse_array({.range{}, .t{std::move(d_nodes)}})
        .map([](ast::Node<ast::Array> a) -> ast::Array { return std::move(*a.t.release()); });
}
