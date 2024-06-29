#pragma once

#include "lexer.hpp"
#include <memory>
#include <tl/expected.hpp>
#include <variant>
#include <vector>

namespace ast_node {
    struct Array;
    struct Assign;
    struct OperatorBinary;
    struct OperatorUnary;
    struct Index;

    using Any = std::variant<Array, Assign, OperatorBinary, OperatorUnary, Index>;

    template <typename T> class Node {
      private:
        diagnostic::Range m_range;
        std::unique_ptr<T> m_t;
    };

    struct Array {
        std::vector<Node<Any>> elements;
    };

    struct Assign {
        Node<Index> lhs;
        Node<Any> rhs;
    };

    using Operable = std::variant<OperatorBinary, OperatorUnary, Index>;
    struct OperatorBinary {
        number::op::Binary kind;
        Node<Operable> lhs;
        Node<Operable> rhs;
    };

    struct OperatorUnary {
        number::op::Unary kind;
        Node<Operable> rhs;
    };

    using Indexable = std::variant<std::monostate, Array, Index>;
    struct Index {
        Node<Indexable> subject;
    };
} // namespace ast_node

tl::expected<ast_node::Node<ast_node::Any>, diagnostic::Diagnostic>
parse(std::span<token::Token> tokens);
