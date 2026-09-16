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

char posReg(char pos){             //totalmente incompleta, pero para acordarme, habria q hacerlo como la de instrucciones
    
}


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

void MostrarCodigo(char MemoriaPrincipal[][4]){
    int i,j,cant;
    char codOperacion,OperandoA,OperandoB,valorOPA[3]={0},valorOPB[3]={0};
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


void main(int argc, char argv[]){
    //con [][4] estarian separas byte a byte, sino 32 y estarian bit a bit
    char Registros[17][4];                      //para la proxima es de 32x32, ahora solo usamos 17//deberian ser booleanos?
    char MemoriaPrincipal[4096][4];            //mismo   16384 bytes tomados de a 4 
    char TablaSegmentos[8][4];                  //0 cs, 1 ds,


    // argv[0] = nombre del propio programa (ej: "./vmx"), siempre está
    // argv[1] = filename.vmx (obligatorio)
    // argv[2] = "-d" (opcional)
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
    }


    Lectura(MemoriaPrincipal);
    MostrarCodigo(MemoriaPrincipal);
    

}