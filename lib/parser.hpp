#pragma once

#include "diagnostic.hpp"
#include "lexer.hpp"
#include "macros.hpp"
#include <memory>
#include <tl/expected.hpp>
#include <variant>
#include <vector>

namespace ast {
    struct Array;
    struct Assign;
    struct Index;
    struct OperatorBinary;
    struct OperatorUnary;
    struct Number;

    using Any = std::variant<Array, Assign, Index, OperatorBinary, OperatorUnary, Number>;

    template <typename T> using Node = diag::WithInfo<std::unique_ptr<T>>;

    struct Array {
        std::vector<Node<Any>> elements;
    };

    struct Assign {
        NON_COPIABLE(Assign)

        Node<Index> lhs;
        Node<Any> rhs;
    };

    struct Index {
        NON_COPIABLE(Index)

        std::optional<Node<Any>> subject;
        Node<Any> index;
    };

    struct OperatorBinary {
        NON_COPIABLE(OperatorBinary)

        number::op::Binary kind;
        Node<Any> lhs;
        Node<Any> rhs;
    };

    struct OperatorUnary {
        NON_COPIABLE(OperatorUnary)

        number::op::Unary kind;
        Node<Any> rhs;
    };

    struct Number {
        number::Value value;
    };
} // namespace ast

std::optional<ast::Array> parse(std::span<token::Token> tokens, diag::Diags& diags);
