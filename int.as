STATUS	equ	0FD8h
INTCON	equ	0FF2h
TMR0IF	EQU	0002
Z	EQU	0002

	psect	intcodelo,global,reloc=2,class=CODE,delta=1

	psect	intcodelo

  	bcf	INTCON, TMR0IF
	bcf	STATUS, Z
	retfie

;	end