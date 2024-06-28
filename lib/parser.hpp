#pragma once

#include "lib/lexer.hpp"
#include <memory>
#include <string_view>
#include <tl/expected.hpp>
#include <variant>
#include <vector>

namespace ast_node {
    struct Array;
    struct Assign;
    struct Operator;
    struct Index;

    using Any = std::variant<Array, Assign, Operator, Index>;

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

    using Operable = std::variant<Operator, Index>;
    struct Operator {
        Node<Operable> lhs;
        Node<Operable> rhs;
    };

    using Indexable = std::variant<std::monostate, Array, Index>;
    struct Index {
        Node<Indexable> subject;
    };
} // namespace ast_node

tl::expected<std::vector<token::Token>, diagnostic::Diagnostic> parse(std::string_view src_code);
