; ---------- TEST 1: aritmética (MOV, ADD, SUB, MUL, DIV, AC) ----------
; Esperado (decimal, celdas 8,12,16,20,24): 13  7  30  3  1
inicio: mov [0], 10
        mov [4], 3
        mov eax, [0]
        add eax, [4]        ; 13
        mov [8], eax
        mov eax, [0]
        sub eax, [4]        ; 7
        mov [12], eax
        mov eax, [0]
        mul eax, [4]        ; 30
        mov [16], eax
        mov eax, [0]
        div eax, [4]        ; cociente 3, resto 1 en AC
        mov [20], eax
        mov [24], ac        ; 1
        mov edx, ds
        add edx, 8
        ldh ecx, 4
        ldl ecx, 5
        mov eax, 1
        sys 2
        stop