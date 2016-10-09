; This file is unchanged from the MANX software systems version
; it is included here only to insure that it gets loaded into data
; space first (so that we can avoid destroying the memory variables)
; :ts=8
codeseg	segment	para public 'code'
	public	$MEMRY
	public	_mbot_, sbot, _lowmem_
dataseg	segment	para public 'data'
$MEMRY	dw	-1
	public	errno_
errno_	dw	0
	public	_dsval_,_csval_
_dsval_	dw	0
_csval_	dw	0
_mbot_	dw	0
sbot	dw	0
_lowmem_:
	extrn	_Uorg_:byte,_Uend_:byte
dataseg	ends
	public	exitad, exitcs
exitad	dw	0
exitcs	dw	0

	assume	cs:codeseg,ds:dataseg,es:dataseg,ss:dataseg
	extrn Croot_:near
	public	$begin
	public	_exit_
$begin	proc	far
	mov	bp,dataseg
	test	bp,bp
	jnz	notcom
	mov	bp,ds
notcom:
	mov	exitcs,ds
	mov	bx,[2]	;get top segment
	sub	bx,bp		;compute size of Data seg
	cmp	bx,4096		;check if greater than 64K
	jbe	smallseg
	lea	bx,[bp+4096]	;end address of segment (paragraphs)
	mov	ax,es
	sub	bx,ax		;compute length of segment
	mov	ah,4aH		;SETBLOCK system call
	int	21H
	mov	bx,4096
smallseg:
	mov	es,bp
	mov	cl,4
	shl	bx,cl
	cli
	mov	ss,bp
	mov	sp,bx
	sti
	cld
;		clear uninitialized data
	mov	di,offset _Uorg_
	mov	cx,offset _Uend_
	sub	cx,di
	inc	cx
	shr	cx,1
	jcxz	noclear
	sub	ax,ax
rep	stosw
noclear:
;
	mov	es,[2cH]		;get enviroment segment
	sub	di,di
	mov	cx,7fffH
arglook:
	mov	ah,es:byte ptr [di]
	cmp	ah,'='			;look for null named env. string
	je	found_args
	test	ah,ah
	jz	no_args
repne	scasb				;look for null byte
	jz	arglook
no_args:
	mov	cl,[80H]
	sub	ch,ch
	mov	si,81H
	mov	ax,1
	jmp	short mov_args
;
found_args:
	sub	ax,ax
	stosb			;zap and skip over '='
	mov	si,di
	mov	di,es
	mov	ds,di
mov_args:
	push	ax			;first arg # for Croot
	mov	es,bp			;reload ES with our real dataseg
	mov	di,es:$MEMRY
	push	di			;argp for Croot
	jcxz	cpy_done
cpy_args:				;copy argument string to work buffer
	lodsb
	test	al,al
	jz	cpy_done
	stosb
	loop	cpy_args
cpy_done:
	sub	al,al
	stosb			;null terminate string
	mov	ds,bp			;set DS, now DS, SS, ES are equal
	inc	di
	and	di,0fffeH		;adjust to word boundary
	mov	$MEMRY,di	;save memory allocation info for sbrk()
	mov	_mbot_,di
	mov	ax,sp
	sub	ax,2048		;allow 3Kb of stack space
	mov	sbot,ax
	mov	_dsval_,ds
	mov	_csval_,cs
	call	Croot_		;Croot(argp, first)
	jmp	short exit
_exit_:
	pop	ax
	pop	ax		;fetch return code
exit:
	mov	ah,4cH
	int	21H
	jmp	dword ptr exitad
$begin	endp
codeseg	ends
	end	$begin
