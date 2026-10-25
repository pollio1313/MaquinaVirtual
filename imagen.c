#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "imagen.h"

// El main asigna gArchivoVMI y gTamMemoriaBytes antes de llamar a EjecutarMaquina.
char *gArchivoVMI = NULL;     // NULL = no se paso .vmi -> los breakpoints se ignoran
int gTamMemoriaBytes = 0;     // tamaño de la memoria principal en bytes
int gPasoAPaso = 0;           // 1 = despues de cada instruccion hay un nuevo breakpoint
int gBreakpointEjecutado = 0; // el ciclo lo apaga antes de cada instruccion; Breakpoint() lo prende

static void EscribirWordBE(FILE *f, uint16_t v)
{
    fputc((v >> 8) & 0xFF, f);
    fputc(v & 0xFF, f);
}

static void EscribirIntBE(FILE *f, uint32_t v)
{
    fputc((v >> 24) & 0xFF, f);
    fputc((v >> 16) & 0xFF, f);
    fputc((v >> 8) & 0xFF, f);
    fputc(v & 0xFF, f);
}

// Escribe el .vmi: header (8) + registros (32 x 4) + tabla de segmentos (8 x 4) + memoria
void GenerarImagen(unsigned char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[])
{
    FILE *f = fopen(gArchivoVMI, "wb");
    if (!f)
    {
        printf("[ERROR] No se pudo crear el archivo de imagen %s\n", gArchivoVMI);
        exit(1);
    }

    // header
    fwrite("VMI26", 1, 5, f);
    fputc(1, f); // version
    EscribirWordBE(f, (uint16_t)(gTamMemoriaBytes / 1024));

    // registros
    for (int i = 0; i < 32; i++)
        EscribirIntBE(f, (uint32_t)Registros[i]);

    // tabla de descriptores: base (2 bytes) + tamaño (2 bytes), las entradas sin usar quedan en 0xFFFF
    for (int i = 0; i < 8; i++)
    {
        EscribirWordBE(f, (uint16_t)TablaSegmentos[i].base);
        EscribirWordBE(f, (uint16_t)TablaSegmentos[i].size);
    }

    // memoria principal completa
    fwrite(MemoriaPrincipal, 1, gTamMemoriaBytes, f);
    fclose(f);
}

// SYS 0xF: no usa ni modifica registros (salvo lo que haga el usuario con g / q / Enter)
void Breakpoint(unsigned char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[])
{
    if (gArchivoVMI == NULL) // sin .vmi el breakpoint se ignora
        return;

    gBreakpointEjecutado = 1;
    GenerarImagen(MemoriaPrincipal, Registros, TablaSegmentos);

    char linea[64];
    while (1)
    {
        printf("[BREAKPOINT] g = continuar | q = salir | Enter = paso a paso: ");
        fflush(stdout);

        if (fgets(linea, sizeof(linea), stdin) == NULL) // EOF: se trata como 'q'
            exit(0);

        if (linea[0] == 'g')
        {
            gPasoAPaso = 0;
            return;
        }
        if (linea[0] == 'q')
            exit(0); // el .vmi queda como se genero recien
        if (linea[0] == '\n')
        {
            gPasoAPaso = 1;
            return;
        }
        // cualquier otra cosa: se vuelve a preguntar
    }
}
