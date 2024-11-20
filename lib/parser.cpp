#include "parser.hpp"

#include "lexer.hpp"
#include "macros.hpp"
#include "number.hpp"

#include <cstdio>
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
                            diags.insert(diag::Diagnostic{.level{diag::error},
                                                          .range{token.range},
                                                          .message{"Can not find matching '(('"}});
                            // Emit an diagnostics but continue as normal otherwise
                            fatal =
                                true; // Make sure to return nullopt as bracket parsing has failed
                        }
                    } else if constexpr (std::is_same_v<T, token::Semicolon>) {
                        if (elements_stack.back().back().empty()) {
                            diags.insert(diag::Diagnostic{.level{diag::warning},
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
            for (const auto& range : open_b_ranges) {
                diags.insert(diag::Diagnostic{
                    .level{diag::error}, .range{range}, .message{"Can not find matching '))'"}});
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
                            diags.insert(diag::Diagnostic{.level{diag::error},
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
            for (const auto& range : open_b_ranges) {
                diags.insert(diag::Diagnostic{
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
    Node<Any> parse_any(std::vector<de_bracket::Debracketed>::iterator& tokens_begin,
                        std::vector<de_bracket::Debracketed>::iterator tokens_end,
                        diag::Diags& diags);

    // greater tighter higher
    int get_precedence(number::op::Binary b) {
        switch (b) {
        case number::op::multiply:
        case number::op::divide:
            return 4;
        case number::op::plus:
        case number::op::minus:
            return 3;
        case number::op::equal:
        case number::op::not_equal:
        case number::op::smaller:
        case number::op::smaller_or_equal:
        case number::op::greater:
        case number::op::greater_or_equal:
            return 2;
        case number::op::bool_and:
        case number::op::bool_or:
            return 1;
        }
    }

    Array parse_double(de_bracket::DoubleBracket&& node, diag::Diags& diags) {
        auto elements = std::vector<Node<Any>>();
        for (auto&& element : std::move(node.elements)) {
            elements.push_back(parse_any(std::move(element), diags));
        }
        return Array{.elements{std::move(elements)}};
    }

    using IRBNA = diag::WithInfo<std::variant< //
        token::OperatorBinary,                 //
        token::OperatorUnary,                  //
        Array,                                 //
        Assign,                                //
        Index,                                 //
        Number                                 //
        >>;
    // NOLINTNEXTLINE(cppcoreguidelines-rvalue-reference-param-not-moved)
    std::vector<IRBNA>
    parse_brackets_number_assign(std::vector<de_bracket::Debracketed>::iterator& token,
                                 std::vector<de_bracket::Debracketed>::iterator tokens_end,
                                 diag::Diags& diags) {
        auto irs = std::vector<IRBNA>();
        for (; token != tokens_end; token++) {
            std::visit(
                [&]<typename T>(T&& t) {
                    static_assert(std::is_same_v<T, std::decay_t<T>>);
                    if constexpr (std::is_same_v<T, de_bracket::DoubleBracket>) {
                        auto arr = parse_double(std::forward<T>(t), diags);
                        irs.push_back(IRBNA{.range{token->range}, .t{std::move(arr)}});
                    } else if constexpr (std::is_same_v<T, de_bracket::SingleBracket>) {
                        auto expr = parse_any(std::move(t.children), diags);
                        auto last = std::optional<Node<Indexable>>();
                        if (!irs.empty()) {
                            if (std::holds_alternative<Array>(irs.back().t)) {
                                last = {.range{irs.back().range},
                                        .t{std::make_unique<Indexable>(
                                            std::move(std::get<Array>(irs.back().t)))}};
                            }
                            if (std::holds_alternative<Index>(irs.back().t)) {
                                last = {.range{irs.back().range},
                                        .t{std::make_unique<Indexable>(
                                            std::move(std::get<Index>(irs.back().t)))}};
                            }
                            if (std::holds_alternative<Number>(irs.back().t)) {
                                last = {.range{irs.back().range},
                                        .t{std::make_unique<Indexable>(
                                            std::move(std::get<Number>(irs.back().t)))}};
                            }
                        }
                        auto range = expr.range;
                        if (last) {
                            irs.pop_back();
                            range.start = last->range.start;
                        }
                        auto index = Index{.subject{std::move(last)}, .index{std::move(expr)}};
                        irs.push_back(IRBNA{.range{range}, .t{std::move(index)}});
                    } else if constexpr (std::is_same_v<T, token::Number>) {
                        irs.push_back(
                            IRBNA{.range{token->range}, .t{Number{.value{std::move(t.value)}}}});
                    } else if constexpr (std::is_same_v<T, token::Assign>) {
                        if (!irs.empty() && std::holds_alternative<Index>(irs.back().t)) {
                            auto lhs = std::get<Index>(std::move(irs.back().t));
                            auto lhs_range = irs.back().range;
                            irs.pop_back();
                            token++;
                            if (token != tokens_end) {
                                auto rhs = parse_any(token, tokens_end, diags);
                                irs.push_back(IRBNA{
                                    .range{.start{lhs_range.start}, .end{rhs.range.end}},
                                    .t{Assign{.lhs{.range{lhs_range},
                                                   .t{std::make_unique<Index>(std::move(lhs))}},
                                              .rhs{std::move(rhs)}}}});
                            } else {
                                diags.insert({.level{diag::error},
                                              .range{(token - 1)->range},
                                              .message{"Expected expression after ':='"}});
                            }
                        } else {
                            diags.insert({.level{diag::error},
                                          .range{token->range},
                                          .message{"Unexpected ':='"}});
                            token = tokens_end; // Stop parsing
                        }
                    } else if constexpr (std::is_same_v<T, token::OperatorBinary> ||
                                         std::is_same_v<T, token::OperatorUnary>) {
                        irs.push_back(IRBNA{.range{token->range}, .t{t}});
                    } else {
                        static_assert(false, "Non exhaustive");
                    }
                },
                std::move(token->t));
            if (token == tokens_end) {
                // token was used during a function call in the loop
                break;
            }
        }
        return irs;
    }

    std::optional<Node<OperatorUnary>> parse_unary(std::vector<IRBNA>::iterator& ir,
                                                   std::vector<IRBNA>::iterator irs_end,
                                                   diag::Diags& diags) {
        auto ops = std::vector<diag::WithInfo<number::op::Unary>>();
        for (; ir != irs_end; ir++) {
            auto node = std::optional<Node<Any>>();
            auto early_return = std::visit(
                [&]<typename T>(T&& t) -> bool {
                    static_assert(std::is_same_v<T, std::decay_t<T>>);
                    if constexpr (std::is_same_v<T, token::OperatorBinary>) {
                        diags.insert({.level{diag::error},
                                      .range{ir->range},
                                      .message{std::format("Unexpected '{}' after unary operator",
                                                           diag::to_string(t.kind))}});
                        return true;
                    } else if constexpr (std::is_same_v<T, token::OperatorUnary>) {
                    } else if constexpr (std::is_same_v<T, Array> || std::is_same_v<T, Assign> ||
                                         std::is_same_v<T, Index> || std::is_same_v<T, Number>) {
                        node = Node<Any>{.range{ir->range},
                                         .t{std::make_unique<Any>(std::forward<T>(t))}};
                    } else {
                        static_assert(false, "Non exhaustive");
                    }
                    return false;
                },
                std::move(ir->t));
            if (early_return) {
                return std::nullopt;
            }
            if (node) {
                assert(!ops.empty());
                auto op = Node<OperatorUnary>{
                    .range{.start{ops.back().range.start}, .end{node->range.end}},
                    .t{std::make_unique<OperatorUnary>(
                        OperatorUnary{.kind{ops.back().t}, .rhs{std::move(*node)}})}};
                ops.pop_back();
                while (!ops.empty()) {
                    op = Node<OperatorUnary>{
                        .range{.start{ops.back().range.start}, .end{op.range.end}},
                        .t{std::make_unique<OperatorUnary>(
                            OperatorUnary{.kind{ops.back().t},
                                          .rhs{convert_node<OperatorUnary, Any>(std::move(op))}})}};
                }
                return op;
            }
        }
        return std::nullopt;
    }

    std::optional<Node<Any>> parse_ops(std::vector<IRBNA>&& irs, diag::Diags& diags) {
        assert(!irs.empty());
        auto operator_stack = std::vector<diag::WithInfo<number::op::Binary>>();
        auto operands = std::vector<Node<Any>>();
        auto pop_operator = [&]() {
            auto op = operator_stack.back().t;
            operator_stack.pop_back();
            auto rhs = std::move(operands.back());
            operands.pop_back();
            auto lhs = std::move(operands.back());
            operands.pop_back();
            auto node = Node<Any>{.range{.start{lhs.range.start}, .end{rhs.range.end}},
                                  .t{std::make_unique<Any>(OperatorBinary{
                                      .kind{op}, .lhs{std::move(lhs)}, .rhs{std::move(rhs)}})}};
            operands.push_back(std::move(node));
        };
        for (auto ir = irs.begin(); ir != irs.end(); ir++) {
            auto operand = std::optional<Node<Any>>();
            // NOLINTNEXTLINE(readability-identifier-naming)
            auto operator_ = std::optional<diag::WithInfo<number::op::Binary>>();
            auto early_return = std::visit(
                [&]<typename T>(T&& t) -> bool {
                    static_assert(std::is_same_v<T, std::decay_t<T>>);
                    if constexpr (std::is_same_v<T, token::OperatorBinary>) {
                        operator_ = std::make_optional<diag::WithInfo<number::op::Binary>>(
                            {.range{ir->range}, .t{t.kind}});
                    } else if constexpr (std::is_same_v<T, token::OperatorUnary>) {
                        auto unary = parse_unary(ir, irs.end(), diags);
                        if (unary) {
                            operand = convert_node<OperatorUnary, Any>(std::move(*unary));
                        } else {
                            return true;
                        }
                    } else if constexpr (std::is_same_v<T, Array> || std::is_same_v<T, Assign> ||
                                         std::is_same_v<T, Index> || std::is_same_v<T, Number>) {
                        operand = Node<Any>{.range{ir->range},
                                            .t{std::make_unique<Any>(std::forward<T>(t))}};
                    } else {
                        static_assert(false, "Non exhaustive");
                    }
                    return false;
                },
                std::move(ir->t));
            if (early_return) {
                return std::nullopt;
            }
            if (operand) {
                assert(!operator_);
                if (operator_stack.size() != operands.size()) {
                    assert(operator_stack.size() + 1 == operands.size());
                    diags.insert(diag::Diagnostic{.level{diag::error},
                                                  .range{operand->range},
                                                  .message{"Expected operator"}});
                    return std::nullopt;
                }
                operands.push_back(std::move(*operand));
            } else {
                assert(operator_);
                if (operator_stack.size() + 1 != operands.size()) {
                    assert(operator_stack.size() == operands.size());
                    diags.insert(diag::Diagnostic{
                        .level{diag::error},
                        .range{operator_->range},
                        .message{std::format("Unexpected '{}'", diag::to_string(operator_->t))}});
                    return std::nullopt;
                }
                while (!operator_stack.empty() &&
                       get_precedence(operator_->t) <= get_precedence(operator_stack.back().t)) {
                    pop_operator();
                }
                operator_stack.push_back(*operator_);
            }
        }
        if (operator_stack.size() + 1 != operands.size()) {
            assert(operator_stack.size() == operands.size());
            diags.insert(
                diag::Diagnostic{.level{diag::error},
                                 .range{operator_stack.back().range},
                                 .message{std::format("Unexpected '{}'",
                                                      diag::to_string(operator_stack.back().t))}});
            return std::nullopt;
        }
        while (!operator_stack.empty()) {
            pop_operator();
        }
        assert(operands.size() == 1);
        return std::move(operands.back());
    }

    // NOLINTNEXTLINE(cppcoreguidelines-rvalue-reference-param-not-moved)
    Node<Any> parse_any(std::vector<de_bracket::Debracketed>&& tokens, diag::Diags& diags) {
        auto begin = tokens.begin();
        return parse_any(begin, tokens.end(), diags);
    }

    Node<Any> parse_any(std::vector<de_bracket::Debracketed>::iterator& tokens_begin,
                        std::vector<de_bracket::Debracketed>::iterator tokens_end,
                        diag::Diags& diags) {
        assert(tokens_begin != tokens_end);
        auto irbna = parse_brackets_number_assign(tokens_begin, tokens_end, diags);
        auto any = parse_ops(std::move(irbna), diags);
        if (any) {
            return std::move(*any);
        } else {
            return Node<Any>{
                .range{},
                .t{std::make_unique<Any>(Number{.value{"Place_holder_to_continue_parsing"}})}};
        }
    }
} // namespace ast

std::optional<ast::Array> parse(std::string_view src_code, diag::Diags& diags) {
    auto lexed = lex(src_code, diags);
    if (!lexed) {
        return std::nullopt;
    }
    auto& tokens = lexed.value();
    auto d_nodes_i = de_double_bracket::parse(tokens, diags);
    if (!d_nodes_i) {
        return std::nullopt;
    }
    auto d_nodes = std::move(*d_nodes_i);
    auto debracketed = de_bracket::parse(std::move(d_nodes), diags);

    return ast::parse_double(std::move(debracketed), diags);
}
