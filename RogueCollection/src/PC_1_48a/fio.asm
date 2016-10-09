;:ts=8
;***************************
; This file contains the interface to the DOS
; file i/o routines used by rogue
;***************************

dataseg	segment para public 'data'
	extrn	errno_:word
dataseg ends

codeseg	segment para public 'code'
	assume	cs:codeseg, ds:dataseg
	public	open_, close_, read_, write_, unlink_, creat_, lseek_

open_	proc	near
	push	bp
	mov	bp,sp
	mov	dx,4[bp]	; File name in dx
	mov	al,6[bp]	; Open mode
	mov	ah,3dh

fio_dos:
	push	si
	push	di
	int	21h
	jnc	aok
	mov	errno_,ax
	mov	ax,0ffffh
aok:
	pop	di
	pop	si
	pop	bp
	ret
open_	endp

close_	proc	near
	push	bp
	mov	bp,sp
	mov	bx,4[bp]
	mov	ah,3eh
	jmp	fio_dos
close_	endp

read_	proc	near
	push	bp
	mov	bp,sp
	mov	bx,4[bp]		; File handle
	mov	cx,8[bp]		; Byte count
	mov	dx,6[bp]		; buffer addr
	mov	ah,3fh
	jmp	fio_dos
read_	endp

write_	proc	near
	push	bp
	mov	bp,sp
	mov	bx,4[bp]		; File handle
	mov	cx,8[bp]		; Byte count
	mov	dx,6[bp]		; buffer addr
	mov	ah,40h
	jmp	fio_dos
write_	endp

unlink_	proc	near
	push	bp
	mov	bp,sp
	mov	dx,4[bp]
	mov	ah,41h
	jmp	fio_dos
unlink_	endp

creat_	proc	near
	push	bp
	mov	bp,sp
	mov	dx,4[bp]
	mov	cx,0
	mov	ah,3ch
	jmp	fio_dos
creat_	endp

lseek_	proc	near
	push	bp
	mov	bp,sp
	mov	bx,4[bp]
	mov	dx,6[bp]
	mov	cx,8[bp]
	mov	al,10[bp]
	mov	ah,42h
	jmp	fio_dos
lseek_	endp

codeseg	ends
