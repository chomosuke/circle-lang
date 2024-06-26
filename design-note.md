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
- Primitives:
    - Numbers are x * PI.
    - Arrays are circular, and the indices are continuous.
    - Functions are circular, with a starting condition. If the starting
      condition is met, a new iteration is started.
        - Functions are actually just an array of instructions.
- There are no local variable, because that implies call stack, which is not
  circular.
- There are one global variable, which is a big global circular array.
    - The circular array is 256 PI long.
    - To index into this global variable, wrap a number in `()`.
    - You can represent number with letters, they will be transformed into
      numbers by rotating it to the lexicographically smallest rotation, and
      then map it to a 256 PI based number.
        - A sequence of characters will be recognized as a number if it only
          contains `A-Z`, `a-z`, `_`, `0-9`.
- There are no control structure, you have to do it with functions.
- Anonymous function are written as s expressions, which are circular, so they
  are executed as loops, with the first element treated and the condition for
  the loop.
- There are operators: `+` `-` `*` `/` `%` `&&` `||` `=` `!=` `<` `>` `<=` `>=`
  `!`.
    - They are included in the language to make doing math easier. Feels wrong
      to not be able to do math easily for a language that have $\pi$ as a
      primitive.
- Index out of bound just returns 1.
- standard library functions:
    - `((std_input))`
        - Put one user input's ASCII value into `((std_input_char))`
    - `((std_output))`
        - Print ASCII value in `((std_output_char))` as a character
    - Possible std functions replacing operators:
        - `((std_plus))`
            - Modify `((std_plus_x))` with `((std_plus_x)) + ((std_plus_y))`.
        - `((std_minus))`
            - Modify `((std_minus_x))` with `((std_minus_x)) - ((std_minus_y))`.
        - `((std_multiply))`
            - Modify `((std_multiply_x))` with `((std_multiply_x)) * ((std_multiply_y))`.
        - `((std_divide))`
            - Modify `((std_divide_x))` with `((std_divide_x)) / ((std_divide_y))`.
        - `((std_mod))`
            - Modify `((std_mod_x))` with `((std_mod_x)) % ((std_mod_y))`.
        - `((std_and))`
            - Modify `((std_and_x))` with `((std_and_x)) && ((std_and_y))`.

Conceptually, a circle-lang program is just a nested array of expressions to
evaluation, with the global circular array. Each array is circular and the first
element of the list are treated as the condition. Array are written as s
expressions. And the way to store variable is to index into the global circular
array.

The entire file is implicitly one global circular array.

Execution and evaluation are different concept because it allows functions being
assigned without being executed.

Problem:
Two people can only eat an even amount of watermelon in liters.
Given the number of liters of watermelon, determine if both people can be
satisfied.

Example program:
```
( # This program has not been updated with the newer design.
    ((main));
    ((len)) := (
        ((len_));
        ((len_)) := 0
        ((len_temp))((0)) := ((len_in))((0))
        ((len_in))((0)) := magic_number
        # Remember the word magic_number will be transformed into a number
        ((len_out)) := 0
        (
            ((len_in))
        )
    );
    ((atoi)) := (
        ((atoi_))
        ((atoi_i)) := 0
        ((atoi_str))((atoi_i))
    );
    input
    ((atoi_str)) := ((input))
    ((atoi))
)
```

```
((unique_name)) := (
    ((F)) = ((F)) + 1*1;

    (( ((F)) ))((some_local_variable)) := something;
    # and some other code

    ((F)) = ((F)) - 1*1;
);

(( ((F)) + 1*1 )) := (parameters);
((unique_name)); # Calls the function
```
