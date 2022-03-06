; skp with no flags = jump
	ldax ADCL
	jmp target
	clr
target:	wrax DACL, 0.0
