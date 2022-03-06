; POT1: RMP AMP

; Read pot and scale by -1
; Then we can test for thresholds by using >= and offseting
	sof	0, 0x00000f
	rdax	POT1, -1.0
	sof	1.0, 0.25
	skp	GEZ, amp3
	sof	1.0, 0.25
	skp	GEZ, amp2
	sof	1.0, 0.25
	skp	GEZ, amp1
	jmp	amp0
amp3:
	clr
	or	AMP512
	jmp	out
amp2:
	clr
	or	AMP1024
	jmp	out
amp1:
	clr
	or	AMP2048
	jmp	out
amp0:
	clr
	or	AMP4096
out:
	wrax	RMP0_RANGE, 0.0

