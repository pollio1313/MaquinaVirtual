; REGISTROS PARCIALES, DESPLAZAMIENTOS Y LOGICA
        mov edx, ds
        ldh eax, 0x1234
        ldl eax, 0x5678
        mov ah, 0xAB            ; 0x1234AB78
        mov [edx], eax
        mov al, 0xFF            ; 0x1234ABFF
        mov [edx+4], eax
        mov ax, 1               ; 0x12340001
        mov [edx+8], eax
        mov ebx, eax
        shl ebx, 4              ; 0x23400010 (se pierde el nibble alto)
        mov [edx+12], ebx
        mov ecx, -16
        sar ecx, 2              ; -4
        mov [edx+16], ecx
        mov ecx, -16
        shr ecx, 28             ; 0xF
        mov [edx+20], ecx
        mov eex, 0x00F0
        and eex, 0x0F0F         ; 0
        mov [edx+24], eex
        mov efx, 0x00F0
        or  efx, 0x0F0F         ; 0x0FFF
        mov [edx+28], efx
        xor efx, 0x00FF         ; 0x0F00
        mov [edx+32], efx
        not efx                 ; 0xFFFFF0FF
        mov [edx+36], efx
        swap eex, efx           ; eex=0xFFFFF0FF efx=0
        mov [edx+40], eex
        mov [edx+44], efx
        mov eax, 0x08
        mov ecx, 12
        ldh ecx, 4
        sys 2
        stop
