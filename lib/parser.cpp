#include "parser.hpp"
#include "diagnostic.hpp"
#include "lexer.hpp"
#include "macros.hpp"
#include <tl/expected.hpp>

namespace de_double_bracket {
    struct Node;
    using Debracketed = diag::WithInfo<std::variant< //
        token::OpenBracket,                          //
        token::CloseBracket,                         //
        token::Number,                               //
        token::Assign,                               //
        token::OperatorBinary,                       //
        token::OperatorUnary,                        //
        Node>>;
    using Element = std::vector<Debracketed>;
    struct Node {
        NON_COPIABLE(Node)

        std::vector<Element> elements;
        explicit Node(std::vector<Element>&& elements) : elements(std::move(elements)) {}
    };

    std::optional<Node> parse(std::span<token::Token> tokens, diag::Diags& diags) {

        auto elements_stack = std::vector<std::vector<Element>>();
        elements_stack.emplace_back();
        elements_stack.back().emplace_back();

        auto open_b_ranges = std::vector<diag::Range>();

        for (int i{0}; i < tokens.size(); i++) {
            auto early_return = std::visit(
                [&]<typename T>(T&& t) -> bool {
                    static_assert(std::is_same_v<T, std::decay_t<T>>);
                    if constexpr (std::is_same_v<T, token::OpenBracket2>) {
                        open_b_ranges.push_back(tokens[i].range);
                        elements_stack.emplace_back();
                        elements_stack.back().emplace_back();
                    } else if constexpr (std::is_same_v<T, token::CloseBracket2>) {
                        if (!open_b_ranges.empty()) {
                            auto elements = std::move(elements_stack.back());
                            elements_stack.pop_back();
                            if (elements.back().empty()) {
                                // For tailing ;
                                elements.pop_back();
                            }
                            auto start_range = open_b_ranges.back();
                            open_b_ranges.pop_back();
                            elements_stack.back().back().push_back(
                                {.range{diag::Range{.start{start_range.start},
                                                    .end{tokens[i].range.end}}},
                                 .t{Node{std::move(elements)}}});
                        } else {
                            for (; i < tokens.size(); i++) {
                                std::visit(
                                    [&]<typename T2>(const T2&) {
                                        static_assert(std::is_same_v<T2, std::decay_t<T2>>);
                                        if constexpr (std::is_same_v<T2, token::CloseBracket2>) {
                                            diags.push_back(diag::Diagnostic{
                                                .level{diag::error},
                                                .range{tokens[i].range},
                                                .message{R"(Can not find matching "((")"}});
                                        }
                                    },
                                    tokens[i].t);
                            }
                            return true;
                        }
                    } else if constexpr (std::is_same_v<T, token::Semicolon>) {
                        if (elements_stack.back().back().empty()) {
                            diags.push_back(diag::Diagnostic{.level{diag::warning},
                                                             .range{tokens[i].range},
                                                             .message{"Extra ';' found"}});
                        } else {
                            elements_stack.back().emplace_back();
                        }
                    } else if constexpr (std::is_same_v<T, token::Comment>) {
                        // do nothing
                    } else if constexpr (std::is_same_v<T, token::OpenBracket> ||
                                         std::is_same_v<T, token::CloseBracket> ||
                                         std::is_same_v<T, token::Number> ||
                                         std::is_same_v<T, token::Assign> ||
                                         std::is_same_v<T, token::OperatorBinary> ||
                                         std::is_same_v<T, token::OperatorUnary>) {
                        elements_stack.back().back().push_back(
                            Debracketed{.range{tokens[i].range}, .t{std::forward<T>(t)}});
                    } else {
                        static_assert(false, "Not exhaustive");
                    }
                    return false;
                },
                std::move(tokens[i].t));
            if (early_return) {
                return std::nullopt;
            }
        }
        if (!open_b_ranges.empty()) {
            diags.reserve(open_b_ranges.size());
            for (const auto& range : open_b_ranges) {
                diags.push_back(diag::Diagnostic{.level{diag::error},
                                              .range{range},
                                              .message{R"range(Can not find matching "))")range"}});
            }
            return std::nullopt;
        } else {
            assert(elements_stack.size() == 1);
            if (elements_stack.back().back().empty()) {
                // For tailing ;
                elements_stack.back().pop_back();
            }
            return Node{std::move(elements_stack.back())};
        }
    }
} // namespace de_double_bracket

// namespace de_single_bracket {
//     struct Node;
//     using Debracketed = diag::WithInfo<std::variant< //
//         token::Number,                               //
//         token::Assign,                               //
//         token::OperatorBinary,                       //
//         token::OperatorUnary,                        //
//         Node>>;
//     struct Node {
//         NON_COPIABLE(Node)
//
//         std::vector<Debracketed> childrens;
//         explicit Node(std::vector<Debracketed>&& childrens) : childrens(std::move(childrens)) {}
//     };
//
//     diag::ExpectedV<std::vector<Debracketed>>
//     parse(const std::vector<de_double_bracket::Debracketed>& tokens) {
//         auto debracketed = std::vector<std::vector<Debracketed>>{};
//         debracketed.emplace_back();
//
//         for (const auto& token : tokens) {
//             std::visit(
//                 [&]<typename T>(const T& t) {
//
//                 },
//                 token.t);
//         }
//
//         return std::move(debracketed[0]);
//     }
// } // namespace de_single_bracket

// namespace to_ast {
//     diag::ExpectedV<ast::Node<ast::Any>>
//     parse_element(const std::vector<de_double_bracket::Debracketed> tokens) {}
//
//     diag::ExpectedV<ast::Node<ast::Array>>
//     parse_array(const diag::WithInfo<de_double_bracket::Node>& node) {
//         ast::Array
//         std::vector<diag::Diagnostic> ds{};
//         for (const auto& element : node.t.elements) {
//             auto e = parse_element(element);
//             if (e) {
//
//             } else {
//
//             }
//         }
//     }
// } // namespace to_ast

std::optional<ast::Array> parse(std::span<token::Token> tokens, diag::Diags& diags) {
    auto d_nodes_i = de_double_bracket::parse(tokens, diags);
    if (!d_nodes_i) {
        return std::nullopt;
    }
    auto d_nodes = std::move(*d_nodes_i);
    auto range = diag::Range{};
    for (const auto& e : d_nodes.elements) {
        for (const auto& t : e) {
            range.start = t.range.start;
        }
    }
    for (const auto& e : d_nodes.elements) {
        for (const auto& t : e) {
            range.start = t.range.start;
        }
    }

    return ast::Array{};
    // return to_ast::parse_array({.range{}, .t{std::move(d_nodes)}})
    //     .map([](ast::Node<ast::Array> a) -> ast::Array { return std::move(*a.t.release()); });
}
