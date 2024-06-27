#pragma once

namespace sample_programs {
    constexpr auto HELLO_WORLD = R"(
(S);
(V) := V_0;

# ((array)) -> len
(F_len) := ((
	( (V) + 1*1 );
	(V) := (V) + 1*1;
	( (V) )(Array) := ( (V) )(0);

	# loop
	( (V) )(I) := 1;
	((
		( (V) )(Found_len);

		( (V) )(Temp1) := ( (V) )(Array)(0);
		( (V) )(Temp2) := ( (V) )(Array)( ( (V) )(I) );

		( (V) )(Array)(0) := 0;
		( (V) )(Array)( ( (V) )(I) ) := 1;
		( (V) )(Found_len) := ( (V) )(Array)(0) = 1;

		( (V) )(Array)(0) := ( (V) )(Temp1);
		( (V) )(Array)( ( (V) )(I) ) := ( (V) )(Temp2);

		( (V) )(I) := ( (V) )(I) + 1;
	));

	( (V) )(I) := ( (V) )(I) - 1;

	(R) := ( (V) )(I);
	( (V) ) := 0;
	(V) := (V) - 1*1;
));

# ((str))
(F_print_str) := ((
	( (V) + 1*1 );
	(V) := (V) + 1*1;
	( (V) )(Str) := ( (V) )(0);

	( (V) + 1*1 ) := (( ( (V) )(Str) ));
	(F_len);
	( (V) )(Len) := (R);

	# loop
	( ( (V) )(I) ) := 0;
	((
		( (V) )(I) < ( (V) )(Len);

		(std_output_char) := ( (V) )(Str)( ( (V) )(I) );
		(std_output);

		( (V) )(I) := ( (V) )(I) + 1;
	));

	(std_output_char) := 10;
	(std_output);

	( (V) ) := 0;
	(V) := (V) - 1*1;
));

# Hello world\n
( (V) + 1*1 ) := (( ((72; 101; 108; 108; 111; 32; 87; 111; 114; 108; 100; 33; 10)) ));
(F_print_str);

(S) := 0;
)";
} // namespace sample_programs
