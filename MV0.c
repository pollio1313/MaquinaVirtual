#include <stdlib.h>
#include <stdio.h>

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


void Lectura(char MemoriaPrincipal[][4]){
    FILE *archivoVMX;
    int i=0;

    archivoVMX=fopen("prueba1.vmx","r");        //abro el archivo vmx
    if (archivoVMX==NULL){
        printf("NO SE ABRIO CAPO");
    }
    else{
        while(fread(&MemoriaPrincipal[i][3],sizeof(char),1,archivoVMX)==1){
            MemoriaPrincipal[i][2]=MemoriaPrincipal[i][1]=MemoriaPrincipal[i][0]=0;
            i++;
        }
        fclose(archivoVMX);
    }
}
void MostrarBinario(char byte) {                    //esto es solo para hacer pruebas, esta prompeado
    for (int i = 7; i >= 0; i--) {
        printf("%d", (byte >> i) & 1);
    }
}

void MostrarCodigo(char MemoriaPrincipal[][4]){
    int i,j,cant;
    char codOperacion,OperandoA,OperandoB,valorOPA[3]={0},valorOPB[3]={0};
    for( i=0;i<=4;i++){
        printf("%c",MemoriaPrincipal[i][3]);            //primeros 5 bytes el VMX
    }
    
    printf(" %02X",MemoriaPrincipal[i][3]);               //muestra la version
    i+=2;
    printf(" %02X%02X \n",MemoriaPrincipal[i-1][3],MemoriaPrincipal[i][3]);  //muestra cnt de lineas
    int largo = (MemoriaPrincipal[i-1][3]<<8) | MemoriaPrincipal[i][3];                //priomero shifteo al mas significativo y le clavo un or con el menos
    printf("(%d)",largo);
    j=i+1;
    while(j<=largo+i){                                                            //este es el while q recorre el copdigo y lo muestra la condicion es j<CS
        codOperacion= MemoriaPrincipal[j][3] & 0x1F;                            //aplico una mascara, para sacarle los ultimos 5 bits y asi tenes el codigo de operacion
        char operacion =MemoriaPrincipal[j][3];
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
                valorOPA[q]=MemoriaPrincipal[j][3];
                printf("%02X",valorOPA[q]);            
            }
        }
        else if (cant==02){                   //se fija cuantos bytes chupa cada operando
            OperandoB=(operacion >>6) & 0x03;
            for(int q=0;q<OperandoB;q++){               
                j++;                                         //parece qprimero viene el byte mas signfiquitaivo
                valorOPB[q]=MemoriaPrincipal[j][3]; 
            }
            OperandoA=(operacion >>4) & 0x03;
            for(int q=0;q<OperandoA;q++){ 
                j++;
                valorOPA[q]=MemoriaPrincipal[j][3];          
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

void AsignarSegmentos(char MemoriaPrincipal[][4],char TablaSegmentos[8],char Registros[][4]){             //aca deberia cargar la tabal de segmentos, pero me perdi

    TablaSegmentos[0]=MemoriaPrincipal[7][0]; //podria ser 7 y DS=(MemoriaPrincipal[5][3]<<8) | MemoriaPrincipal[6][3]][0] +7 //deberia ser asi o con q guarde el valor de i seria suficiente?
    TablaSegmentos[1]=MemoriaPrincipal[(MemoriaPrincipal[5][3]<<8) | MemoriaPrincipal[6][3]][0];

    Registros[26][0]=Registros[26][1]=Registros[26][2]=Registros[26][3]; //CS los primeros 16 bits apuntan a la posicion de la tabla de segmentos 0, y los otros van con 0
    Registros[27][1]=1;  // DS apunta a la posicion 01 y el resto 0
    Registros[27][2]=Registros[27][3]=Registros[27][0]=0;
    
}
void main(int argc, char argv[]){
    //con [][4] estarian separas byte a byte, sino 32 y estarian bit a bit
    char Registros[32][4];                      
    char MemoriaPrincipal[4096][4];            //mismo   16384 bytes tomados de a 4 
    char *TablaSegmentos[8];                  //0 cs, 1 ds,

/*
    // argv[0] = nombre del propio programa (ej: "./vmx"), siempre está
    // argv[1] = filename.vmx (obligatorio)
    // argv[2] = "-d" (opcional)
    if (argc < 2) {
        printf("Uso: vmx filename.vmx [-d]\n");
    }
    else{
        filename = argv[1];                 //no se tipo es esto (sam lo creo xd), comente toda la linea para poder probar
        int mostrar_disassembler = 0;

        if (argc >= 3 && strcmp(argv[2], "-d") == 0) {
            mostrar_disassembler = 1;
        }

        printf("Archivo a ejecutar: %s\n", filename);
        printf("Modo disassembler: %s\n", mostrar_disassembler ? "SI" : "NO");

        // Acá seguiría: abrir el archivo, leer la cabecera, cargar en memoria,
        // ejecutar (y si mostrar_disassembler, imprimir el disassembler)
    }
*/

    Lectura(MemoriaPrincipal);
    AsignarSegmentos(MemoriaPrincipal,TablaSegmentos,Registros);
    MostrarCodigo(MemoriaPrincipal);
    

}