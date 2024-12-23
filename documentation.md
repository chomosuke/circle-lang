- We think that there are 7 sins of modern programming languages:
  - Not being able to represent transcendental numbers.
  - Having function parameters.
  - Function being able to return values.
  - Local variables.
  - Control flow that is not a loop.
  - Non-circular arrays.
  - Having any type that's not circular.

Hence, we design this brand-new programming language that is perfect in every
way:

# Circle Lang

## Concept
Circle lang is **homoiconic**, which means circle lang code and circle lang data
have the same representation. A circle lang program a single circle lang array
expression. When a circle lang array gets executed, it acts like a while loop
where the first element is treated as the condition and the rest are treated as
the body of the loop.

## Objects and expression
Each circle lang expression is also a circle lang object:

- Numbers are $x \times \pi$. The literal `9` will be translated to $9 \times
  \pi$.
  - Numbers literal can also take on the form of a sequence of characters
    that only contains `A-Z`, `a-z`, `_` and `0-9`. It will be translated
    into a number by rotating it to the lexicographically smallest
    rotation, and then map it to a $256 \pi$ based number where the ASCII
    value of each character are a digit.

- Assignment.
  - They are in the form of `<lvalue> := <rvalue>`.
  - Their execution leads to `<rvalue>` being evaluated and copied into
    `lvalue>`.
  - Note that if `<lvalue> := <rvalue>` is in a sub-expression, it will
    evaluate to itself.
  - `:=` is right associative and have the lowest precedence.

- Arrays are circular, and the indices are continuous.
  - Arrays can be created by `((e1; e2; e3))`. Whitespace doesn't matter.
    `;` is used as separator.
    - The last element in the array can be optionally followed by a `;`.
  - Array has a length of multiple of $\pi$. An array such as `((e1; e2;
    e3))` has a length of $3 \pi$.

- Index
  - Array can be indexed by the `()` operator. For example `((e1; e2;
    e3))(2) = e3`. Zero indexing are used here.
  - Indexing the array with an uninitialized index will return 1.
  - Array indexing will wrap around after its length.
    - `((e1; e2; e3))(<index>) = ((e1; e2; e3))(<index> + <any integer> *
      3)`. (Note that while the array have a length of $3\pi$, the
      literal `3` will be translated into $3\pi$).

- Operator expression.
  - There are operators: `+` `-` `*` `/` `&&` `||` `=` `!=` `<` `>` `<=` `>=`
    `!`.
  - They do exactly what you'd expect.
  - Precedence:
    - `!`
    - `*` `/`
    - `+` `-`
    - `=` `!=` `<` `>` `<=` `>=`
    - `&&` `||`

*Note*: Since all expressions are also objects, they can be passed around
without being evaluated.

## Execution and evaluation:
- Evaluation have no side effect and will never mutate anything.
  - Assignment statement evaluate to themselves.
  - Array evaluate to themselves.
  - Operators evaluate as you'd expect.
    - `(3 * 3 + 3) / (2 * 2 + 2)` evaluate to the fractional polynomial:
      $(3\pi \times 9\pi^2)/(2\pi \times 4\pi^2)$.
    - Boolean operator treats zero as truthy and non-zero value (including non
      numbers) as falsy.
      - All boolean operators: `!` `=` `!=` `<` `>` `<=` `>=` `&&` `||` evaluate
        to either `0` or `1` (reminder that `1` $= 1\pi$)
- When assignment statement in the form of `<lvalue> := <rvalue>` are
  executed, `<rvalue>` will be evaluated, copied and assigned to
  `<lvalue>`.
- When array are executed, it will go through each of its elements whose index
  is a multiple of $\pi$ in order. Each element at the index that is a
  multiple of the array's length will be evaluated and if that element
  evaluates to 0, the array will stop executing. Each element that's not at
  the index that is a multiple of the array's length will be executed.

## Variables
- There are no local variable, because that implies call stack, which is not
  circular.
- There are one global variable, which is a big global circular array.
  - The circular array is $256 \pi$ long.
  - To index into this global variable, simply index with nothing in front of
    the `()`.

## Standard library:
- `(std_input)`
  - Put one user input's ASCII value into `(std_input_char)` and times it by
    $\pi$.
- `(std_output)`
  - Divide `(std_output_char)` by $\pi$ and print it out as an ASCII
    character.
  - `(std_output_char) := 10; (std_output);` will print `\n`.
- `(std_decompose)`
  - Transform store the coefficients of `(std_decompose_number)` in
    `(std_decompose_numerator)` and `(std_decompose_denominator)`
    respectively.
  - E.g.: if `(std_decompose_number)` = $(1 + 2 \times \pi)/(3 + 4 \times
    \pi)$, then after running `(std_decompose)`, `(std_decompose_numerator)`
    will be `((1 / 1; 2 / 1))` and `(std_decompose_denominator)` will be
    `((3 / 1; 4 / 1))`.


## Misc
The source file are implicitly wrapped in a `(())`, turning it into an array
which are then executed when the source file is interpreted.

Anything not defined in this document is undefined behavior.
