; POT0 = L gain
; POT1 = R gain
	clr
	ldax ADCL
	mulx POT0
	wrax DACL, 0.0
	ldax ADCR
	mulx POT1
	wrax DACR, 0.0
