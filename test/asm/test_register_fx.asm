; Plausibility tests for RDFX, WRLX

init:
	skp	RUN, start

start:
	ldax	ADCL
	rdfx	REG0, 0.1
	wrax	REG0, 1.0
	wrax	DACL, 0

	ldax	ADCL
	wrlx	REG1, 0.2
	wrax	DACR, 0

exit:
