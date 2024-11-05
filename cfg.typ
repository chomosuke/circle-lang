Since circle lang is dynamically typed, things like `( I )( I )` are legal,
and `( I )` could possibily evaluate to `1`, things that are obviously illegal
like `1( I )` (indexing a number) will be legal at parse time and will result in
runtime error.
$
&"S" &::= &"ArrayBody" \

&"Expr" &::= &"Expr0" \

&"Expr0" &::= &"Index ':=' Expr0" \
& & | & "Expr1" \

&"Expr1" &::= &"Expr1 '&&' Expr2" \
& & | & "Expr1 '||' Expr2" \
& & | & "Expr2" \

&"Expr2" &::= &"Expr2 '=' Expr3" \
& & | & "Expr2 '!=' Expr3" \
& & | & "Expr2 '<' Expr3" \
& & | & "Expr2 '>' Expr3" \
& & | & "Expr2 '<=' Expr3" \
& & | & "Expr2 '>=' Expr3" \
& & | & "Expr3" \

&"Expr3" &::= &"Expr3 '+' Expr4" \
& & | & "Expr3 '-' Expr4" \
& & | & "Expr4" \

&"Expr4" &::= &"Expr4 '*' Expr5" \
& & | & "Expr4 '/' Expr5" \
& & | & "Expr4 '%' Expr5" \
& & | & "Expr5" \

&"Expr5" &::= &"'!' Expr5" \
& & | & "Expr6" \

&"Expr6" &::= &"Index" \
& & | & "Array" \
& & | & italic("identifier") \
& & | & italic("number") \

&"Index" &::= &"'(' Expr ')'" \
& & | & "Expr6 '(' Expr ')'" \

&"Array" &::= &"'((' ArrayBody '))'"\
&"ArrayBody" &::= &"Expr ';' ArrayBody" \
& & | & "';' ArrayBody" \
& & | & epsilon \
$
