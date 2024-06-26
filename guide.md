Start all your variables with an uppercase letter and have the rest be lowercase
to make sure there will be no variable collision.

Utilize the first uppercase letter to categorize global circular array indexing
so that there are no possibility of global circular array collision.

The category of capital letters in global circular array indexing are as
follows:
- Make `((V))` as the call stack counter. Set it to `((V_0)) + 1*n` where `n` is
  the depth.
- Prefix your function name with `F_`.
- Standard library function are prefixed with `F_std_`.
- Store return value in `((R))`.
