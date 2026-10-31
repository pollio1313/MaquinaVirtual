\\DATA 64
\\EXTRA 16
\\STACK 64
; PILA: valores iniciales, PUSH/POP, CALL/RET
main:   mov edx, ds
        mov eax, [sp]           ; direccion de retorno de main = -1
        mov [edx], eax
        mov eax, [sp+4]         ; argc = 0
        mov [edx+4], eax
        mov eax, [sp+8]         ; argv = -1
        mov [edx+8], eax
        mov [edx+12], sp        ; SP inicial (SS tiene 64 bytes, 12 usados)
        push 5
        push 7
        call suma
        add sp, 8
        mov [edx+16], eax       ; 12
        mov [edx+20], sp        ; SP vuelve al valor inicial
        push 0x1234
        pop ebx
        mov [edx+24], ebx
        mov eax, 0x08
        mov ecx, 8
        ldh ecx, 4
        sys 2
        stop
suma:   mov [edx+28], sp        ; SP dentro de la subrutina (3 pushes mas abajo)
        mov eax, [sp+4]
        add eax, [sp+8]
        ret
