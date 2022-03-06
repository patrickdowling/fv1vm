; Smoke tests for register functions
; Gleaned from delay effect source code to get a "realistic" pattern

EQU HALF	0x3fffff
EQU DELAY_LEN	16383

EQU FEEDBACK 	REG15

EQU OFFSET	REG16

MEM DELAY0	DELAY_LEN
MEM DELAY1	DELAY_LEN

EQU DELAY0_DATA	REG4
EQU DELAY1_DATA REG5

init:
	skp	RUN, start

	or	HALF
	wrax	FEEDBACK, 1.0
	wrax	REG0, 0.5
	wrax	REG1, 0

	or	0x20 << 8
	wrax	OFFSET, 0

	or	0x12345
	wra	DELAY0 + 32, 0

	or 	0x67890
	wra	DELAY1 + 32, 0


start:	clr
	rdax	REG1, 0.5
	wrax	REG2, 0

	or	DELAY0 * 256
	rdax	OFFSET, 1.0
	wrax	ADDR_PTR, 0
	rmpa	1.0
	wrax	DELAY0_DATA, 0

	or	DELAY1 * 256
	rdax	OFFSET, 1.0
	wrax	ADDR_PTR, 0
	rmpa	1.0
	wrax	DELAY1_DATA, 0

	rdax	ADCR, 0.6
	rdax	ADCL, 0.6
	wra	DELAY0, 0
