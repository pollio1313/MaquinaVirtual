; ERROR: fallo de segmento - direccion fisica fuera del segmento de datos
; DS mide 16384 - largo del codigo, asi que [16380] (4 bytes) siempre se pasa.
; Esperado: imprime 111 y despues fallo de segmento (fuera de limite).
        mov [0], 111        ; marca: debe imprimir 111 antes del error
        mov edx, ds
        ldh ecx, 4
        ldl ecx, 1
        mov eax, 1
        sys 2

        mov eax, [16380]
        stop
