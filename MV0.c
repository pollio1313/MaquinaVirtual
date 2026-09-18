#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct {    // para organizar un segmento necesitamos anotar dos datos: donde empieza y cuanto mide
    int base;
    int size;
} Segmento;

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
    char codigo;
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

const char *tablaRegistros[32] = {          //la tabla de registros, solamente devuelve el nombre, es apra la muestra, quizas habria q pasarla a estructura, para poner punteros a func
    "IP",       // 0
    "OPC",      // 1
    "OP1",      // 2
    "OP2",      // 3
    "LAR",      // 4
    "MAR",      // 5
    "MBR",      // 6
    "Reservado",// 7
    "Reservado",// 8
    "Reservado",// 9
    "EAX",      // 10
    "EBX",      // 11
    "ECX",      // 12
    "EDX",      // 13
    "EEX",      // 14
    "EFX",      // 15
    "AC",       // 16
    "CC",       // 17
    "Reservado",// 18
    "Reservado",// 19
    "Reservado",// 20
    "Reservado",// 21
    "Reservado",// 22
    "Reservado",// 23
    "Reservado",// 24
    "Reservado",// 25
    "CS",       // 26
    "DS",       // 27
    "Reservado",// 28
    "Reservado",// 29
    "Reservado",// 30
    "Programa"  // 31
};


void Lectura(unsigned char MemoriaPrincipal[],const char *filename){
    FILE *archivoVMX;
    int i=0;

    archivoVMX=fopen(filename,"rb");        //abro el archivo vmx
    if (archivoVMX==NULL){
        printf("NO SE ABRIO CAPO");
    }
    else{
        fread(MemoriaPrincipal, sizeof(unsigned char), 16384, archivoVMX);
        fclose(archivoVMX);
    }
}
void MostrarBinario(char byte) {                    //esto es solo para hacer pruebas, esta prompeado
    for (int i = 7; i >= 0; i--) {
        printf("%d", (byte >> i) & 1);
    }
}

void MostrarCodigo(unsigned char MemoriaPrincipal[]){
    int i,j,cant;
    char codOperacion,OperandoA,OperandoB,valorOPA[3]={0},valorOPB[3]={0};
    for( i=0;i<=4;i++){
        printf("%c",MemoriaPrincipal[i]);            //primeros 5 bytes el VMX
    }

    printf(" %02X",MemoriaPrincipal[5]);               //muestra la version

    printf(" %02X%02X \n",MemoriaPrincipal[6],MemoriaPrincipal[7]);  //muestra cnt de lineas
    int largo = (MemoriaPrincipal[6]<<8) | MemoriaPrincipal[7];                //priomero shifteo al mas significativo y le clavo un or con el menos
    printf("(%d)",largo);
    j=8;
    while(j <= (largo+8)){                                                            //este es el while q recorre el copdigo y lo muestra la condicion es j<CS
        codOperacion= MemoriaPrincipal[j] & 0x1F;                            //aplico una mascara, para sacarle los ultimos 5 bits y asi tenes el codigo de operacion
        char operacion =MemoriaPrincipal[j];
        cant =tablaInstrucciones[codOperacion].cantOP;
        /*
        printf("\n");
        MostrarBinario(operacion);
        printf("\n"); */

        printf("\n %s   ",tablaInstrucciones[codOperacion].nombre);
        if (cant==01){
            OperandoA=operacion >>6 & 0x03;
            for(int q=0;q<OperandoA;q++){                 //while apra consumir los valores de operandos, si es 0 sigeun de largo ,uso operandoB y no cantBytes porq valen lo mismo
                j++;
                valorOPA[q]=MemoriaPrincipal[j];
                printf("%02X",valorOPA[q]);
            }
        }
        else if (cant==02){                   //se fija cuantos bytes chupa cada operando
            OperandoB=(operacion >>6) & 0x03;
            for(int q=0;q<OperandoB;q++){
                j++;                                         //parece qprimero viene el byte mas signfiquitaivo
                valorOPB[q]=MemoriaPrincipal[j];
            }
            OperandoA=(operacion >>4) & 0x03;
            for(int q=0;q<OperandoA;q++){
                j++;
                valorOPA[q]=MemoriaPrincipal[j];
            }
            if (OperandoA==01){
                printf("%s",tablaRegistros[valorOPA[0]]);
            }
            else{
                for(int q=0;q<OperandoA;q++){
                    printf("%02X",valorOPA[q]);
                }
            }
            printf(",");
            if (OperandoB==01){
                printf("%s",tablaRegistros[valorOPB[0]]);
            }
            else{
                for(int q=0;q<OperandoB;q++){
                    printf("%02X",valorOPB[q]);
                }
            }

        }
        else if (cant==00){                   //esta linea esta de mas, porq ya deberian valer 0 de antes
            OperandoA=OperandoB=0;
        }

        j++;
    }
    printf("\nj:%d",j);
}

void AsignarSegmentos(unsigned char MemoriaPrincipal[],Segmento TablaSegmentos[8],int Registros[]){
    int largo=(MemoriaPrincipal[6]<<8) | MemoriaPrincipal[7];

    TablaSegmentos[0].base = 0;
    TablaSegmentos[0].size = largo;

    TablaSegmentos[1].base = largo;
    TablaSegmentos[1].size = 16384-largo;

    Registros[26] = 0; //CS apuntan a la posicion 0
    Registros[27] = 1; // DS apunta a la posicion 1
}
int main(int argc, char *argv[]){

    int Registros[32]={0};                          //Int ya ocupa 4bytes
    unsigned char MemoriaPrincipal[16384];     //La ram es unidimensional un byte tras otro
    Segmento TablaSegmentos[8];                  //0 cs, 1 ds,

    const char *filename;

    if (argc < 2) {
        printf("Uso: vmx filename.vmx [-d]\n");
    }
    else{
        filename = argv[1];
        int mostrar_disassembler = 0;

        if (argc >= 3 && strcmp(argv[2], "-d") == 0) {
            mostrar_disassembler = 1;
        }

        printf("Archivo a ejecutar: %s\n", filename);
        printf("Modo disassembler: %s\n", mostrar_disassembler ? "SI" : "NO");

        // Acá seguiría: abrir el archivo, leer la cabecera, cargar en memoria,
        // ejecutar (y si mostrar_disassembler, imprimir el disassembler)
        Lectura(MemoriaPrincipal,filename);
        AsignarSegmentos(MemoriaPrincipal,TablaSegmentos,Registros);

        if (mostrar_disassembler)
            MostrarCodigo(MemoriaPrincipal);
    }

    return 0;
}
