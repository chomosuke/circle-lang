#pragma once

#include "diagnostic.hpp"
#include "macros.hpp"
#include "number.hpp"
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
        Node<Index> lhs;
        Node<Any> rhs;
    };

    struct Index {
        std::optional<Node<Any>> subject;
        Node<Any> index;
    };

    struct OperatorBinary {
        number::op::Binary kind;
        Node<Any> lhs;
        Node<Any> rhs;
    };

    struct OperatorUnary {
        number::op::Unary kind;
        Node<Any> rhs;
    };

    struct Number {
        number::Value value;
    };

    template <typename T1, typename T2> Node<T2> convert_node(Node<T1>&& n) {
        return diag::convert_with_info<std::unique_ptr<T1>, std::unique_ptr<T2>>(
            std::move(n),
            [](std::unique_ptr<T1>&& t) { return std::make_unique<T2>(std::move(*t)); });
    }
} // namespace ast

std::optional<ast::Array> parse(std::string_view src_code, diag::Diags& diags);
