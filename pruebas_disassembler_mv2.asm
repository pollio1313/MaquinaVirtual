; ============================================================
; PRUEBAS DISASSEMBLER - MAQUINA VIRTUAL PARTE II
; ============================================================

; -------------------------
; Definicion de segmentos
; -------------------------
\\DATA 64
\\EXTRA 32
\\STACK 128

; -------------------------
; Constantes
; -------------------------
NUMERO      EQU 1234
HEXA        EQU 0x1234ABCD
CARACTER    EQU 'A'
TEXTO       EQU "Hola, MV2!\n"
TEXTO2      EQU "Disassembler"

; -------------------------
; Codigo
; -------------------------
main:
; Registros extendidos y pseudonimos
MOV EAX, HEXA
MOV AX, NUMERO
MOV AH, CARACTER
MOV AL, 0x5A

MOV EBX, EAX
MOV BX, AX
MOV BH, AH
MOV BL, AL

; Otros registros
MOV ECX, 10
MOV EDX, 20
MOV EEX, 30
MOV EFX, 40

; Registros de segmentos
MOV EAX, DS
MOV EBX, ES
MOV ECX, SS
MOV EDX, KS
MOV EEX, PS

; Punteros de pila
MOV BP, SP

; Memoria directa
MOV b[0], AL
MOV w[2], AX
MOV l[4], EAX

MOV AL, b[0]
MOV AX, w[2]
MOV EAX, l[4]

; Memoria con segmentos
MOV b[DS+8], AH
MOV w[ES+10], BX
MOV l[ES+12], EDX

; Memoria con registros base
MOV AX, w[EBX]
MOV EAX, l[EDX+4]
MOV BL, b[BP+8]

; Operaciones aritmeticas y logicas
ADD EAX, 1
SUB EBX, 2
MUL ECX, 3
DIV EDX, 4
AND EAX, EBX
OR AX, 0x00FF
XOR AL, 0x0F
NOT EAX

; Desplazamientos
SHL EAX, 1
SHR EBX, 2
SAR ECX, 1

; Pila
PUSH EAX
PUSH AX
PUSH 0x12345678
POP EFX
POP w[ES+4]
PUSH b[DS+8]
POP l[DS+12]

; Fin del programa
STOP