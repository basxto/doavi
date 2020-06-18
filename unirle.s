;!THEORETICAL CODE; NOT YET TESTED!
; Incremental RLE decompression for Game Boy
; This is useful for tilemaps with long incremental sequences
;  and values between 0 and 127
; It compresses to 0.8%-200% of the original size
;
; by basxto and ISSOtm


; Format:
; AAAA AAA0 1111 1111 write A and terminate
; AAAA AAA0 CCCC CCCC write A++ C+2 times
; AAAA AAA1 write A once
; @param de Data stream being Encoded as above
; @param hl Destination buffer
.irle_unpack::
	ld c, #0
.irle_cmd:
	; load literal
	ld a, (de)
	inc de
	; shift command bit to C
	srl a
	jr C, .irle_pp
	ld b, a
	; load literal
	ld a, (de)
	inc a
	; 0xFF+1 == 0x00
	jr Z, .irle_end
	ld c, a
	inc de
	; restore literal
	ld a, b
.irle_pp:
	inc c
.irle_write:
	ld (hl+), a
	inc a
	dec c
	jr NZ, .irle_write
	; c is 0
	jr .irle_cmd
.irle_end:
	ld (hl), b
	ret



