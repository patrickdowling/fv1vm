; Testing special casing of certain instructions

; RDFX => LDAX
rdfx ADCL, 1.0
rdfx ADCL, 0

; MAXX => ABSA
maxx REG0, 1.0
maxx REG0, 0

; AND => CLR
and 0xf
and 0

; XOR => NOT
xor 0xffff
xor 0xffffff

; SKP special cases
skp RUN, 1	; SKP
skp 0, 2 	; JMP
skp 0, 0	; NOP
skp RUN, 0	; NOP


