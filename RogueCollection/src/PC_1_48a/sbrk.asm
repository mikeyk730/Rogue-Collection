; :ts=8
;Copyright (C) 1983 by Manx Software Systems
dataseg	segment	para public 'data'
	extrn	$MEMRY:word
	extrn	_mbot_:word, sbot:word
	extrn	errno_:word
dataseg	ends
codeseg	segment	para public 'code'
	assume	cs:codeseg,ds:dataseg,es:dataseg,ss:dataseg
;
; sbrk(size): return address of current top & bump by size bytes
;
	public sbrk_
sbrk_	proc	near
	mov	bx,sp
	push	di
	mov	ax,2[bx]
	mov	di,$MEMRY
	add	ax,di
	push	ax
	call	brk_
	pop	cx
	jnz	brk_error
	mov	ax,di		;return original value of the break
brk_error:
	pop	di
	test	ax,ax		;set flags for C
	ret
;
; brk(addr):	set current top address to addr
;		returns 0 if ok, -1 if error
;
	public	brk_
brk_:
	mov	bx,sp
	mov	ax,2[bx]
	cmp	ax,sbot			;check for stack/heap overflow
	jae	brk_ov
	cmp	ax,bx			;double check with sp for saftey
	jae	brk_ov
;	cmp	ax,_mbot_		; ** DISABLED FOR ROGUE
;	jb	brk_ov			; ** DISABLED FOR ROGUE
	mov	$MEMRY,ax	;new value is good so save it away
	sub	ax,ax
	ret
; invalid request
brk_ov:
	mov	errno_,-4
	mov	ax,-1
	test	ax,ax
	ret
;
; rsvstk(size):		set saftey margin for stack
;			this will make sure that at least size
;			bytes of stack below the current level remain.
;
rsvstk_:
	mov	bx,sp
	sub	bx,2[bx]
	mov	sbot,bx
	ret
sbrk_	endp
codeseg	ends
	end
