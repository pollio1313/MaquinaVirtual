; ERROR: fallo de segmento - desplazamiento negativo que cae antes del inicio del segmento
; Esperado: imprime 111 y despues fallo de segmento (fuera de limite).
        mov [0], 111        ; marca: debe imprimir 111 antes del error
        mov edx, ds
        ldh ecx, 4
        ldl ecx, 1
        mov eax, 1
        sys 2

        mov edx, ds
        mov eax, [edx-4]
        stop
