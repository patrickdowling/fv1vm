; SIN0 -> L, RMP0 ->R

	skp	RUN, main
	wlds	SIN0, 511, 32767
	wldr	RMP0, 32767, 4096

main:	cho	rdal, SIN0
	wrax	DACL, 0.0
	cho	rdal, RMP0
	wrax	DACR, 0.0
