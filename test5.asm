; ---------- TEST 5: saltos condicionales (CMP, JNZ, JZ, JN, JMP) ----------
; Suma 10+9+...+1 = 55
; Esperado (celdas 60,64,68): 55  1  1
        mov ebx, 10
        mov eax, 0
suma:   add eax, ebx
        sub ebx, 1
        jnz suma            ; repite hasta que ebx = 0
        mov [60], eax       ; 55
        cmp eax, 55
        jz iguales
        mov [64], 0         ; (no debería pasar por acá)
        jmp sigue1
iguales: mov [64], 1
sigue1: cmp eax, 100
        jn menor
        mov [68], 0         ; (no debería pasar por acá)
        jmp imprime
menor:  mov [68], 1
imprime: mov edx, ds
        add edx, 60
        ldh ecx, 4
        ldl ecx, 3
        mov eax, 1
        sys 2