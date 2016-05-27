start:
	call erase
	call wait
	call wait
	call wait
	call wait
	call set_pattern
	call wait
	call wait
	call wait
	call wait
	; does not work and i don't know why
	call longwait
jump, start


longwait:
	load s3, 09
	longwait_loop:
		call wait
		sub s3, 01
	jump nz, longwait_loop
return

set_pattern:
	load s1, 04

	sub s1, 01
	load s2, 73
	segset s1, s2

	sub s1, 01
	load s2, 5C
	segset s1, s2

	sub s1, 01
	load s2, 73
	segset s1, s2

	sub s1, 01
	load s2, 5C
	segset s1, s2
return

wait:
	; burn approx 100000 cycles of processing time
	load s6, ff
	wait_01:
		load s7, ff
		wait_02:
			sub s7, 01
		jump nz, wait_02
		sub s6, 01
	jump nz, wait_01
return

erase:
	load s4, 00
	load s5, 04
	loop:
		sub s5, 01
		segset s5, s4
	jump nz, loop
return
