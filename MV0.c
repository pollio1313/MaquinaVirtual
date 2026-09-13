#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct operandos {
    int cod;
    char *tipo;
    int cantBytes;
};

const struct operandos tablaOperandos[] = {
    {0,"ninguno",0},
    {1,"registro",1},
    {2,"inmediato",2},
    {3,"memoria",3}
};
struct Instruccion {
    char *nombre;       //esta sirve por ahrao, pero en realidad tendria que ser punteros a funciones con cada metodo
    unsigned char codigo;
    int cantOP;
};

const struct Instruccion tablaInstrucciones[] = {
    {"SYS",  0x00, 1},
    {"JMP",  0x01, 1},
    {"JP",   0x02, 1},
    {"JN",   0x03, 1},
    {"JZ",   0x04, 1},
    {"JC",   0x05, 1},
    {"JV",   0x06, 1},
    {"JNP",  0x07, 1},
    {"JNN",  0x08, 1},
    {"JNZ",  0x09, 1},
    {"NOT",  0x0A, 1},
    {"---",  0x0B, 0},
    {"---",  0x0C, 0},
    {"---",  0x0D, 0},
    {"---",  0x0E, 0},
    {"STOP", 0x0F, 0},
    {"MOV",  0x10, 2},
    {"ADD",  0x11, 2},
    {"SUB",  0x12, 2},
    {"MUL",  0x13, 2},
    {"DIV",  0x14, 2},
    {"CMP",  0x15, 2},
    {"AND",  0x16, 2},
    {"OR",   0x17, 2},
    {"XOR",  0x18, 2},
    {"SWAP", 0x19, 2},
    {"SHL",  0x1A, 2},
    {"SHR",  0x1B, 2},
    {"SAR",  0x1C, 2},
    {"LDL",  0x1D, 2},
    {"LDH",  0x1E, 2},
    {"RND",  0x1F, 2}
};

char posReg(unsigned char pos){             //totalmente incompleta, pero para acordarme, habria q hacerlo como la de instrucciones

}

// Lectura
//Modifique la lectura para no tener 'prueba1.vmx' hardcodeado
int CargarArchivo(const char* filename, unsigned char* memoria) {
    int estado_carga = 0;
    int bytes_leidos;
    FILE *archivoVMX;

    archivoVMX = fopen(filename, "rb");

    if (archivoVMX == NULL) {
        printf("Error: No se pudo abrir el archivo %s\n", filename);
    } else {
        bytes_leidos = fread(memoria, sizeof(unsigned char), 16384, archivoVMX);
        fclose(archivoVMX);
        printf("Se cargaron %d bytes en la memoria.\n", bytes_leidos);
        estado_carga = 1;
    }

    return estado_carga;
}

//Lo unico que falta adaptar a este arreglo de 1 dimensión es la función MostrarCodigo
/*
void MostrarCodigo(unsigned char MemoriaPrincipal[][4]){
    int i,j,cant;
    unsigned char codOperacion,OperandoA,OperandoB,valorOPA[3]={0},valorOPB[3]={0};
    for( i=0;i<=4;i++){
        printf("%c",MemoriaPrincipal[i][3]);            //primeros 5 bytes el VMX
    }

    printf(" %02X",MemoriaPrincipal[i][3]);               //muestra la version
    i+=2;
    printf(" %02X%02X \n",MemoriaPrincipal[i-1][3],MemoriaPrincipal[i][3]);
    int largo = MemoriaPrincipal[i-1][3]+MemoriaPrincipal[i][3];                //esta suma taria bien?
    j=i+1;
    while(j<=largo){                                                            //este es el while q recorre el copdigo y lo muestra
        codOperacion= MemoriaPrincipal[j][3] & 0x1F;                            //aplico una mascara, para sacarle los ultimos 5 bits y asi tenes el codigo de operacion
        cant =tablaInstrucciones[codOperacion].cantOP;
        if (cant==1){
            OperandoA=MemoriaPrincipal[j][3] >>6;
        }
        else if (cant==2){                   //se fija cuantos bytes chupa cada operando
            OperandoB=MemoriaPrincipal[j][3] >>6;
            OperandoA=(MemoriaPrincipal[j][3] >>4) & 0x03;
        }
        else if (cant==0){                   //esta linea esta de mas, porq ya deberian valer 0 de antes
            OperandoA=OperandoB=0;
        }

        for(int q=0;q<OperandoB;q++){                 //while apra consumir los valores de operandos, si es 0 sigeun de largo ,uso operandoB y no cantBytes porq valen lo mismo
            j++;                                         //parece qprimero viene el byte mas signfiquitaivo
            valorOPB[q]=MemoriaPrincipal[j][3];
            //printf("      valor[%d]=%02X q%d  ",q,valorOPB[q],q);
        }
        for(int q=0;q<OperandoA;q++){                 //while apra consumir los valores de operandos, si es 0 sigeun de largo ,uso operandoB y no cantBytes porq valen lo mismo
            j++;
            valorOPA[q]=MemoriaPrincipal[j][3];
        }
        printf("\n %s",tablaInstrucciones[codOperacion].nombre);
        printf("  %02X%02X%02X,%02X%02X%02X",valorOPA[0],valorOPA[1],valorOPA[2],valorOPB[0],valorOPB[1],valorOPB[2]);      //esta mierda es de prueba, hay q hacer codicional de la cantidad de bytes del operando

        j++;
    }
    printf("\nj:%d",j);
}
*/

int main(int argc, char *argv[]) {
    int codigo_salida = 0;
    int mostrar_disassembler = 0;
    const char* filename;
    int Registros[32] = {0};
    unsigned char MemoriaPrincipal[16384] = {0}; //unsigned char ya es exactamente 1 byte (8 bits)
    //se nos iba a hacer un quilombo para calcular las direcciones fisicas
    struct {
        unsigned short base;
        unsigned short size;
    } TablaSegmentos[8];

    // argv[0] = nombre del propio programa (ej: "./vmx"), siempre está
    // argv[1] = filename.vmx (obligatorio)
    // argv[2] = "-d" (opcional)
    if (argc < 2) {
        printf("Uso: vmx filename.vmx [-d]\n");
        codigo_salida = 1;
    } else {
        filename = argv[1];

        if (argc >= 3 && strcmp(argv[2], "-d") == 0) {
            mostrar_disassembler = 1;
        }

        printf("Archivo a ejecutar: %s\n", filename);
        printf("Modo disassembler: %s\n", mostrar_disassembler ? "SI" : "NO");

        if (CargarArchivo(filename, MemoriaPrincipal) == 0) {
            codigo_salida = 1;
        } else {
            /* Aca iria la llamada a MostrarCodigo o EjecutarMaquina */
        }
    }

    return codigo_salida;
}
