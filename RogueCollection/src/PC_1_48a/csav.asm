;***************
; Copied from MANX Library, modified to do stack overflow checking
; :ts=8
dataseg	segment byte public 'data'
	extrn	sbot:word, smsg_:word
dataseg	ends

codeseg	segment byte public 'code'
	assume cs:codeseg,ds:dataseg
	extrn	$begin:far
	dw	near ptr $begin
	public	$csav, $cret, _overflow_
	ifdef	is_com_
	extrn	PutCom_:near
	endif
$csav	proc	near
	pop	bx
	push	bp
	mov	bp,sp
	add	sp,ax
	cmp	sp,sbot
	ja	sok
;
; Here we are, in stack overflow land !!!!
;
_overflow_:
	mov	dx,smsg_
	mov	ah,9
	int	21h
	mov	ah,4ch
	int	21h
sok:
	push	di
	push	si
	call	bx

$cret:
	pop	si
	pop	di
	mov	sp,bp
	pop	bp
	ret
$csav	endp
codeseg	ends
	end
