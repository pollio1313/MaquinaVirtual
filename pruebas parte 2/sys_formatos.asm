; SYS 2 en todos los formatos (ejemplo del ASM) y SYS 1 (lectura)
        mov [10], 0x41
        shl [10], 8
        or  [10], 'a'           ; 0x4161
        mov edx, ds
        add edx, 12
        ldh ecx, 2
        ldl ecx, 1
        mov eax, 0x0F
        sys 2                   ; 0x4161 0o40541 Aa 16737
        ldh ecx, 4
        mov eax, 1
        mov edx, ds
        sys 1                   ; lee decimal
        mov eax, 0x08
        sys 2
        mov eax, 8
        mov edx, ds
        sys 1                   ; lee hexa
        mov eax, 0x01
        sys 2
        stop
