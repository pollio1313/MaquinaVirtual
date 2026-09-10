#include <stdlib.h>
#include <stdio.h>

void main(){
    FILE *archivoVMX;
    char dato;


    archivoVMX=fopen("prueba1.vmx","r");        //abro el archivo vmx
    if (archivoVMX==NULL){
        printf("NO SE ABRIO CAPO");
    }
    else{
        while(fread(&dato,sizeof(dato),1,archivoVMX)==1){
            printf("%02X ",dato);                            // %X para hexa y 02 la cant de caracteres
        }


        fclose(archivoVMX);
    }

}