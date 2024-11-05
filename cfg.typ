Since circle lang is dynamically typed, things like `( I )( I )` are legal,
and `( I )` could possibily evaluate to `1`, things that are obviously illegal
like `1( I )` (indexing a number) will be legal at parse time and will result in
runtime error.

$
&"S" &::= &"ArrayBody" \

&"Expr" &::= &"Expr0" \

&"Expr0" &::= &"Expr0 '&&' Expr1" \
& & | & "Expr0 '||' Expr1" \
& & | & "Expr1" \

&"Expr1" &::= &"Expr1 '=' Expr2" \
& & | & "Expr1 '!=' Expr2" \
& & | & "Expr1 '<' Expr2" \
& & | & "Expr1 '>' Expr2" \
& & | & "Expr1 '<=' Expr2" \
& & | & "Expr1 '>=' Expr2" \
& & | & "Expr2" \

&"Expr2" &::= &"Expr2 '+' Expr3" \
& & | & "Expr2 '-' Expr3" \
& & | & "Expr3" \

&"Expr3" &::= &"Expr3 '*' Expr4" \
& & | & "Expr3 '/' Expr4" \
& & | & "Expr3 '%' Expr4" \
& & | & "Expr4" \

&"Expr4" &::= &"'!' Expr4" \
& & | & "Expr5" \

&"Expr5" &::= &"Index" \
& & | & "Assign" \
& & | & "Array" \
& & | & italic("identifier") \
& & | & italic("number") \

&"Assign" &::= &"Index ':=' Expr" \

&"Index" &::= &"'(' Expr ')'" \
& & | & "Expr5 '(' Expr ')'" \

&"Array" &::= &"'((' ArrayBody '))'"\
&"ArrayBody" &::= &"Expr" \
& & | &"Expr ';' ArrayBody" \
& & | & "';' ArrayBody" \
& & | & epsilon \
$

Assignment bind tightest to the left but loosest to the right. I can't figure
out a way to make the grammar express that.
