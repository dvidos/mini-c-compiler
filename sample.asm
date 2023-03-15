	PUSH EBP
	MOV EBP, ESP
	JMP get_next_counter_value_end
get_next_counter_value_end:
	POP EBP
	MOV ESP, EBP
	PUSH EBP
	MOV EBP, ESP
	JMP rect_area_end
rect_area_end:
	POP EBP
	MOV ESP, EBP
	PUSH EBP
	MOV EBP, ESP
	JMP triangle_area_end
triangle_area_end:
	POP EBP
	MOV ESP, EBP
	PUSH EBP
	MOV EBP, ESP
	JMP circle_area_end
circle_area_end:
	POP EBP
	MOV ESP, EBP
	PUSH EBP
	MOV EBP, ESP
	JMP fibonacci_end
if_3_end:
	JMP fibonacci_end
fibonacci_end:
	POP EBP
	MOV ESP, EBP
	PUSH EBP
	MOV EBP, ESP
	JMP factorial_end
if_4_end:
	JMP factorial_end
factorial_end:
	POP EBP
	MOV ESP, EBP
	PUSH EBP
	MOV EBP, ESP
while_5_begin:
	JMP while_5_begin
while_6_begin:
	JMP while_6_begin
math_demo_end:
	POP EBP
	MOV ESP, EBP
	PUSH EBP
	MOV EBP, ESP
while_8_begin:
	JMP while_8_begin
while_8_end:
	JMP while_7_begin
nested_loops_test_end:
	POP EBP
	MOV ESP, EBP
	PUSH EBP
	MOV EBP, ESP
main_end:
	POP EBP
	MOV ESP, EBP
