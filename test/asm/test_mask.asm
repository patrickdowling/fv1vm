; test some logical ops
equ	MAX	0x7fffff
equ	MIN	0x800000
equ	ALL	0xffffff

	clr
	and 0xf
	wrax REG0, 0 ; acc = 0
	or 0xf
	or 0xf00
	wrax REG1, 0 ; acc = 0xf0f
	or MAX
	wrax REG2, 0 ; acc = MAX
	or MIN
	wrax REG3, 0 ; acc = MIN
	not
	wrax REG4, 0 ; acc = -1
	or MIN
	xor ALL
	wrax REG5, 0 ; acc = MAX
