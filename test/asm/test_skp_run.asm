; Check SKP RUN flag
	ldax ADCL
	skp RUN, target
	clr
target:	wrax DACL, 0.0
