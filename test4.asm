; ---------- TEST 4: caracteres (como el ejemplo de la spec) ----------
; Esperado: 4 líneas con H, o, l, a (en binario y caracter)
        mov [43], 'a'
        mov [42], 'l'
        mov [41], 'o'
        mov [40], 'H'
        mov edx, ds
        add edx, 43
        ldh ecx, 1
        ldl ecx, 4
        mov eax, 0x12       ; binario + caracter
        sys 2