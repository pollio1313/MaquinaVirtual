; ERROR: division por cero (segundo operando = 0)
; Esperado: imprime 111 y despues el error de division por cero. No llega al STOP.
; Variantes (cambiar las 2 lineas de abajo): "div eax, 0"  o  "mov [4], 0" + "div eax, [4]"
        mov [0], 111        ; marca: debe imprimir 111 antes del error
        mov edx, ds
        ldh ecx, 4
        ldl ecx, 1
        mov eax, 1
        sys 2

        mov eax, 10
        mov ebx, 0
        div eax, ebx
        stop
