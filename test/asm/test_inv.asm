; Invert outputs every sample
	skp RUN, loop
	sof 0, 0.5
	wrax REG0, 0
loop:	rdax REG0, -2.0
	wrax DACL, -2.0
	wrax DACR, -2.0
	wrax REG0, 0
