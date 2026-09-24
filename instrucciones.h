#include <stdint.h>

typedef struct
{ // para organizar un segmento necesitamos anotar dos datos: donde empieza y cuanto mide
    int base;
    int size;
} Segmento;

typedef void (*InstruccionFunc)(unsigned char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[]); // puntero a funcion

// auxiliares
int ObtenerValorOperando(int operando, unsigned char MemoriaPrincipal[], int Registros[]);
void GuardarDestino(int operandoDestino, int resultado, unsigned char MemoriaPrincipal[], int Registros[]);
// prototipos de las funciones de ejecucion
void op_STOP(unsigned char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[]);
void op_MOV(unsigned char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[]);
void op_ADD(unsigned char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[]);
void op_JMP(unsigned char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[]);
void op_JP(unsigned char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[]);
void op_JN(unsigned char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[]);
void op_JZ(unsigned char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[]);
void op_JC(unsigned char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[]);
void op_JV(unsigned char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[]);
void op_JNP(unsigned char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[]);
void op_JNN(unsigned char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[]);
void op_JNZ(unsigned char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[]);
void op_NOT(unsigned char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[]);
void op_SUB(unsigned char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[]);
void op_MUL(unsigned char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[]);
void op_DIV(unsigned char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[]);
void op_CMP(unsigned char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[]);
void op_AND(unsigned char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[]);
void op_OR(unsigned char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[]);
void op_XOR(unsigned char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[]);
void op_SWAP(unsigned char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[]);
void op_SHL(unsigned char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[]);
void op_SHR(unsigned char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[]);
void op_SAR(unsigned char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[]);
void op_LDL(unsigned char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[]);
void op_LDH(unsigned char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[]);
void op_RND(unsigned char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[]);