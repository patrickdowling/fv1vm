; Program: RMP AMP values
; RMP0 = variable, RMP1 = fixed
; POT2: --

.include "defs.asm"

init:
	skp RUN, start
	wldr	0, 32767, AMP4096
	wldr	1, 32767, AMP4096

start:
.include "pot0_rmp0_rate.asm"
.include "pot1_rmp0_amp.asm"

	cho	rdal, RMP0
	wrax	DACL, 0
	cho	rdal, RMP1
	wrax	DACR, 0
