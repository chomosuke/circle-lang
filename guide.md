Start all your variables with an uppercase letter and have the rest be lowercase
to make sure there will be no variable collision.

Utilize the first uppercase letter to categorize global circular array indexing
so that there are no possibility of global circular array collision.

The category of capital letters in global circular array indexing are as
follows:
- Use `(V)` as the call stack counter. Set it to `(V_0) + 1*n` where `n` is
  the depth.
  - Use `( (V)+1*1)` as the starting boolean variable for functions.
- Prefix your function name with `F_`.
- Store return value in `(R)`.
- Use `(S)` as the starting boolean variable for simulating if with while
  loop. Set it to 1 before every single run while loop, including function
  calls.
- For function arguments:
  - Use `( (V) + 1*1 ) := ((arg1; arg2))` first to set all positional arguments.
  - Then `( (V) + 1*1 )(arg3) := x` to set named arguments.
