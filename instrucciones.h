#include <stdint.h>

typedef struct
{ // para organizar un segmento necesitamos anotar dos datos: donde empieza y cuanto mide
    int base;
    int size;
} Segmento;

typedef void (*InstruccionFunc)(char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[]); // puntero a funcion

// auxiliares
int CalcularDireccionFisica(int operandoMemoria, char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[]);
int ObtenerValorOperando(int operando, char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[]);
void GuardarDestino(int operandoDestino, int resultado, char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[]);
// prototipos de las funciones de ejecucion
void op_SYS(char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[]);
void op_STOP(char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[]);
void op_MOV(char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[]);
void op_ADD(char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[]);
void op_JMP(char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[]);
void op_JP(char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[]);
void op_JN(char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[]);
void op_JZ(char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[]);
void op_JC(char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[]);
void op_JV(char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[]);
void op_JNP(char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[]);
void op_JNN(char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[]);
void op_JNZ(char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[]);
void op_NOT(char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[]);
void op_SUB(char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[]);
void op_MUL(char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[]);
void op_DIV(char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[]);
void op_CMP(char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[]);
void op_AND(char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[]);
void op_OR(char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[]);
void op_XOR(char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[]);
void op_SWAP(char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[]);
void op_SHL(char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[]);
void op_SHR(char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[]);
void op_SAR(char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[]);
void op_LDL(char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[]);
void op_LDH(char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[]);
void op_RND(char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[]);