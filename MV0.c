#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "imagen.h"
#include "instrucciones.h"

struct operando
{
    int cod;
    char *tipo;
    int cantBytes;
};

const struct operando tablaOperandos[] = {
    {0, "ninguno", 0},
    {1, "registro", 1},
    {2, "inmediato", 2},
    {3, "memoria", 3}};

struct Instruccion
{
    char *nombre; // esta sirve por ahrao, pero en realidad tendria que ser punteros a funciones con cada metodo
    char codigo;
    int cantOP;
    InstruccionFunc ejecutar;
};

const struct Instruccion tablaInstrucciones[] = {
    {"SYS", 0x00, 1, op_SYS},
    {"JMP", 0x01, 1, op_JMP},
    {"JP", 0x02, 1, op_JP},
    {"JN", 0x03, 1, op_JN},
    {"JZ", 0x04, 1, op_JZ},
    {"JC", 0x05, 1, op_JC},
    {"JV", 0x06, 1, op_JV},
    {"JNP", 0x07, 1, op_JNP},
    {"JNN", 0x08, 1, op_JNN},
    {"JNZ", 0x09, 1, op_JNZ},
    {"NOT", 0x0A, 1, op_NOT},
    {"---", 0x0B, 0, NULL},
    {"---", 0x0C, 0, NULL},
    {"---", 0x0D, 0, NULL},
    {"---", 0x0E, 0, NULL},
    {"STOP", 0x0F, 0, op_STOP},
    {"MOV", 0x10, 2, op_MOV},
    {"ADD", 0x11, 2, op_ADD},
    {"SUB", 0x12, 2, op_SUB},
    {"MUL", 0x13, 2, op_MUL},
    {"DIV", 0x14, 2, op_DIV},
    {"CMP", 0x15, 2, op_CMP},
    {"AND", 0x16, 2, op_AND},
    {"OR", 0x17, 2, op_OR},
    {"XOR", 0x18, 2, op_XOR},
    {"SWAP", 0x19, 2, op_SWAP},
    {"SHL", 0x1A, 2, op_SHL},
    {"SHR", 0x1B, 2, op_SHR},
    {"SAR", 0x1C, 2, op_SAR},
    {"LDL", 0x1D, 2, op_LDL},
    {"LDH", 0x1E, 2, op_LDH},
    {"RND", 0x1F, 2, op_RND}};

const char *tablaRegistros[32] = {
    // la tabla de registros, solamente devuelve el nombre, es apra la muestra, quizas habria q pasarla a estructura, para poner punteros a func
    "IP",        // 0
    "OPC",       // 1
    "OP1",       // 2
    "OP2",       // 3
    "LAR",       // 4
    "MAR",       // 5
    "MBR",       // 6
    "Reservado", // 7
    "Reservado", // 8
    "Reservado", // 9
    "EAX",       // 10
    "EBX",       // 11
    "ECX",       // 12
    "EDX",       // 13
    "EEX",       // 14
    "EFX",       // 15
    "AC",        // 16
    "CC",        // 17
    "Reservado", // 18
    "Reservado", // 19
    "Reservado", // 20
    "Reservado", // 21
    "Reservado", // 22
    "Reservado", // 23
    "Reservado", // 24
    "Reservado", // 25
    "CS",        // 26
    "DS",        // 27
    "Reservado", // 28
    "Reservado", // 29
    "Reservado", // 30
    "Programa"   // 31
};

// lee dos bytes en formato Big-Endian
static uint16_t LeerWordBE(FILE *arch)
{
    unsigned char b[2];
    if (fread(b, 1, 2, arch) != 2)
        return 0;
    return (uint16_t)((b[0] << 8) | b[1]);
}

int CargarProgramaVMX(const char *filename, unsigned char **memoria, int tamMemoriaBytes, HeaderVM *hdr, char *argvParams[], int cantParams)
{
    FILE *arch = fopen(filename, "rb");
    if (!arch)
    {
        printf("[ERROR] No se pudo abrir el archivo %s\n", filename);
        exit(1);
    }

    char firma[5];
    if (fread(firma, 1, 5, arch) != 5 || strncmp(firma, "VMX26", 5) != 0)
    {
        printf("[ERROR] Formato de archivo invalido (no es VMX26)\n");
        fclose(arch);
        exit(1);
    }

    unsigned char ver;
    fread(&ver, 1, 1, arch);
    hdr->version = ver;

    if (ver == 1)
    {
        hdr->tamCS = LeerWordBE(arch);
    }
    else if (ver == 2)
    {
        hdr->tamCS = LeerWordBE(arch);
        hdr->tamDS = LeerWordBE(arch);
        hdr->tamES = LeerWordBE(arch);
        hdr->tamSS = LeerWordBE(arch);
        hdr->tamKS = LeerWordBE(arch);
        hdr->entryPoint = LeerWordBE(arch);
    }
    else
    {
        printf("[ERROR] Version de archivo VMX no soportada: %d\n", ver);
        fclose(arch);
        exit(1);
    }

    // calcular tamaño de Param Segment si existen parámetros -p
    if (cantParams > 0)
    {
        int bytesStrings = 0;
        for (int i = 0; i < cantParams; i++)
            bytesStrings += strlen(argvParams[i]) + 1; // incluye '\0'

        int bytesPunteros = cantParams * 4; // arreglo argv de punteros de 4 bytes
        hdr->tamPS = bytesStrings + bytesPunteros;
    }
    else
    {
        hdr->tamPS = 0;
    }

    // validar si la memoria alcanza
    int totalProceso = hdr->tamPS + hdr->tamKS + hdr->tamCS + hdr->tamDS + hdr->tamES + hdr->tamSS;
    if (ver == 1)
    {
        totalProceso = hdr->tamCS; // En v1, DS ocupa el resto de la memoria disponible
    }

    if (totalProceso > tamMemoriaBytes)
    {
        printf("[ERROR] Memoria insuficiente para alojar el proceso (%d bytes requeridos, %d disponibles)\n",
               totalProceso, tamMemoriaBytes);
        fclose(arch);
        exit(1);
    }

    // cargar contenidos en memoria
    int baseFisicaPS = 0;
    int baseFisicaKS = baseFisicaPS + hdr->tamPS;
    int baseFisicaCS = baseFisicaKS + hdr->tamKS;

    // cargar Param Segment
    if (hdr->tamPS > 0)
    {
        int offsetStr = 0;
        int offsetArrayArgv = 0;
        for (int i = 0; i < cantParams; i++)
            offsetArrayArgv += strlen(argvParams[i]) + 1;

        for (int i = 0; i < cantParams; i++)
        {
            int len = strlen(argvParams[i]) + 1;
            for (int k = 0; k < len; k++)
            {
                (*memoria)[baseFisicaPS + offsetStr + k] = argvParams[i][k];
            }
            // escribir puntero en el arreglo argv (segmento 0, offset del string)
            int dirLogicaStr = (0 << 16) | (offsetStr & 0xFFFF);
            int posPtr = baseFisicaPS + offsetArrayArgv + (i * 4);
            (*memoria)[posPtr + 0] = (dirLogicaStr >> 24) & 0xFF;
            (*memoria)[posPtr + 1] = (dirLogicaStr >> 16) & 0xFF;
            (*memoria)[posPtr + 2] = (dirLogicaStr >> 8) & 0xFF;
            (*memoria)[posPtr + 3] = dirLogicaStr & 0xFF;

            offsetStr += len;
        }
    }

    // en v2: primero viene el CS y luego KS en el archivo
    if (ver == 2)
    {
        if (hdr->tamCS > 0)
            fread(&(*memoria)[baseFisicaCS], 1, hdr->tamCS, arch);
        if (hdr->tamKS > 0)
            fread(&(*memoria)[baseFisicaKS], 1, hdr->tamKS, arch);
    }
    else // en v1: solo CS
    {
        fread(&(*memoria)[0], 1, hdr->tamCS, arch);
    }

    fclose(arch);
    return 0;
}

void ConstruirTablaSegmentos(HeaderVM *hdr, Segmento TablaSegmentos[], int Registros[], int tamMemoriaBytes)
{
    Registros[26] = -1; // CS
    Registros[27] = -1; // DS
    Registros[28] = -1; // ES
    Registros[29] = -1; // SS
    Registros[30] = -1; // KS
    Registros[31] = -1; // PS

    for (int i = 0; i < 8; i++)
    {
        TablaSegmentos[i].base = -1;
        TablaSegmentos[i].size = -1;
    }

    if (hdr->version == 1)
    {
        TablaSegmentos[0].base = 0;
        TablaSegmentos[0].size = hdr->tamCS;

        TablaSegmentos[1].base = hdr->tamCS;
        TablaSegmentos[1].size = tamMemoriaBytes - hdr->tamCS;

        Registros[26] = 0 << 16;      // CS
        Registros[27] = 1 << 16;      // DS
        Registros[0] = Registros[26]; // IP = CS:0
        return;
    }

    // v2: orden fisico PS -> KS -> CS -> DS -> ES -> SS
    int entradaTabla = 0;
    int baseActual = 0;

    if (hdr->tamPS > 0)
    {
        TablaSegmentos[entradaTabla].base = baseActual;
        TablaSegmentos[entradaTabla].size = hdr->tamPS;
        Registros[31] = (entradaTabla << 16); // PS
        baseActual += hdr->tamPS;
        entradaTabla++;
    }

    if (hdr->tamKS > 0)
    {
        TablaSegmentos[entradaTabla].base = baseActual;
        TablaSegmentos[entradaTabla].size = hdr->tamKS;
        Registros[30] = (entradaTabla << 16); // KS
        baseActual += hdr->tamKS;
        entradaTabla++;
    }

    if (hdr->tamCS > 0)
    {
        TablaSegmentos[entradaTabla].base = baseActual;
        TablaSegmentos[entradaTabla].size = hdr->tamCS;
        Registros[26] = (entradaTabla << 16); // CS
        baseActual += hdr->tamCS;
        entradaTabla++;
    }

    if (hdr->tamDS > 0)
    {
        TablaSegmentos[entradaTabla].base = baseActual;
        TablaSegmentos[entradaTabla].size = hdr->tamDS;
        Registros[27] = (entradaTabla << 16); // DS
        baseActual += hdr->tamDS;
        entradaTabla++;
    }

    if (hdr->tamES > 0)
    {
        TablaSegmentos[entradaTabla].base = baseActual;
        TablaSegmentos[entradaTabla].size = hdr->tamES;
        Registros[28] = (entradaTabla << 16); // ES
        baseActual += hdr->tamES;
        entradaTabla++;
    }

    if (hdr->tamSS > 0)
    {
        TablaSegmentos[entradaTabla].base = baseActual;
        TablaSegmentos[entradaTabla].size = hdr->tamSS;
        Registros[29] = (entradaTabla << 16); // SS
        baseActual += hdr->tamSS;
        entradaTabla++;
    }

    // inicializar IP con el CS y el offset del entry point
    Registros[0] = (Registros[26] & 0xFFFF0000) | (hdr->entryPoint & 0xFFFF);
}

void MostrarBinario(char byte)
{ // esto es solo para hacer pruebas, esta prompeado
    for (int i = 7; i >= 0; i--)
    {
        printf("%d", (byte >> i) & 1);
    }
}

void MostrarCodigo(unsigned char *MemoriaPrincipal, char *cabecera, uint16_t LargoCod)
{
    int i = 0, j = 0, cant;
    char codOperacion, OperandoA, OperandoB, valorOPA[3] = {0}, valorOPB[3] = {0};
    printf("%.5s\n", cabecera);

    while (j < LargoCod)
    {                                              // este es el while q recorre el copdigo y lo muestra la condicion es j<CS
        codOperacion = MemoriaPrincipal[j] & 0x1F; // aplico una mascara, para sacarle los ultimos 5 bits y asi tenes el codigo de operacion
        char operacion = MemoriaPrincipal[j];
        cant = tablaInstrucciones[codOperacion].cantOP;
        /*
        printf("\n");
        MostrarBinario(operacion);
        printf("\n"); */
        char buffer[100] = "";
        int pos = 0;
        pos += snprintf(buffer + pos, sizeof(buffer) - pos, "[%04X] %02X ", j, (operacion & 0xFF)); // esta linea sumas al buffer la celda de mem y el binario de la operacion
        // printf("[%04X] %2X ",j,(operacion)&0xFF);

        if (cant == 01)
        {

            OperandoA = operacion >> 6 & 0x03;

            for (int q = 0; q < OperandoA; q++)
            { // while apra consumir los valores de operandos, si es 0 sigeun de largo ,uso operandoB y no cantBytes porq valen lo mismo
                j++;
                valorOPA[q] = MemoriaPrincipal[j];
                pos += snprintf(buffer + pos, sizeof(buffer) - pos, "%X", valorOPA[q] & 0xFF);
                // printf("%X", valorOPA[q]);
            }
            printf("%-30s", buffer);
            printf("| %s   ", tablaInstrucciones[codOperacion].nombre);
            for (int q = 0; q < OperandoA; q++)
            {
                printf("%X", valorOPA[q] & 0xFF);
            }
        }
        else if (cant == 02)
        { // se fija cuantos bytes chupa cada operando
            OperandoB = (operacion >> 6) & 0x03;
            for (int q = 0; q < OperandoB; q++)
            {
                j++; // parece qprimero viene el byte mas signfiquitaivo
                valorOPB[q] = MemoriaPrincipal[j];
                // printf("%02X ", valorOPB[q]);
                pos += snprintf(buffer + pos, sizeof(buffer) - pos, "%02X ", valorOPB[q] & 0xFF);
            }
            OperandoA = (operacion >> 4) & 0x03;
            for (int q = 0; q < OperandoA; q++)
            {
                j++;
                valorOPA[q] = MemoriaPrincipal[j];
                // printf("%02X ", valorOPA[q]);
                pos += snprintf(buffer + pos, sizeof(buffer) - pos, "%02X ", valorOPA[q] & 0xFF);
            }

            printf("%-30s", buffer);
            printf("| %s   ", tablaInstrucciones[codOperacion].nombre);
            if (OperandoA == 01)
            {
                printf("%s", tablaRegistros[valorOPA[0]]);
            }
            else if (OperandoA == 2)
            {
                for (int q = 0; q < OperandoA; q++)
                {
                    printf("%2X", valorOPA[q] & 0xFF);
                }
            }
            else
            {
                int offset = (valorOPA[0] << 8) | (valorOPA[1]);
                int registro = valorOPA[2] & 0x1F;

                if (registro == 0)
                {
                    printf("[%X]", offset);
                }
                else if (offset == 0)
                {
                    printf("[%s]", tablaRegistros[registro]);
                }
                else
                {
                    printf("[%s+%X]", tablaRegistros[registro], offset);
                }
            }
            printf(",");
            if (OperandoB == 01)
            {
                printf("%s", tablaRegistros[valorOPB[0]]);
            }
            else if (OperandoB == 2)
            {
                for (int q = 0; q < OperandoB; q++)
                {
                    printf("%X", valorOPB[q] & 0xFF);
                }
            }
            else
            {
                int offset = (valorOPB[0] << 8) | (valorOPB[1]);
                int registro = valorOPB[2] & 0x1F;

                if (registro == 0)
                {
                    printf("[%X]", offset);
                }
                else if (offset == 0)
                {
                    printf("[%s]", tablaRegistros[registro]);
                }
                else
                {
                    printf("[%s+%X]", tablaRegistros[registro], offset);
                }
            }
        }
        else if (cant == 00)
        { // esta linea esta de mas, porq ya deberian valer 0 de antes
            OperandoA = OperandoB = 0;
            printf("%-30s", buffer);
            printf("| %s   ", tablaInstrucciones[codOperacion].nombre);
        }
        j++;
        printf("\n");
    }
}

void EjecutarMaquina(unsigned char MemoriaPrincipal[], Segmento TablaSegmentos[], int Registros[])
{
    char codOperacion, operandoA, operandoB;
    int pos, cantOP;

    int segCS = (Registros[26] >> 16) & 0xFFFF;     //antes tomaba al cs en la pos 0 de la tabla de seg
    int baseCS = TablaSegmentos[segCS].base;
    int tamCS = TablaSegmentos[segCS].size;
    unsigned char *codigo = MemoriaPrincipal + baseCS;


    // Registros[0] = Registros[26]; // IP se inicia aputando a la primera instruccion del codigo, mismo valor que CS
    // el IP ya viene inicializado (entry point o imagen), por eso la comenté
   
   
    while ((Registros[0] & 0xFFFF) < tamCS)     //cambie el while a una ver mas simple aprovechando las 4 var que agregué arriba
    { // Se itera hasta que IP apunte fuera del limite del segmento de codigo
        pos = Registros[0] & 0xFFFF;
        codOperacion = Registros[1] = codigo[pos] & 0x1F; // decodifico y guardo el codigo limpio en OPC
        cantOP = tablaInstrucciones[codOperacion].cantOP;

        if (cantOP == 0)
        {
            Registros[2] = Registros[3] = 0;
        }

        if (cantOP == 1)
        {
            operandoA = (codigo[pos] >> 6) & 0x03;
            Registros[2] = operandoA << 24; // guardamos OP A en OP1 son de 32 bits

            if (operandoA == 1)
            {
                Registros[2] = Registros[2] | codigo[++pos];
            }
            else if (operandoA == 2)
            {
                Registros[2] = Registros[2] | (codigo[++pos] << 8);
                Registros[2] = Registros[2] | codigo[++pos];
            }
            else if (operandoA == 3)
            {
                Registros[2] = Registros[2] | (codigo[++pos] << 16);
                Registros[2] = Registros[2] | (codigo[++pos] << 8);
                Registros[2] = Registros[2] | codigo[++pos];
            }
        }

        if (cantOP == 2)
        {
            operandoB = (codigo[pos] >> 6) & 0x03;
            operandoA = (codigo[pos] >> 4) & 0x03;
            Registros[2] = operandoA << 24; // A en OP1
            Registros[3] = operandoB << 24; // B en OP2

            if (operandoB == 1)
            {
                Registros[3] = Registros[3] | codigo[++pos];
            }
            else if (operandoB == 2)
            {
                Registros[3] = Registros[3] | (codigo[++pos] << 8);
                Registros[3] = Registros[3] | codigo[++pos];
            }
            else if (operandoB == 3)
            {
                Registros[3] = Registros[3] | (codigo[++pos] << 16);
                Registros[3] = Registros[3] | (codigo[++pos] << 8);
                Registros[3] = Registros[3] | codigo[++pos];
            }

            if (operandoA == 1)
            {
                Registros[2] = Registros[2] | codigo[++pos];
            }
            else if (operandoA == 2)
            {
                Registros[2] = Registros[2] | (codigo[++pos] << 8);
                Registros[2] = Registros[2] | codigo[++pos];
            }
            else if (operandoA == 3)
            {
                Registros[2] = Registros[2] | (codigo[++pos] << 16);
                Registros[2] = Registros[2] | (codigo[++pos] << 8);
                Registros[2] = Registros[2] | codigo[++pos];
            }
        }

        Registros[0] = (Registros[0] & 0xFFFF0000) | ((pos + 1) & 0xFFFF); // IP -> siguiente instruccion
        /*En ese punto pos apunta al último byte de la instrucción (los ++pos fueron consumiendo los operandos), así que 
        pos + 1 es el offset de la siguiente. La línea conserva los 16 bits altos (el segmento) y reemplaza los bajos. 
        Tres razones para hacerlo antes y no después, como estaba:

        La imagen. SYS 0xF guarda los registros dentro de op_SYS, o sea, durante la ejecución. 
        Si el IP todavía apunta al SYS, al retomar desde el .vmi vuelve a ejecutar el mismo breakpoint, y así indefinidamente.

        CALL. El TP dice que al ejecutarse, el IP “ya se encuentra apuntando a la siguiente instrucción”, 
        porque eso es lo que se apila como dirección de retorno.
        
        Un bug latente del esquema anterior. 
        Detectabas el salto con if (Registros[0] == ipPrevio). 
        Un salto cuyo destino sea la propia instrucción (un JMP a sí mismo, por ejemplo) se confundía con “no hubo salto” y el IP avanzaba igual. 
        Ahora los saltos simplemente pisan el IP ya avanzado y no hace falta comparar nada, por eso desaparece ipPrevio.*/

        if (codOperacion < 0 || codOperacion > 0x1F || tablaInstrucciones[codOperacion].ejecutar == NULL)
        {
            printf("\n[ERROR] Instruccion invalida: 0x%02X en IP [%04X]\n", (unsigned char)codOperacion, pos);
            exit(1);
        }
        // aca elimine un else  -Simon(capo)

        // ejecucion
        gBreakpointEjecutado = 0;
        tablaInstrucciones[codOperacion].ejecutar(MemoriaPrincipal, Registros, TablaSegmentos);

        if (codOperacion == 0x0F) // STOP
            break;

        if (gPasoAPaso && !gBreakpointEjecutado) // modo paso a paso
            Breakpoint(MemoriaPrincipal, Registros, TablaSegmentos);
        
        /*Sacar el else. Ya está validado más arriba: si ejecutar fuera NULL, el programa aborta con “Instruccion invalida”. 
        Ese else nunca se alcanzaba.*/
    }
}

int main(int argc, char *argv[])
{
    char *filenameVMX = NULL;
    char *filenameVMI = NULL;
    int tamMemoriaKiB = 16; // Por defecto 16 KiB
    int mostrar_disassembler = 0;

    char *params[64];
    int cantParams = 0;

    // 1. parsear los argumentos de la linea de comandos
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-d") == 0)
        {
            mostrar_disassembler = 1;
        }
        else if (strncmp(argv[i], "m=", 2) == 0)
        {
            tamMemoriaKiB = atoi(argv[i] + 2);
            if (tamMemoriaKiB <= 0)
                tamMemoriaKiB = 16;
        }
        else if (strcmp(argv[i], "-p") == 0)
        {
            if (filenameVMX != NULL)
            {
                params[cantParams++] = filenameVMX;
            }

            for (int j = i + 1; j < argc; j++)
            {
                params[cantParams++] = argv[j];
            }

            break; // -p siempre va al final
        }
        else
        {
            // puede ser archivo .vmx o .vmi
            int len = strlen(argv[i]);
            if (len > 4 && strcasecmp(argv[i] + len - 4, ".vmx") == 0)
            {
                filenameVMX = argv[i];
            }
            else if (len > 4 && strcasecmp(argv[i] + len - 4, ".vmi") == 0)
            {
                filenameVMI = argv[i];
            }
        }
    }

    if (cantParams > 0 && params[0] != filenameVMX && filenameVMX != NULL)
    {
        // desplazamos los elementos para ubicar filenameVMX en params[0]
        for (int k = cantParams; k > 0; k--)
        {
            params[k] = params[k - 1];
        }
        params[0] = filenameVMX;
        cantParams++;
    }

    if (!filenameVMX && !filenameVMI)
    {
        printf("Uso: vmx [filename.vmx] [filename.vmi] [m=M] [-d] [-p param1 param2 ...]\n");
        return 1;
    }

    // si no hay .vmx, se ignoran los parámetros -p
    if (!filenameVMX)
    {
        cantParams = 0;
    }

    // 2. reserva de memoria dinámica
    int tamMemoriaBytes = tamMemoriaKiB * 1024;
    unsigned char *MemoriaPrincipal = (unsigned char *)malloc(tamMemoriaBytes * sizeof(unsigned char));

    if (!MemoriaPrincipal)
    {
        printf("[ERROR] No se pudo asignar memoria principal de %d KiB\n", tamMemoriaKiB);
        return 1;
    }

    for (int i = 0; i < tamMemoriaBytes; i++)
    {
        MemoriaPrincipal[i] = 0;
    }

    int Registros[32] = {0};
    Segmento TablaSegmentos[8];
    HeaderVM hdr = {0};

    // 3. carga del proceso
    if (filenameVMX)
    {
        printf("Archivo a ejecutar: %s\n", filenameVMX);
        printf("Memoria asignada: %d KiB (%d bytes)\n", tamMemoriaKiB, tamMemoriaBytes);
        printf("Modo disassembler: %s\n", mostrar_disassembler ? "SI" : "NO");

        CargarProgramaVMX(filenameVMX, &MemoriaPrincipal, tamMemoriaBytes, &hdr, params, cantParams);
        ConstruirTablaSegmentos(&hdr, TablaSegmentos, Registros, tamMemoriaBytes);

        // (etapa 2): inicializar Pila (SP, RET -1, argc, argv)
    }
    else if (filenameVMI)
    {
        // (Etapa 5): cargar imagen .vmi directamente
    }

    gArchivoVMI = filenameVMI;
    gTamMemoriaBytes = tamMemoriaBytes;

    EjecutarMaquina(MemoriaPrincipal, TablaSegmentos, Registros);

    // Liberar memoria
    free(MemoriaPrincipal);
    return 0;
}