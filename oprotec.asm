       ;:ts=8
       ;
       ; Protection search code
       ;       Copyright (c) 1985, HLS Duplication, Inc.
       ;
       ; This listing contains confidential information.
       ;
       ;

       dataseg segment para public 'data'
       public  buff1, buff2
       buff1   db      32 dup (0ffh)
       buff2   db      512 dup (0ffH)
       extrn   goodchk_:word
       dataseg ends

       codeseg segment para public 'code'

       public  protect_
       assume  cs:codeseg
       assume  ds:dataseg

       protect_        proc    near
       push    bp
       mov     bp, sp

       ;
       ;
       ; first read 6 sectors on track 39
       ; use rom as dummy buffer
       ;
       ;

       push    es      ; save es for later
       push    di
       push    si
       mov     di,7    ; try 6 times
       pool:   mov     ax,0206H; read 6 sectors
       mov     bx,0F800H       ; rom at 0F8000H
       mov     es,bx
       xor     bx,bx
       mov     cx,2701H; track 39 / sector 1
       mov     dx,[bp + 4]     ; head 0 / drive A
       int     13
       dec     di      ; 6 times?
       je      nogood  ; yes
       jc      pool    ; if error try again

       ;
       ; now read the signature
       ;
       mov     di,4    ; try 3 times
       pool1:  mov     ax,0201H; read 1 sector
       mov     bx,offset buff1
       mov     cx,2707H; track 39 / sector 7
       mov     dx,[bp + 4]
       push    ds      ; buffer is in cs
       pop     es
       nop     ; for debuggers
       int     13
       dec     di
       je      nogood
       jc      pool1

       ;
       ; now check integrity of signature
       ;
       mov     di,4
       pool2:  mov     ax,0201H
       mov     bx,offset buff2
       mov     cx,27F1H; track 39 / sector ????
       mov     dx,[bp + 4]
       int     13
       dec     di
       je      nogood
       jnc     pool2   ; if no error try again
       cmp     ah,10H  ; crc??
       jne     pool2   ; nope

       ;
       ; final check
       ;
       cld     ; auto-increment
       mov     cx,10H  ; test 32 bytes
       mov     si,offset buff1
       mov     di,offset buff2 + 08cH
       rep     cmpsw
       cmp     cx,0
       jne     nogood

       ;
       ; signature present routine
       ;
       good:
       mov     goodchk_, 0D0DH

       ;
       ; signature failed routine
       ;
       nogood:
       pop     si
       pop     di
       pop     es
       pop     bp
       ret

       protect_endp
       codeseg ends
       end
