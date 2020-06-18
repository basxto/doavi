;!THEORETICAL CODE; NOT YET TESTED!
; Incremental RLE decompression for Game Boy
; This is useful for tilemaps with long incremental sequences
;  and values between 0 and 127
; It compresses to 0.8%-100% of the original size
; The worst case can be avoided by changing the tile order
;  in the image loaded to vram
;
; Code:
; 0XXX XXXX are literals
; 1XXX XXXX increments <0XXX XXXX> by <next byte> times
; -> 0x00 is 256 times
;
; Example:
; DE: 0x80 0x03 0x01
; HL: 0x00 0x01 0x02 0x01

	.area	_CODE
	; unpack DE to HL for B decompressed bytes
	; A is incremental counter
	; C is the byte
.irle_unpack_block::
	ld	a, #0x01
.irle_unpack_loop:
	dec a
	jr nz, .irle_unpack_noload
	; A is 0
	ld a, (de)
	inc de
	ld c, a
	; highest bit decides mode
	bit 7, a
	jr z, .irle_unpack_literal
	; remove command bit
	res 7, c
	; load amount
	ld a, (de)
	inc de
	jr .irle_unpack_noload
.irle_unpack_literal:
	; one run
	ld	a, #0x01
.irle_unpack_noload:
	ld (hl+), c
	inc c
	dec b
	jr nz,.irle_unpack_loop
	ret