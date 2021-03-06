; compression format: https://github.com/bonimy/MushROMs/blob/master/doc/LC_LZ3%20Compression%20Format.md
; compressor: https://github.com/aaaaaa123456789/lzcomp but set MAX_COMMAND_COUNT to 256

; see unlz3.h for calling convention

; format:
;[x] .lz3_unpack_block_000 / 0x0: ; direct copy
; CCCL LLLL MMMM MMMM NNNN NNNN ... (L+1 bytes)
;[x] .lz3_unpack_block_001 / 0x2: ; byte fill
; CCCL LLLL YYYY YYYY
;[x] .lz3_unpack_block_010 / 0x4: ; word fill
; CCCL LLLL YYYY YYYY ZZZZ ZZZZ
;[x] .lz3_unpack_block_011 / 0x6: ; zero fill
; CCCL LLLL
;[x] .lz3_unpack_block_100 / 0x8: ; repeat
; CCCL LLLL AYYY YYYY [ZZZZ ZZZZ]
;[x] .lz3_unpack_block_101 / 0xA: ; bit-reverse repeat
; CCCL LLLL AYYY YYYY [ZZZZ ZZZZ]
;[x] .lz3_unpack_block_110 / 0xC: ; backwards repeat
; CCCL LLLL AYYY YYYY [ZZZZ ZZZZ]
;[~] .lz3_unpack_block_111 / 0xE: ; long length
; 111C CC00 LLLL LLLL [depends on CCC]
; simplified this one => 256B instead of 1024B

_lz3_unpack_sprite_data::
	ld	bc, #_set_sprite_data
	jr	.lz3_unpack_data

_lz3_unpack_win_data::
_lz3_unpack_bkg_data::
	; set new return address for .lz3_unpack_block
	ld	bc, #_set_bkg_data
.lz3_unpack_data:
	push	bc

	ldhl	sp,#(6+3)
	; jr	.lz3_unpack_block_data
	; reads the next 2 byte to BC
	.db #0x11

_lz3_unpack_block::
	; skip over return address
	ldhl	sp, #(2+3) ; 2 bytes
.lz3_unpack_block_data:
	; src    - de
	ld	a, (hl-)
	ld	d, a
	ld	a, (hl-)
	ld	e, a
	; dst     - hl
	ld	a, (hl-)
	ld	l, (hl)
	ld	h, a

	; a and	b are free
	push	hl ; so we can restore later in start_extended

.lz3_unpack_block_header:
	; read 1byte header
	ld	a, (de) ; CCCL LLLL
	inc	de
	ld	b, a
	and	a, #0x1F
	ld	c, a
.lz3_unpack_block_command:
	inc	c ; 0 is always once

	bit	7, b
	jr	NZ, .lz3_unpack_block_1XX
.lz3_unpack_block_0XX:
	bit	6, b
	jr	NZ, .lz3_unpack_block_01X
.lz3_unpack_block_00X:
	bit	5, b
	jr	NZ, .lz3_unpack_block_001
.lz3_unpack_block_000: ; direct copy
	ld	a, (de)
	ld	(hl+), a
	inc	de
	dec	c
	jr	NZ, .lz3_unpack_block_000
	jr	.lz3_unpack_block_header
.lz3_unpack_block_01X:
	bit	5, b
	jr	NZ, .lz3_unpack_block_011
.lz3_unpack_block_010: ; word fill
	ld	a, (de)
	inc	de
	ld	b, a
	ld	a, (de)
	inc	de
	jr	.lz3_unpack_block_fill
.lz3_unpack_block_011: ; zero fill
	xor	a
	jr	.lz3_unpack_block_fill_doubler
.lz3_unpack_block_1XX:
	bit	6, b
	; repeat and	bit-reverse repeat
	; will be distinguised later
	jr	Z, .lz3_unpack_block_start ; aka _10X
.lz3_unpack_block_11X:
	bit	5, b
	jr	Z, .lz3_unpack_block_110
.lz3_unpack_block_111: ; long length
	; read length byte
	ld	a, (de) ; LLLL LLLL
	inc	de
	ld	c, a
	; b<<3
	ld	a, b
	rlca
	rlca
	rlca
	ld	b, a
	; terminate on 1111 1111
	inc	a
	jr	Z, .lz3_unpack_block_terminate
	jr	.lz3_unpack_block_command
.lz3_unpack_block_terminate:
	pop	hl
	ret

.lz3_unpack_block_001: ; byte fill
	ld	a, (de)
	inc	de
.lz3_unpack_block_fill_doubler:
	ld	b, a
	; fall through
; a: second byte to write
; b: first byte  to write (alternate)
; c: amount (1 is once)
.lz3_unpack_block_fill:
	ld	(hl), b
	inc	hl
	dec	c
	jr	Z, .lz3_unpack_block_header
	ld	(hl+), a
	dec	c
	jr	NZ, .lz3_unpack_block_fill
	jr	.lz3_unpack_block_header

.lz3_unpack_block_110: ; backwards repeat
; b: command
; c: amount (1 is once)
; get start address for repeat
; pushes de
; sets de to address
.lz3_unpack_block_start:
	ld	a, (de)
	inc	de
	ld	(hl), b
	bit	7, a
	jr	Z, .lz3_unpack_block_start_extended
	push	de
	; de will become hl
	push	hl
	; first bit	ignored
	res	7, a
	; two complement, we want to substract (Y+1)
	cpl
	ld	d, #0xFF
	; it’s relative to the output buffer
	jr	.lz3_unpack_block_start_trail
.lz3_unpack_block_start_extended:
	inc	de
	push	de
	push	hl
	ld	b, a ; upper byte
	; set de to dest buffer start
	ldhl	sp, #(4) ; skip hl, de
	ld	a, (hl+)
	ld	h, (hl)
	ld	l, a
	; get lower byte
	dec	de
	ld	a, (de)
	ld	d, b
.lz3_unpack_block_start_trail:
	ld	e, a ; lower byte
	add	hl, de
	ld	e, l
	ld	d, h
	pop	hl
	; fall through
	bit	5, (hl)
	jr	NZ, .lz3_unpack_block_repeat_rev
	bit	6, (hl)
	jr	NZ, .lz3_unpack_block_repeat_dec
; three specialized functions are a lot faster for bigger L
; b: command
; c: amount (1 is once)
.lz3_unpack_block_repeat:
	ld	a, (de)
	ld	(hl+), a
	inc	de
	dec	c
	jr	NZ, .lz3_unpack_block_repeat
.lz3_unpack_block_repeat_end:
	pop	de
	jr	.lz3_unpack_block_header

.lz3_unpack_block_repeat_rev:
	ld	a, (de)
	; rotate bits of A
	ld	b, a
	rlca
	rlca
	xor	b
	and	#0xAA
	xor	b
	ld	b, a
	swap	b
	xor	b
	and	#0x33
	xor	b
	rrca
	; restore B
	ld	(hl+), a
	inc	de
	dec	c
	jr	NZ, .lz3_unpack_block_repeat_rev
	jr	.lz3_unpack_block_repeat_end

.lz3_unpack_block_repeat_dec:
	ld	a, (de)
	ld	(hl+), a
	dec	de
	dec	c
	jr	NZ, .lz3_unpack_block_repeat_dec
	jr	.lz3_unpack_block_repeat_end