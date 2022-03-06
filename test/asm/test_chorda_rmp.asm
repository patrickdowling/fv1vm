; CHO RDA (modelled after frequency shift example)
;
mem	delay	4096
mem	temp	1

init:
	skp	RUN, start
	wldr	RMP0, 8192, 4096
start:
	ldax	ADCL
	wra	delay, 0
	cho	rda, RMP0, REG|COMPC, delay
	cho	rda, RMP0, 0, delay + 1
	wra	temp, 0
	cho	rda, RMP0, RPTR2|COMPC, delay
	cho	rda, RMP0, RPTR2, delay + 1
	cho	sof, RMP0, NA|COMPC, 0.0
	cho	rda, RMP0, NA, temp
	wrax	DACL, 0.0
	cho	rdal, RMP0
	wrax	DACR, 0.0
