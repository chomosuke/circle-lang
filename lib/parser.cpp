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
        auto fatal = false;

        auto elements_stack = std::vector<std::vector<Element>>();
        elements_stack.emplace_back();
        elements_stack.back().emplace_back();

        auto open_b_ranges = std::vector<diag::Range>();

        for (auto&& token : tokens) {
            std::visit(
                [&]<typename T>(T&& t) -> bool {
                    static_assert(std::is_same_v<T, std::decay_t<T>>);
                    if constexpr (std::is_same_v<T, token::OpenBracket2>) {
                        open_b_ranges.push_back(token.range);
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
                                {.range{
                                     diag::Range{.start{start_range.start}, .end{token.range.end}}},
                                 .t{Node{std::move(elements)}}});
                        } else {
                            diags.push_back(
                                diag::Diagnostic{.level{diag::error},
                                                 .range{token.range},
                                                 .message{R"(Can not find matching "((")"}});
                            // Emit an diagnostics but continue as normal otherwise
                            fatal =
                                true; // Make sure to return nullopt as bracket parsing has failed
                        }
                    } else if constexpr (std::is_same_v<T, token::Semicolon>) {
                        if (elements_stack.back().back().empty()) {
                            diags.push_back(diag::Diagnostic{.level{diag::warning},
                                                             .range{token.range},
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
                            Debracketed{.range{token.range}, .t{std::forward<T>(t)}});
                    } else {
                        static_assert(false, "Not exhaustive");
                    }
                    return false;
                },
                std::move(token.t));
        }
        if (!open_b_ranges.empty()) {
            diags.reserve(open_b_ranges.size());
            for (const auto& range : open_b_ranges) {
                diags.push_back(
                    diag::Diagnostic{.level{diag::error},
                                     .range{range},
                                     .message{R"range(Can not find matching "))")range"}});
            }
            return std::nullopt;
        }
        if (fatal) {
            return std::nullopt;
        }
        assert(elements_stack.size() == 1);
        if (elements_stack.back().back().empty()) {
            // For tailing ;
            elements_stack.back().pop_back();
        }
        return Node{std::move(elements_stack.back())};
    }
} // namespace de_double_bracket

namespace de_bracket {
    struct SingleBracket;
    struct DoubleBracket;
    using Debracketed = diag::WithInfo<std::variant< //
        token::Number,                               //
        token::Assign,                               //
        token::OperatorBinary,                       //
        token::OperatorUnary,                        //
        SingleBracket,                               //
        DoubleBracket>>;
    struct SingleBracket {
        NON_COPIABLE(SingleBracket)

        std::vector<Debracketed> children;
        explicit SingleBracket(std::vector<Debracketed>&& children);
    };
    struct DoubleBracket {
        NON_COPIABLE(DoubleBracket)

        std::vector<std::vector<Debracketed>> elements;
        explicit DoubleBracket(std::vector<std::vector<Debracketed>>&& elements);
    };
    SingleBracket::SingleBracket(std::vector<Debracketed>&& children)
        : children(std::move(children)) {}
    DoubleBracket::DoubleBracket(std::vector<std::vector<Debracketed>>&& elements)
        : elements(std::move(elements)) {}

    DoubleBracket parse(de_double_bracket::Node&& d_node, diag::Diags& diags);

    std::vector<Debracketed> parse_element(std::vector<de_double_bracket::Debracketed>&& tokens,
                                           diag::Diags& diags) {
        auto debracketed_stack = std::vector<std::vector<Debracketed>>{};
        debracketed_stack.emplace_back();
        auto open_b_ranges = std::vector<diag::Range>{};
        for (auto&& token : std::move(tokens)) {
            std::visit(
                [&]<typename T>(T&& t) -> void {
                    static_assert(std::is_same_v<T, std::decay_t<T>>);
                    if constexpr (std::is_same_v<T, token::OpenBracket>) {
                        open_b_ranges.push_back(token.range);
                        debracketed_stack.emplace_back();
                    } else if constexpr (std::is_same_v<T, token::CloseBracket>) {
                        if (!open_b_ranges.empty()) {
                            auto debracketed = std::move(debracketed_stack.back());
                            debracketed_stack.pop_back();
                            auto start_range = open_b_ranges.back();
                            open_b_ranges.pop_back();
                            debracketed_stack.back().push_back(
                                {.range{
                                     diag::Range{.start{start_range.start}, .end{token.range.end}}},
                                 .t{SingleBracket{std::move(debracketed)}}});
                        } else {
                            diags.push_back(
                                diag::Diagnostic{.level{diag::error},
                                                 .range{token.range},
                                                 .message{"Can not find matching '('"}});
                            // Emit an diagnostics but continue as normal otherwise
                        }
                    } else if constexpr (std::is_same_v<T, token::Number> ||
                                         std::is_same_v<T, token::Assign> ||
                                         std::is_same_v<T, token::OperatorBinary> ||
                                         std::is_same_v<T, token::OperatorUnary>) {
                        debracketed_stack.back().push_back(
                            {.range{token.range}, .t{std::forward<T>(t)}});
                    } else if constexpr (std::is_same_v<T, de_double_bracket::Node>) {
                        debracketed_stack.back().push_back(
                            Debracketed{.range{token.range}, .t{parse(std::forward<T>(t), diags)}});
                    } else {
                        static_assert(false, "Not exhaustive");
                    }
                },
                std::move(token.t));
        }
        if (!open_b_ranges.empty()) {
            diags.reserve(open_b_ranges.size());
            for (const auto& range : open_b_ranges) {
                diags.push_back(diag::Diagnostic{
                    .level{diag::error}, .range{range}, .message{"Can not find matching ')'"}});
            }
            // Ignore missing brackets after emitting diagnostics
            auto debracketed = std::vector<Debracketed>();
            for (auto&& d : debracketed_stack) {
                debracketed.insert(debracketed.end(), std::make_move_iterator(d.begin()),
                                   std::make_move_iterator(d.end()));
            }
            return debracketed;
        }
        assert(debracketed_stack.size() == 1);
        return std::move(debracketed_stack.back());
    }

    DoubleBracket parse(de_double_bracket::Node&& d_node, diag::Diags& diags) {
        auto elements = std::vector<std::vector<Debracketed>>{};
        for (auto&& element : std::move(d_node.elements)) {
            elements.push_back(parse_element(std::move(element), diags));
        }
        return DoubleBracket{std::move(elements)};
    }
} // namespace de_bracket

// namespace ast {
//     diagnostic::ExpectedV<Node<Any>> parse_any(std::span<de_single_bracket::Debracketed> tokens)
//     {
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
