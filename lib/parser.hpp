#pragma once

#include "lexer.hpp"
#include "diagnostic.hpp"
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
    using Indexable = std::variant<Array, Index>;
    using Operable = std::variant<Index, OperatorBinary, OperatorUnary, Number>;

    template <typename T> struct Node {
        diagnostic::Range range;
        std::unique_ptr<T> t;

        template <typename U> Node<U> cast() { return Node{.range{range}, .t{t}}; }
    };

    struct Array {
        std::vector<Node<Any>> elements;
    };

    struct Assign {
        Node<Index> lhs;
        Node<Any> rhs;
    };

    struct Index {
        Node<Indexable> subject;
        Node<Operable> index;
    };

    struct OperatorBinary {
        number::op::Binary kind;
        Node<Operable> lhs;
        Node<Operable> rhs;
    };

    struct OperatorUnary {
        number::op::Unary kind;
        Node<Operable> rhs;
    };

    struct Number {
        number::Value value;
    };
} // namespace ast

diagnostic::Expected<ast::Array> parse(std::span<token::Token> tokens);
