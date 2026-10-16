; NO es un error: sin STOP, cuando IP sale del segmento de codigo el programa termina normal
; (la lectura de instrucciones queda exceptuada del fallo de segmento).
; Esperado: imprime 111 y termina sin ningun mensaje de error.
        mov [0], 111        ; marca: debe imprimir 111 antes del error
        mov edx, ds
        ldh ecx, 4
        ldl ecx, 1
        mov eax, 1
        sys 2

