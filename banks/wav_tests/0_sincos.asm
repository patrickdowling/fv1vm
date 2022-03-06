; Program: sin/cos on l/r
; POT0: --
; POT1: --
; POT2: --

EQU	RATE	0x1ff
EQU 	MAX	0x7fff

init:
	skp	RUN, start
	wlds	0, RATE, MAX

start:
	cho	rdal, SIN0
	wrax	DACL, 0
	cho 	rdal, SIN0, COS
	wrax	DACR, 0
