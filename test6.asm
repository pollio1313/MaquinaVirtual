; ---------- TEST 6: lectura + eco (SYS 1 y SYS 2) ----------
; Ingresá un número decimal: debe imprimirlo de vuelta
        mov eax, 1
        mov edx, ds
        add edx, 72
        ldh ecx, 4
        ldl ecx, 1
        sys 1
        mov eax, 1
        sys 2
        stop