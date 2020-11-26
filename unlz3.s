; compression format: https://github.com/bonimy/MushROMs/blob/master/doc/LC_LZ3%20Compression%20Format.md
; compressor: https://github.com/aaaaaa123456789/lzcomp but set MAX_COMMAND_COUNT to 256

; see unlz3.h for calling convention

; format:
;[x] ._lz3_unpack_block_000 / 0x0: ; direct copy
; CCCL LLLL MMMM MMMM NNNN NNNN ... (L+1 bytes)
;[x] ._lz3_unpack_block_001 / 0x2: ; byte fill
; CCCL LLLL YYYY YYYY
;[x] ._lz3_unpack_block_010 / 0x4: ; word fill
; CCCL LLLL YYYY YYYY ZZZZ ZZZZ
;[x] ._lz3_unpack_block_011 / 0x6: ; zero fill
; CCCL LLLL
;[x] ._lz3_unpack_block_100 / 0x8: ; repeat
; CCCL LLLL AYYY YYYY [ZZZZ ZZZZ]
;[x] ._lz3_unpack_block_101 / 0xA: ; bit-reverse repeat
; CCCL LLLL AYYY YYYY [ZZZZ ZZZZ]
;[x] ._lz3_unpack_block_110 / 0xC: ; backwards repeat
; CCCL LLLL AYYY YYYY [ZZZZ ZZZZ]
;[~] ._lz3_unpack_block_111 / 0xE: ; long length
; 111C CC00 LLLL LLLL [depends on CCC]
; simplified this one => 256B instead of 1024B

_lz3_unpack_block::
	; skip over return address
	ldhl	sp,#(2)
.lz3_unpack_block_data:
	; packets - c
	; 16B blocks (8x8 pixels)
	; not enough space for this
	;ld	a, (hl+)
	;ld	b, a
	; dst     - hl
	ld	a, (hl+)
	ld	c, a
	ld	a, (hl+)
	ld	d, a
	; src    - de
	ld	a, (hl+)
	ld	e, a
	ld	a, (hl)
	; swap back
	ld	l, c
	ld	h, d
	ld	d, a

	; a and b are free
	push hl ; so we can restore later in start_extended

._lz3_unpack_block_header:
	; read 1byte header
	ld a, (de) ; CCCL LLLL
	inc de
	ld (hl), a ; store it
	ld b, a
	and a, #0x1F
	ld c, a
._lz3_unpack_block_command:
	inc c ; 0 is always once

	bit 7, b
	jr NZ, ._lz3_unpack_block_1XX
._lz3_unpack_block_0XX:
	bit 6, b
	jr NZ, ._lz3_unpack_block_01X
._lz3_unpack_block_00X:
	bit 5, b
	jr NZ, ._lz3_unpack_block_001
._lz3_unpack_block_000: ; direct copy
	ld a, (de)
	ld (hl+), a
	inc de
	dec c
	jr NZ, ._lz3_unpack_block_000
	jr ._lz3_unpack_block_header
._lz3_unpack_block_001: ; byte fill
	ld a, (de)
	inc de
	ld b, a
	jr ._lz3_unpack_block_fill
._lz3_unpack_block_01X:
	bit 5, b
	jr NZ, ._lz3_unpack_block_011
._lz3_unpack_block_010: ; word fill
	ld a, (de)
	inc de
	ld b, a
	ld a, (de)
	inc de
	jr ._lz3_unpack_block_fill
._lz3_unpack_block_011: ; zero fill
	xor a
	ld b, a
	jr ._lz3_unpack_block_fill
._lz3_unpack_block_1XX:
	bit 6, b
	jr NZ, ._lz3_unpack_block_11X
._lz3_unpack_block_10X:
	bit 5, b
	jr NZ, ._lz3_unpack_block_101
._lz3_unpack_block_100: ; repeat
	jr ._lz3_unpack_block_start
._lz3_unpack_block_101: ; bit-reverse repeat
	jr ._lz3_unpack_block_start
._lz3_unpack_block_11X:
	bit 5, b
	jr NZ, ._lz3_unpack_block_111
._lz3_unpack_block_110: ; backwards repeat
	jr ._lz3_unpack_block_start
._lz3_unpack_block_111: ; long length
	; terminate on 1111 1111
	ld a, #0xFF
	cp a, b
	ret Z
	; read length byte
	ld a, (de) ; LLLL LLLL
	inc de
	ld c, a
	; b<<3
	ld a, b
	add a
	add a
	add a
	ld b, a
	jr ._lz3_unpack_block_command

; a: second byte to write
; b: first byte  to write (alternate)
; c: amount (1 is once)
._lz3_unpack_block_fill:
	ld (hl), b
	inc hl
	dec c
	jr Z, ._lz3_unpack_block_header
	ld (hl+), a
	dec c
	jr NZ, ._lz3_unpack_block_fill
	jr ._lz3_unpack_block_header

; b: command
; c: amount (1 is once)
; get start address for repeat
; pushes de
; sets de to address
._lz3_unpack_block_start:
	ld a, (de)
	inc de
	bit 7, a
	jr Z, ._lz3_unpack_block_start_extended
	push de
	push bc
	; first bit ignored
	res 7, a
	; two complement, we want to substract (Y+1)
	cpl
	ld c, a
	ld b, #0xFF
	; it’s relative to the output buffer
	ld e, l
	ld d, h
	jr ._lz3_unpack_block_start_trail
._lz3_unpack_block_start_extended:
	inc de
	push de
	push bc
	ld b, a ; upper byte
	dec de
	ld a, (de)
	inc de
	ld c, a ; lower byte
	; set de to dest buffer start
	push hl
	ldhl	sp,#(6) ; skip hl, bc, de
	ld	e, (hl)
	inc hl
	ld	d, (hl)
	pop hl
._lz3_unpack_block_start_trail:
	ld a, e
	add c
	ld e, a
	ld a, d
	adc b
	ld d, a
	pop bc
	; fall through

; b: command
; c: amount (1 is once)
._lz3_unpack_block_repeat:
	ld a, (de)
	bit 5, b
	jr Z, ._lz3_unpack_block_repeat_skip_rev
	ld (hl), b
	.rept 8
	rla
	rr b
	.endm
	ld a, b
	ld b, (hl)
	inc de 
	jr ._lz3_unpack_block_repeat_skip_dec
._lz3_unpack_block_repeat_skip_rev:
	bit 6, b
	inc de 
	jr Z, ._lz3_unpack_block_repeat_skip_dec
	dec de
	dec de
._lz3_unpack_block_repeat_skip_dec:
	ld (hl+), a
	dec c
	jr NZ, ._lz3_unpack_block_repeat
	pop de
	jp ._lz3_unpack_block_header
