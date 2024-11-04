#include "parser.hpp"
#include "diagnostic.hpp"
#include "lexer.hpp"
#include "macros.hpp"
#include <tl/expected.hpp>

namespace de_double_bracket {
    struct Node;
    using Debracketed = diagnostic::WithInfo<std::variant< //
        token::OpenBracket,                                //
        token::CloseBracket,                               //
        token::Number,                                     //
        token::Assign,                                     //
        token::OperatorBinary,                             //
        token::OperatorUnary,                              //
        Node>>;
    struct Node {
        NON_COPIABLE(Node)

        std::vector<std::vector<Debracketed>> elements;
        explicit Node(std::vector<std::vector<Debracketed>>&& elements)
            : elements(std::move(elements)) {}
    };

    // Fatal error: 
    diagnostic::ExpectedV<std::pair<Node, int>>
    parse(std::span<token::Token> tokens, std::vector<diagnostic::Range>& open_b_ranges) {
        auto elements = std::vector<std::vector<Debracketed>>{};
        elements.emplace_back();
        auto ds = std::vector<diagnostic::Diagnostic>{};
        for (int i{0}; i < tokens.size(); i++) {
            auto r = std::visit(
                [&]<typename T>(
                    T&& t) -> std::optional<diagnostic::ExpectedV<std::pair<Node, int>>> {
                    static_assert(std::is_same_v<T, std::decay_t<T>>);
                    if constexpr (std::is_same_v<T, token::OpenBracket2>) {
                        open_b_ranges.push_back(tokens[i].range);
                        auto node_i_r = parse(tokens.subspan(i + 1), open_b_ranges);
                        auto n_ds = node_i_r.extract_ds();
                        auto node_i = node_i_r.extract_value();
                        ds.insert(ds.end(), std::make_move_iterator(n_ds.begin()),
                                  std::make_move_iterator(n_ds.end()));
                        if (node_i) {
                            auto node = std::move(node_i.value().first);
                            auto new_i = i + node_i.value().second + 1;
                            elements.back().push_back(
                                {.range{diagnostic::Range{.start{tokens[i].range.start},
                                                          .end{tokens[new_i].range.end}}},
                                 .t{std::move(node)}});
                            i = new_i;
                        } else {
                            return std::make_optional(diagnostic::UnexpectedV{std::move(ds)});
                        }
                    } else if constexpr (std::is_same_v<T, token::CloseBracket2>) {
                        if (open_b_ranges.empty()) {
                            ds.push_back(diagnostic::Diagnostic{
                                .range{tokens[i].range}, .message{"Can not find matching '(('"}});
                        } else {
                            open_b_ranges.pop_back();
                            return std::make_optional(diagnostic::ExpectedV{
                                std::make_pair(Node{std::move(elements)}, i), std::move(ds)});
                        }
                    } else if constexpr (std::is_same_v<T, token::Semicolon>) {
                        if (elements.back().empty()) {
                            ds.push_back(diagnostic::Diagnostic{.range{tokens[i].range},
                                                                .message{"Extra ';' found"}});
                        } else {
                            elements.emplace_back();
                        }
                    } else if constexpr (std::is_same_v<T, token::Comment>) {
                        // do nothing
                    } else if constexpr (std::is_same_v<T, token::OpenBracket> ||
                                         std::is_same_v<T, token::CloseBracket> ||
                                         std::is_same_v<T, token::Number> ||
                                         std::is_same_v<T, token::Assign> ||
                                         std::is_same_v<T, token::OperatorBinary> ||
                                         std::is_same_v<T, token::OperatorUnary>) {
                        elements.back().push_back(
                            Debracketed{.range{tokens[i].range}, .t{std::forward<T>(t)}});
                    } else {
                        static_assert(false, "Not exhaustive");
                    }
                    return std::nullopt;
                },
                std::move(tokens[i].t));
            if (r) {
                if (auto v = r->extract_value()) {
                    auto& elements = v->first.elements;
                    if (elements.back().empty()) {
                        elements.pop_back();
                    }
                }
                return std::move(*r);
            }
        }
        if (!open_b_ranges.empty()) {
            for (const auto& range : open_b_ranges) {
                ds.push_back(diagnostic::Diagnostic{
                    .range{range}, .message{R"range(Can not find matching "))")range"}});
            }
            return tl::unexpected(ds);
        }
        if (elements.back().empty()) {
            elements.pop_back();
        }
        return std::make_pair(Node{std::move(elements)}, tokens.size());
    }
} // namespace de_double_bracket

// namespace de_single_bracket {
//     struct Node;
//     using Debracketed = diagnostic::WithInfo<std::variant< //
//         token::Number,                                     //
//         token::Assign,                                     //
//         token::OperatorBinary,                             //
//         token::OperatorUnary,                              //
//         de_double_bracket::Node,                           //
//         Node>>;
//     struct Node {
//         NON_COPIABLE(Node)
//
//         std::vector<Debracketed> childrens;
//         explicit Node(std::vector<Debracketed>&& childrens) : childrens(std::move(childrens)) {}
//     };
//
//     diagnostic::ExpectedV<std::vector<Debracketed>>
//     parse(std::vector<de_double_bracket::Debracketed>&& tokens) {
//         auto debracketed = std::vector<std::vector<Debracketed>>{};
//         auto bracket_range_start = std::vector<diagnostic::Range>{};
//         auto mismatch_b = std::make_optional<int>();
//
//         for (auto i = 0; i < tokens.size(); i++) {
//             mismatch_b = std::visit(
//                 [&]<typename T>(T&& t) -> std::optional<int> {
//                     static_assert(std::is_same_v<T, std::decay_t<T>>);
//                     if constexpr (std::is_same_v<T, token::OpenBracket>) {
//                         debracketed.emplace_back();
//                         bracket_range_start.emplace_back(tokens[i].range);
//                     } else if constexpr (std::is_same_v<T, token::CloseBracket>) {
//                         if (bracket_range_start.empty()) {
//                             return std::make_optional(i);
//                         }
//                         auto childrens = std::move(debracketed.back());
//                         debracketed.pop_back();
//                         auto range_start = bracket_range_start.back();
//                         bracket_range_start.pop_back();
//                         auto node = Debracketed{
//                             .range{.start{range_start.start}, .end{tokens[i].range.end}},
//                             .t{Node{std::move(childrens)}}};
//                         debracketed.back().push_back(std::move(node));
//                     } else if constexpr (std::is_same_v<T, token::Number> ||
//                                          std::is_same_v<T, token::Assign> ||
//                                          std::is_same_v<T, token::OperatorUnary> ||
//                                          std::is_same_v<T, token::OperatorBinary> ||
//                                          std::is_same_v<T, de_double_bracket::Node>) {
//                         debracketed.back().push_back(
//                             Debracketed{.range{tokens[i].range}, .t{std::forward<T>(t)}});
//                     } else {
//                         static_assert(false, "Not exhaustive");
//                     }
//                     return std::nullopt;
//                 },
//                 std::move(tokens[i].t));
//             if (mismatch_b) {
//                 break;
//             }
//         }
//
//         if (mismatch_b) {
//             auto ds = std::vector<diagnostic::Diagnostic>{};
//             for (auto i = *mismatch_b; i < tokens.size(); i++) {
//                 std::visit(
//                     [&]<typename T>(const T&) {
//                         static_assert(std::is_same_v<T, std::decay_t<T>>);
//                         if constexpr (std::is_same_v<T, token::CloseBracket>) {
//                             ds.push_back(diagnostic::Diagnostic{
//                                 .range{tokens[i].range}, .message{"Can not find matching '('"}});
//                         }
//                     },
//                     tokens[i].t);
//             }
//             return tl::unexpected(ds);
//         }
//
//         if (!bracket_range_start.empty()) {
//             auto ds = std::vector<diagnostic::Diagnostic>{};
//             for (const auto& start_b_range : bracket_range_start) {
//                 ds.push_back(diagnostic::Diagnostic{.range{start_b_range},
//                                                     .message{"Can not find matching ')'"}});
//             }
//             return tl::unexpected(ds);
//         }
//
//         return std::move(debracketed[0]);
//     }
// } // namespace de_single_bracket
//
// namespace ast {
//     diagnostic::ExpectedV<Node<Any>> parse_any(std::span<de_single_bracket::Debracketed> tokens) {
//         assert(tokens.size() > 0);
//         // check if this is an assignment
//         if (tokens.size() >= 1 && std::holds_alternative<token::Assign>(tokens[1].t)) {
//         }
//         for (auto& token : tokens) {
//             std::visit(
//                 [&]<typename T>(T&& t) {
//                     static_assert(std::is_same_v<T, std::decay_t<T>>);
//                     if constexpr (std::is_same_v<T, token::Number>) {
//                     } else if constexpr (std::is_same_v<T, token::Assign>) {
//                     } else if constexpr (std::is_same_v<T, token::OperatorUnary>) {
//                     } else if constexpr (std::is_same_v<T, token::OperatorBinary>) {
//                     } else if constexpr (std::is_same_v<T, de_double_bracket::Node>) {
//                     } else if constexpr (std::is_same_v<T, de_single_bracket::Node>) {
//                     } else {
//                         static_assert(std::is_same_v<T, void>, "Non exhaustive");
//                     }
//                 },
//                 std::move(token.t));
//         }
//     }
//
//     diagnostic::ExpectedV<Node<Any>>
//     parse_element(std::vector<de_double_bracket::Debracketed>&& tokens) {
//         auto db_tokens_ex = de_single_bracket::parse(std::move(tokens));
//         if (!db_tokens_ex.has_value()) {
//             return tl::unexpected(db_tokens_ex.error());
//         }
//         auto db_tokens = std::move(db_tokens_ex.value());
//         return parse_any(db_tokens);
//     }
//
//     diagnostic::ExpectedV<Node<Array>>
//     parse_array(diagnostic::WithInfo<de_double_bracket::Node>&& node) {
//         auto elements = std::vector<Node<Any>>{};
//         auto ds = std::vector<diagnostic::Diagnostic>{};
//         for (auto& element : node.t.elements) {
//             auto e = parse_element(std::move(element));
//             if (e.has_value()) {
//                 elements.push_back(std::move(e.value()));
//             } else {
//                 ds.insert(ds.end(), e.error().begin(), e.error().end());
//             }
//         }
//         if (!ds.empty()) {
//             return tl::unexpected(ds);
//         }
//         return Node<Array>{.range{node.range},
//                            .t{std::make_unique<Array>(Array{.elements{std::move(elements)}})}};
//     }
// } // namespace ast

diagnostic::ExpectedV<ast::Array> parse(std::span<token::Token> tokens) {
    std::vector<diagnostic::Range> open_b_ranges{};
    auto d_nodes_i = de_double_bracket::parse(tokens, open_b_ranges);
    if (!d_nodes_i) {
        return tl::unexpected(d_nodes_i.error());
    }
    auto d_nodes = std::move(d_nodes_i.value().first);
    auto range = diagnostic::Range{};
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
    //     .map([](ast::Node<ast::Array> a) -> ast::Array { return std::move(*a.t.release());
    //     });
}
