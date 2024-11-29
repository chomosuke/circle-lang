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
- Use `(V)` as the call stack counter. Set it to `V_a + 1*n` where `n` is
  the depth.
  - Use `( (V) + 1*1 )` as the starting boolean variable for functions.
  - For function arguments:
    - Use `( (V) + 1*1 ) := ((0))` first initialized the index with an array of
      length $\pi$.
    - Then `( (V) + 1*1 )(Arg1) := x` to set named arguments.
- Prefix your function name with `F_`.
- Store return value in `(R)`.
- Use `(S)` as the starting boolean variable for simulating if with while
  loop.
- Example:
```
# This evaluates to 1, which allow the whole file to start executing
(S)

# As long as no other global circular array index starts with V there will not be a naming collision
; (V) := V_a

# Initialize ( (V) ) so that we can use this to store local variables
; ( (V) ) := ((0))

# ((Array)) -> len
; (F_len) := ((
  # Assumes that `( (V) + 1*1 )` has been set to `((0))` before the function is called.
  ( (V) + 1*1 )

  # Increment `(V)` for convenience and for recursive function call.
  ; (V) := (V) + 1*1

  # Put all local variable on `( (V) )`.
  ; ( (V) )(I) := 1

  # loop
  ; ((
    ( (V) )(Not_found_len)

    ; ( (V) )(Array)(0) := 0
    ; ( (V) )(Array)( ( (V) )(I) ) := 1
    ; ( (V) )(Not_found_len) := ( (V) )(Array)(0) != 1

    ; ( (V) )(I) := ( (V) )(I) + 1
  ))

  ; ( (V) )(I) := ( (V) )(I) - 1

  # Use `(R)` to store return values
  ; (R) := ( (V) )(I)

  # Set `( (V) )` to 0 to prevent the function / array to be executed again
  ; ( (V) ) := 0

  # Decrement `(V)` to revert earlier incrementation.
  ; (V) := (V) - 1*1
))

# Initialized the next call stack to be `((0))` just as every function assumes.
# Without this (F_len) would not execute
; ( (V) + 1*1 ) := ((0))

# Set argument
; ( (V) + 1*1 )(Array) := ((a; r; r))

# Evaluate the array at `(F_len)` which we defined earlier
; (F_len)

# Fetch return value
; ( (V) )(Len) := (R)

# If statement
# Set `(S)` to the condition of the if statement
; (S) := ( (V) )(Len) = 3

# If `(S)` is non zero the if statement will execute
; ((
  (S)
  ; (std_output_char) := y * 1
  ; (std_output)

  # Prevent the array from executing again
  ; (S) := 0
))
# (S) = 0 no matter if the array executed or not

# To prevent the whole file from executing again
; (S) := 0
```

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
