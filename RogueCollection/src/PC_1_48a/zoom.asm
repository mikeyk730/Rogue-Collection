;:ts=8-----------------------------------------------|
; Fast screen I/O routines.  These peek and poke     |
; the screen directly to run faster than a raped ape |
; Michael Toy, AI Design, March 1984		     |
;----------------------------------------------------|

dataseg	segment para public 'data'
	extrn scr_row_:word, LINES_:word, COLS_:word, ch_attr_:word
	extrn iscuron_:word, c_row_:word, c_col_:word, scr_ds_:word
	extrn no_check_:word, page_no_:word
dataseg	ends

codeseg	segment byte public 'code'
	assume	ds:dataseg, cs:codeseg
	public	move_, putchr_, curch_
;--------------------------------------------------------|
; Here are the functions provided and their C interfaces |
;                                                        |
; move(row, col)					 |
; putchr(ch)						 |
; curch()						 |
; 	Required support from the C language is to set up|
; scr_ds_ to point to the base of the screen and scr_row_  |
; to point to the beginning of each row.  The variables  |
; LINES_ and COLS_ should be set up too.  If iscuron_ is set|
; then we call the BIOS to do things since we need to	 |
; move the hardware cursor too and we probably don't care|
; so much about speed.					 |
;--------------------------------------------------------|

;**********
; move(row, col)
;	Move the cursor to the given row and column
;
move_	proc	near
	push	bp
	mov	bp,sp

;
; Set up the current row and column numbers to point to the correct
; positions.
;
	mov	dx,[bp + 4]
	mov	c_row_,dx
	mov	dx,[bp + 6]
	mov	c_col_,dx
	cmp	iscuron_,0
	jz	mvexit
;
; Here we call the BIOS to update the cursor position since the cursor is
; on and I don't care to play with the 6845.
;
	mov	dh,byte ptr [bp + 4]		; dl is left from above.
	mov	ah,2
	mov	bh,byte ptr page_no_
	int	10h
mvexit:
	pop	bp
	ret
move_	endp

;**********
; putchar(ch)
;	Put the given character on the screen
;
putchr_	proc	near
	push	bp
	mov	bp,sp
	push	di
	push	si

	cmp	iscuron_,0
	jnz	putout

	mov	si,c_row_
	shl	si,1
	mov	di,scr_row_[si]
	mov	ax,c_col_
	shl	ax,1
	add	di,ax
	push	es
	mov	es,scr_ds_
	mov	ah,byte ptr ch_attr_
	mov	al,byte ptr [bp + 4]
	cmp	no_check_,1
	jz	ontime
;
; We need to wait for retrace since this is not the super fast monochrome
; screen that we love so much.  We have to save ax in cx since the brain
; damaged instruction set won't let you do an IN to somewhere else.
;
	mov	cx,ax
	mov	dx,03dah
	cli
wait1:	in	al,dx
	test	al,1
	jnz	wait1
wait2:	in	al,dx
	test	al,1
	jz	wait2
	mov	ax,cx

ontime:
	stosw
	sti
	pop	es
	jmp	putdone
putout:
	mov	ah,9
	mov	al,byte ptr [bp + 4]
	mov	bl,byte ptr ch_attr_
	mov	bh,byte ptr page_no_
	mov	cx,1
	int	10h
putdone:
	pop	si
	pop	di
	pop	bp
	ret
putchr_	endp

;**********
; curch()
;	Return character and attribute at given position
;
curch_	proc	near
	push	bp
	mov	bp,sp
	push	di
	push	si

	cmp	iscuron_,0
	jnz	dosget

	mov	si,c_row_
	shl	si,1
	mov	di,scr_row_[si]
	mov	ax,c_col_
	shl	ax,1
	add	di,ax
	push	es
	mov	es,scr_ds_
	cmp	no_check_,1
	jz	goget
;
; We need to wait for retrace since this is not the super fast monochrome
; screen that we love so much.  We have to save ax in cx since the brain
; damaged instruction set won't let you do an IN to somewhere else.
;
	mov	dx,03dah
	cli
iwait1:	in	al,dx
	test	al,1
	jnz	iwait1
iwait2:	in	al,dx
	test	al,1
	jz	iwait2

goget:
	mov	ax,es:[di]
	sti
	pop	es
	jmp	getdone
dosget:
	mov	ah,2
	mov	dh,byte ptr c_row_
	mov	dl,byte ptr c_col_
	mov	bh,byte ptr page_no_
	int	10h
	mov	ah,8
	mov	bh,byte ptr page_no_
	int	10h
getdone:
	pop	si
	pop	di
	pop	bp
	ret
curch_	endp


codeseg	ends
	end
