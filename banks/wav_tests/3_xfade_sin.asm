; Program: xfade sin signal
; POT1: --
; POT2: SIN0 rate

.include "defs.asm"

init:
	skp	RUN, start
	wldr	RMP0, 0, AMP4096
	wlds	SIN0, 511, 32767

start:
.include "pot0_rmp0_rate.asm"

	ldax	POT2
	wrax	SIN0_RATE, 0.0

	cho	rdal, SIN0
	cho	sof, RMP0, NA|REG, 0
	wrax	DACL, 0

	cho	rdal, SIN0
	cho	sof, RMP0, NA|COMPC, 0
	wrax	DACR, 0
