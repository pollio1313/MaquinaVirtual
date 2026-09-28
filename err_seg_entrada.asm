; ERROR: fallo de segmento - entrada de la tabla con valor -1 (segmento 2 no usado)
; Esperado: imprime 111 y despues fallo de segmento (segmento invalido 2).
        mov [0], 111        ; marca: debe imprimir 111 antes del error
        mov edx, ds
        ldh ecx, 4
        ldl ecx, 1
        mov eax, 1
        sys 2

        mov edx, 0
        ldh edx, 2          ; EDX = segmento 2, offset 0
        mov eax, [edx]
        stop
