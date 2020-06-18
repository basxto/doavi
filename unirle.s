;!THEORETICAL CODE; NOT YET TESTED!
; Incremental RLE decompression for Game Boy
; This is useful for tilemaps with long incremental sequences
;  and values between 0 and 127
; It compresses to 0.8%-200% of the original size
;
; by basxto and ISSOtm

; Format:
; 0000 0000 terminate
; CCCC CCC0 AAAA AAAC write A++ C times
; AAAA AAA1 write A
; @param de Data stream being Encoded as above
; @param hl Destination buffer
.irle_unpack::
	ld c, #0
.irle_cmd:
	ld a, (de)
	inc de
	inc c
	srl a
	jr C, .irle_write
	; C is 0
	ret Z
	; a was upper 7 bits of counter
	ld c, a
	ld a, (de)
	inc de
	; move last bit of a to c
	rra
	rl c
.irle_write:
	ld (hl+), a
	inc a
	dec c
	jr NZ, .irle_write
	; c is 0
	jr .irle_cmd



