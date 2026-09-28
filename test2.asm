; ---------- TEST 2: lógicas y corrimientos ----------
; Esperado (celdas 0..24): 12  63  240  16  -4  15  -6
        mov eax, 0x0F
        and eax, 0x3C       ; 12
        mov [0], eax
        mov eax, 0x0F
        or eax, 0x30        ; 63
        mov [4], eax
        mov eax, 0x0F
        xor eax, 0xFF       ; 240
        mov [8], eax
        mov eax, 1
        shl eax, 4          ; 16
        mov [12], eax
        mov eax, 0
        sub eax, 16         ; -16
        sar eax, 2          ; -4 (conserva signo)
        mov [16], eax
        mov eax, 0
        sub eax, 16         ; -16 = 0xFFFFFFF0
        shr eax, 28         ; 15 (rellena con ceros)
        mov [20], eax
        mov eax, 5
        not eax             ; -6
        mov [24], eax
        mov edx, ds
        ldh ecx, 4
        ldl ecx, 7
        mov eax, 1
        sys 2
        stop