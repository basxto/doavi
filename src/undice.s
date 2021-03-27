; decompress Digram Chain Encoding aka DiCE (byte pair encoding)

	.area	_BSS
.undice_storage: ; jumpback; jumpback adress; size; buffer; str
	.ds	8

	.area	_CODE

_undice_init::
	; just write them to storage
	ld	de, #.undice_storage
	ldhl	sp,#(2)
	; jumpback adress
	xor a
	ld	(de), a
	inc	de
	ld	(de), a
	; jumpback
	inc	de
	ld	(de), a
	inc	de
	; buffer size
	ld	a, (hl+)
	ld	(de), a
	inc	de
	; buffer
	ld	a, (hl+)
	ld	(de), a
	inc	de
	ld	a, (hl+)
	ld	(de), a
	inc	de
	; str
	ld	a, (hl+)
	ld	(de), a
	inc	de
	ld	a, (hl+)
	ld	(de), a
	ret

_undice_line::
	; get parameters from storage
	ld	hl, #.undice_storage+2
	; jumpback- b
	ld	a, (hl+)
	ld	b, a
	; buffer size - c
	ld	a, (hl+)
	ld	c, a
	; buffer - de
	ld	a, (hl+)
	ld	e, a
	ld	a, (hl+)
	ld	d, a
	; str - hl
	ld	a, (hl+)
	ld	h, (hl)
	ld	l, a
.undice_line_loop:
	; read from str
	ld	a, (hl+)
	add	a ; check leftmost bit
	jr	C, .undice_line_reference
	rra ; shift it back
	; write to buffer
	ld	(de), a
	inc	de
	; b=(b>0?b-1:0)
	srl	b
	; jump on b == 1
	jr	c, .undice_line_jumpback
.undice_line_continue_loop:
	cp	#1
	; terminate for value == 0
	jr	C, .undice_line_terminate
	; terminate for value == 1
	jr	Z, .undice_line_fix_nl
	; return on counter == 0
	dec	c
	jr	NZ, .undice_line_loop
.undice_line_terminate:
	ld e, l
	ld d, h

	ld	hl, #.undice_storage+2
	; store b
	ld	(hl), b
	ld	hl, #.undice_storage+6
	; store hl
	ld	(hl), e
	inc	hl
	ld	(hl), d
	; return a
	ld	e, a
	ret
.undice_line_fix_nl:
	dec a
	dec de
	ld	(de), a
	inc a
	jr .undice_line_terminate
.undice_line_reference:
	; a is already *2
	rrc	b ; test for 0
	; skip if we already have a  jumpback address
	jr	NZ, .undice_line_skip
	ld	a, l
	ld	(#.undice_storage), a
	ld	a, h
	ld	(#.undice_storage+1), a
	; fetch a again (faster than push/pep)
	dec	hl
	ld	a, (hl+)
	add a
	.undice_line_skip:
	rlc	b ; revert change
	;_text + 2a will be new hl
	ld	hl, #_text
	; 16bit hl+=a
	add	l
	ld	l, a
	adc	h
	sub	l
	ld	h, a
	; set jumpback counter to 2
	ld	b, #2
	jr .undice_line_loop
.undice_line_jumpback:
	; b is 0
	; C is set
	; jump back
	ld	hl, #.undice_storage
	ld	b, (hl)
	inc	hl
	ld	h, (hl)
	ld	l, b
	ld	b, #0
	jr .undice_line_continue_loop

