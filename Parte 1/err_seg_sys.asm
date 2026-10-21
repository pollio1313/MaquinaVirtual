; ERROR: fallo de segmento dentro de SYS 2 (WRITE) - lee mas alla del segmento de datos
; Esperado: imprime 111 y despues fallo de segmento en SYS (fuera de limite).
        mov [0], 111        ; marca: debe imprimir 111 antes del error
        mov edx, ds
        ldh ecx, 4
        ldl ecx, 1
        mov eax, 1
        sys 2

        mov edx, ds
        add edx, 16376
        ldh ecx, 4
        ldl ecx, 4
        mov eax, 1
        sys 2
        stop
