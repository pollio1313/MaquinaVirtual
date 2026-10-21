; ERROR: fallo de segmento - codigo de segmento fuera de la tabla (9 >= 8)
; Esperado: imprime 111 y despues fallo de segmento (segmento invalido 9).
        mov [0], 111        ; marca: debe imprimir 111 antes del error
        mov edx, ds
        ldh ecx, 4
        ldl ecx, 1
        mov eax, 1
        sys 2

        mov edx, 0
        ldh edx, 9          ; EDX = segmento 9, offset 0
        mov eax, [edx]
        stop
