; ---------- TEST 3: SWAP, LDH, LDL (decimal + hexa a la vez) ----------
; Esperado (celdas 28,32,36): 2 (0x2)   1 (0x1)   305419896 (0x12345678)
        mov eax, 1
        mov ebx, 2
        swap eax, ebx
        mov [28], eax       ; 2
        mov [32], ebx       ; 1
        mov ecx, 0
        ldh ecx, 0x1234
        ldl ecx, 0x5678
        mov [36], ecx       ; 0x12345678
        mov edx, ds
        add edx, 28
        ldh ecx, 4
        ldl ecx, 3
        mov eax, 0x09       ; decimal + hexa
        sys 2