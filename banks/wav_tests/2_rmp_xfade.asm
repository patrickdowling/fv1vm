; Program: XFADE plausibility test
; POT0: --
; POT1: --
; POT2: --

.include "defs.asm"

init:
	skp RUN, start
	wldr	0, 32767, AMP4096
	wldr	1, 32767, AMP4096

start:
	or 	MAX
	cho	sof, RMP0, NA|REG, 0
	wrax	REG0, 0.0

	clr
	cho	sof, RMP0, NA|COMPC, 0
	rdax	REG0, 1.0
	wrax	DACL, 0

	cho	rdal, RMP0
	wrax	DACR, 0
