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


void Lectura(unsigned char MemoriaPrincipal[][4]){
    FILE *archivoVMX;
    int i=0;

    archivoVMX=fopen("prueba1.vmx","r");        //abro el archivo vmx
    if (archivoVMX==NULL){
        printf("NO SE ABRIO CAPO");
    }
    else{
        while(fread(&MemoriaPrincipal[i][3],sizeof(unsigned char),1,archivoVMX)==1){
            MemoriaPrincipal[i][2]=MemoriaPrincipal[i][1]=MemoriaPrincipal[i][0]=0;
            i++;
        }
        fclose(archivoVMX);
    }
}

void MostrarCodigo(unsigned char MemoriaPrincipal[][4]){
    int i;
    unsigned char codOperacion,OperandoA,OperandoB;
    for( i=0;i<=4;i++){
        printf("%c",MemoriaPrincipal[i][3]);            //primeros 5 bytes el VMX
    }
    
    printf(" %02X",MemoriaPrincipal[i][3]);               //muestra la version
    i+=2;
    printf(" %02X%02X \n",MemoriaPrincipal[i-1][3],MemoriaPrincipal[i][3]);
    int largo = MemoriaPrincipal[i-1][3]+MemoriaPrincipal[i][3];                //esta suma taria bien?
    for (int j=i+1;j<=largo;j++){                                               //este es el for q recorre el copdigo y lo muestra
        codOperacion= MemoriaPrincipal[j][3] & 0x1F;                            //aplico una mascara, para sacarle los ultimos 5 bits y asi tenes el codigo de operacion
        if (tablaInstrucciones[codOperacion].cantOP==1){
            OperandoA=MemoriaPrincipal[j][3] >>6; 
        }
        else if (tablaInstrucciones[codOperacion].cantOP==1){                   //se fija cuantos bytes chupa cada operando
            OperandoB=MemoriaPrincipal[j][3] >>6;
            OperandoA=(MemoriaPrincipal[j][3] >>4) & 0x03;
        }
        //si es por ejemplo memoria, como se leeria? osea el operando es de tipo 11, lee el priemr byte y lo pone ---B luego el segundo --BB o como?
        printf("\n %s",tablaInstrucciones[codOperacion].nombre);
        
    }
}


void main(){
    //con [][4] estarian separas byte a byte, sino 32 y estarian bit a bit
    int Registros[17][4];                      //para la proxima es de 32x32, ahora solo usamos 17//deberian ser booleanos?
    unsigned char MemoriaPrincipal[4096][4];            //mismo   16384 bytes tomados de a 4 
    int TablaSegmentos[8][4];                  //0 cs, 1 ds,


    Lectura(MemoriaPrincipal);
    MostrarCodigo(MemoriaPrincipal);
    

}