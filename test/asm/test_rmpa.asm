; Write a negative value in ADDR_PTR register
; Then try to lookup using it.
	clr
	or	0x800200
	wrax	ADDR_PTR, 0
	or 	0x123456
	wra	2, 0
	rmpa	1.0
	wrax	DACL, 0
