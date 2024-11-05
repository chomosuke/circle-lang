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

namespace ast {
    Array parse_double(de_bracket::DoubleBracket&& node, diag::Diags& diags);
    Node<Any> parse_any(std::vector<de_bracket::Debracketed>&& tokens, diag::Diags& diags);

    Array parse_double(de_bracket::DoubleBracket&& node, diag::Diags& diags) {
        auto elements = std::vector<Node<Any>>();
        for (auto&& element : std::move(node.elements)) {
            elements.push_back(parse_any(std::move(element), diags));
        }
        return Array{.elements{std::move(elements)}};
    }

    // lower tighter
    int get_precedence(number::op::Binary b) {
        switch (b) {
        case number::op::multiply:
        case number::op::divide:
        case number::op::remainder:
            return 0;
        case number::op::plus:
        case number::op::minus:
            return 1;
        case number::op::equal:
        case number::op::not_equal:
        case number::op::smaller:
        case number::op::smaller_or_equal:
        case number::op::greater:
        case number::op::greater_or_equal:
            return 2;
        case number::op::bool_and:
        case number::op::bool_or:
            return 3;
        }
    }

    Node<Any> parse_any(std::vector<de_bracket::Debracketed>&& tokens, diag::Diags& diags) {
        auto output = std::vector<Node<Any>>();
        for (auto&& token : std::move(tokens)) {
            std::visit(
                [&]<typename T>(T&& t) {
                    if constexpr (std::is_same_v<T, de_bracket::DoubleBracket>) {
                        auto arr = parse_double(std::forward<T>(t), diags);
                        output.push_back({.range{token.range}, .t{std::make_unique<Any>(std::move(arr))}});
                    } else if constexpr (std::is_same_v<T, de_bracket::SingleBracket>) {
                        auto expr = parse_any(std::move(t.children), diags);
                        auto index = Index{};
                        output.push_back({.range{token.range}, .t{std::make_unique<Any>(std::move(expr))}})
                    } else if constexpr (std::is_same_v<T, token::Number>) {
                    } else if constexpr (std::is_same_v<T, token::Assign>) {
                    } else if constexpr (std::is_same_v<T, token::OperatorBinary>) {
                    } else if constexpr (std::is_same_v<T, token::OperatorUnary>) {
                    } else {
                        static_assert(false, "Non exhaustive");
                    }
                },
                std::move(token.t));
        }
    }
} // namespace ast

std::optional<ast::Array> parse(std::span<token::Token> tokens, diag::Diags& diags) {
    auto d_nodes_i = de_double_bracket::parse(tokens, diags);
    if (!d_nodes_i) {
        return std::nullopt;
    }
    auto d_nodes = std::move(*d_nodes_i);
    auto debracketed = de_bracket::parse(std::move(d_nodes), diags);

    return ast::parse_double(std::move(debracketed), diags);
}
