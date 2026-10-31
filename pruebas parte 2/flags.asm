; PRUEBA DE FLAGS (CC) - tabla de ejemplos del enunciado MV1
; Se guarda CC y el resultado de cada operacion en el segmento de datos y al final se vuelcan en hexa.
; bits de CC: N=31 Z=30 C=29 V=28
        mov edx, ds
        ; MOV -1                  -> N=1
        mov eax, -1
        mov [edx], cc
        mov [edx+4], eax
        ; ADD 2147483647+2147483647 -> N=1 V=1
        ldh eax, 0x7FFF
        ldl eax, 0xFFFF
        mov ebx, eax
        add eax, ebx
        mov [edx+8], cc
        mov [edx+12], eax
        ; SUB -3, 5               -> N=1 C=1
        mov eax, -3
        sub eax, 5
        mov [edx+16], cc
        mov [edx+20], eax
        ; MUL 1073741827 * 4      -> C=1 V=1 (resultado 12)
        ldh eax, 0x4000
        ldl eax, 3
        mul eax, 4
        mov [edx+24], cc
        mov [edx+28], eax
        ; DIV 6 / 7               -> Z=1 (resultado 0, resto 6 en AC)
        mov eax, 6
        div eax, 7
        mov [edx+32], cc
        mov [edx+36], eax
        mov [edx+40], ac
        ; CMP 5, 5                -> Z=1 (no modifica EAX)
        mov eax, 5
        cmp eax, 5
        mov [edx+44], cc
        ; volcado: 12 celdas de 4 bytes en hexa
        mov eax, 0x08
        mov ecx, 12
        ldh ecx, 4
        sys 2
        stop
