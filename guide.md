# How to actually write any useful program in circle-lang
We all know that circle-lang is extremely difficult to work with due to us mere
mortals being too inferior to reason about it. This document highlights some
tricks to make working with circle-lang more manageable for us mere mortals.

## Naming Conventions
Start all your variables with an uppercase letter and have the rest be lowercase
to make sure there will be no variable collision.

Utilize the first uppercase letter to categorize global circular array indexing
so that there are no possibility of global circular array collision.

The category of capital letters in global circular array indexing are as
follows:
- Use `(V)` as the call stack counter. Set it to `V_0 + 1*n` where `n` is
  the depth.
  - Use `( (V) + 1*1 )` as the starting boolean variable for functions.
  - For function arguments:
    - Use `( (V) + 1*1 ) := ((0))` first initialized the index with an array of
      length $\pi$.
    - Then `( (V) + 1*1 )(Arg1) := x` to set named arguments. Make sure that the
      name of the arguments aren't capital letters.
- Prefix your function name with `F_`.
- Store return value in `(R)`.
- Use `(S)` as the starting boolean variable for simulating if with while
  loop.

## ; Semicolons
- Put semicolons at the front of each but the first statement of each loop. This
  is because semicolons are much harder to miss when they're aligned at the
  start.

## Dynamic array
- Use an array of two elements, the first element being the length, the second
  element being the array of length `1` where all elements are stored on `n*1`
  where `n` is any integer. Note that because all number implicitly multiplies
  by $\pi$, this result in all elements being stored on indices on multiplies
  $\pi^2$.

## Homoiconicity
*Array evaluate to themselves!* This means that if you write:
```
(Y) := a * 1
; (X) := (( (Y) ))
; (Y) := b * 1
; (std_output_char) := (X)(0)
; (std_output)
; (Y) := c * 1
; (std_output_char) := (X)(0)
; (std_output)
```
It will print `bc`. This is because `(Y)` within the array isn't evaluated on
line two, it is only evaluated on line 4 when `(X)(0)` evaluate to `(Y)` which
then evaluate to `b * 1`. And after that `(X)` is still occupied by `(( (Y) ))`
which is why when `(Y)` changed to `c * 1`, it prints `c` on line 8.

## Examples
- For some examples, check out [these sample programs](./sample-program/).
