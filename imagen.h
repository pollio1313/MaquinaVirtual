#ifndef IMAGEN_H
#define IMAGEN_H

#include "instrucciones.h" // para el tipo Segmento

// estado del breakpoint (se definen en imagen.c)
extern char *gArchivoVMI;       
extern int gTamMemoriaBytes;    
extern int gPasoAPaso;          
extern int gBreakpointEjecutado; 

void GenerarImagen(unsigned char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[]);
void Breakpoint(unsigned char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[]);

#endif
