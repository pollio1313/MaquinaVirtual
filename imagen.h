#ifndef IMAGEN_H
#define IMAGEN_H

#include "instrucciones.h" // para el tipo Segmento

// estado del breakpoint (se definen en imagen.c)
extern char *gArchivoVMI;       // NULL = no se paso .vmi
extern int gTamMemoriaBytes;    // tamaño de la memoria principal en bytes
extern int gPasoAPaso;          // 1 = nuevo breakpoint despues de cada instruccion
extern int gBreakpointEjecutado; // lo prende Breakpoint(), lo apaga el ciclo de ejecucion

void GenerarImagen(unsigned char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[]);
void Breakpoint(unsigned char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[]);

#endif
