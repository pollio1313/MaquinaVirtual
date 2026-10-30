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
    {"PUSH", 0x0B, 1, op_PUSH},
    {"POP", 0x0C, 1, op_POP},
    {"CALL", 0x0D, 1, op_CALL},
    {"RET", 0x0E, 0, op_RET},
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
    "IP",        // 0
    "OPC",       // 1
    "OP1",       // 2
    "OP2",       // 3
    "LAR",       // 4
    "MAR",       // 5
    "MBR",       // 6
    "SP",        // 7  (era Reservado)
    "BP",        // 8  (era Reservado)
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
    "ES",        // 28 (era Reservado)
    "SS",        // 29 (era Reservado)
    "KS",        // 30 (era Reservado)
    "PS"         // 31 (era Programa)
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

const char *ObtenerNombreRegistro(unsigned char regByte)
{
    int cod = regByte & 0x1F;
    int sector = (regByte >> 5) & 0x03;

    // si es uno de los 6 registros de propósito general (EAX a EFX: 10 a 15)
    if (cod >= 10 && cod <= 15)
    {
        char letra = 'A' + (cod - 10);
        static char nombreReg[5];
        if (sector == 0) // EAX, EBX, etc.
            snprintf(nombreReg, sizeof(nombreReg), "E%cX", letra);
        else if (sector == 1) // AL, BL, etc.
            snprintf(nombreReg, sizeof(nombreReg), "%cL", letra);
        else if (sector == 2) // AH, BH, etc.
            snprintf(nombreReg, sizeof(nombreReg), "%cH", letra);
        else if (sector == 3) // AX, BX, etc.
            snprintf(nombreReg, sizeof(nombreReg), "%cX", letra);
        return nombreReg;
    }

    // para el resto de los registros (IP, SP, BP, CS, DS, etc.)
    if (cod < 32)
        return tablaRegistros[cod];

    return "DESCONOCIDO";
}

static void ImprimirOperando(int tipoOp, unsigned char bytes[])
{
    if (tipoOp == 1) // Registro
    {
        printf("%s", ObtenerNombreRegistro(bytes[0]));
    }
    else if (tipoOp == 2) // Inmediato (2 bytes Big-Endian)
    {
        short valInm = (short)((bytes[0] << 8) | bytes[1]);
        printf("%d", valInm); // o printf("0x%X", valInm & 0xFFFF);
    }
    else if (tipoOp == 3) // memoria (offset: 2 bytes, reg/modificador: 1 byte)
    {
        short offset = (short)((bytes[0] << 8) | bytes[1]);
        unsigned char modReg = bytes[2];
        int codReg = modReg & 0x1F;
        int cantMod = (modReg >> 5) & 0x07;

        // Prefijo segun el tamano
        char prefijo = 'l';
        if (cantMod == 1)
            prefijo = 'b';
        else if (cantMod == 2)
            prefijo = 'w';
        else if (cantMod == 4)
            prefijo = 'l';

        printf("%c[", prefijo);

        if (codReg == 0 && offset != 0) // sin registro base (o IP = 0), solo offset directo
        {
            printf("%d", offset);
        }
        else if (offset == 0) // solo registro base
        {
            printf("%s", ObtenerNombreRegistro(codReg));
        }
        else // registro base + offset
        {
            if (offset > 0)
                printf("%s+%d", ObtenerNombreRegistro(codReg), offset);
            else
                printf("%s%d", ObtenerNombreRegistro(codReg), offset);
        }
        printf("]");
    }
}

void MostrarCodigo(unsigned char *MemoriaPrincipal, char *cabecera, uint16_t LargoCod)
{
    int j = 0, cant;
    unsigned char codOperacion, OperandoA, OperandoB;
    unsigned char valorOPA[3] = {0}, valorOPB[3] = {0};

    printf("%.5s\n", cabecera);

    while (j < LargoCod)
    {
        int posInicio = j;
        unsigned char operacion = MemoriaPrincipal[j];
        codOperacion = operacion & 0x1F;
        cant = tablaInstrucciones[codOperacion].cantOP;

        char buffer[100] = "";
        int pos = 0;
        pos += snprintf(buffer + pos, sizeof(buffer) - pos, "[%04X] %02X ", posInicio, operacion);

        if (cant == 1)
        {
            OperandoA = (operacion >> 6) & 0x03;

            for (int q = 0; q < OperandoA; q++)
            {
                j++;
                valorOPA[q] = MemoriaPrincipal[j];
                pos += snprintf(buffer + pos, sizeof(buffer) - pos, "%02X ", valorOPA[q]);
            }

            printf("%-30s | %s ", buffer, tablaInstrucciones[codOperacion].nombre);
            ImprimirOperando(OperandoA, valorOPA);
        }
        else if (cant == 2)
        {
            // En memoria binaria primero vienen los bytes de OPB y luego OPA
            OperandoB = (operacion >> 6) & 0x03;
            for (int q = 0; q < OperandoB; q++)
            {
                j++;
                valorOPB[q] = MemoriaPrincipal[j];
                pos += snprintf(buffer + pos, sizeof(buffer) - pos, "%02X ", valorOPB[q]);
            }

            OperandoA = (operacion >> 4) & 0x03;
            for (int q = 0; q < OperandoA; q++)
            {
                j++;
                valorOPA[q] = MemoriaPrincipal[j];
                pos += snprintf(buffer + pos, sizeof(buffer) - pos, "%02X ", valorOPA[q]);
            }

            printf("%-30s | %s ", buffer, tablaInstrucciones[codOperacion].nombre);
            // Formato tradicional ASM: Destino (A), Origen (B)
            ImprimirOperando(OperandoA, valorOPA);
            printf(", ");
            ImprimirOperando(OperandoB, valorOPB);
        }
        else // cant == 0
        {
            printf("%-30s | %s", buffer, tablaInstrucciones[codOperacion].nombre);
        }

        j++;
        printf("\n");
    }
}

void EjecutarMaquina(unsigned char MemoriaPrincipal[], Segmento TablaSegmentos[], int Registros[])
{
    char codOperacion, operandoA, operandoB;
    int pos, cantOP;

    int segCS = (Registros[26] >> 16) & 0xFFFF; // antes tomaba al cs en la pos 0 de la tabla de seg
    int baseCS = TablaSegmentos[segCS].base;
    int tamCS = TablaSegmentos[segCS].size;
    unsigned char *codigo = MemoriaPrincipal + baseCS;

    // Registros[0] = Registros[26]; // IP se inicia aputando a la primera instruccion del codigo, mismo valor que CS
    // el IP ya viene inicializado (entry point o imagen), por eso la comenté

    while ((Registros[0] & 0xFFFF) < tamCS) // cambie el while a una ver mas simple aprovechando las 4 var que agregué arriba
    {                                       // Se itera hasta que IP apunte fuera del limite del segmento de codigo
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

    // parsear los argumentos de la linea de comandos
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

    // reserva de memoria dinámica
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

    // carga del proceso
    if (filenameVMX)
    {
        printf("Archivo a ejecutar: %s\n", filenameVMX);
        printf("Memoria asignada: %d KiB (%d bytes)\n", tamMemoriaKiB, tamMemoriaBytes);
        printf("Modo disassembler: %s\n", mostrar_disassembler ? "SI" : "NO");

        CargarProgramaVMX(filenameVMX, &MemoriaPrincipal, tamMemoriaBytes, &hdr, params, cantParams);
        ConstruirTablaSegmentos(&hdr, TablaSegmentos, Registros, tamMemoriaBytes);

        // (etapa 2): inicializar Pila (SP, RET -1, argc, argv)
        if (hdr.version == 2 && hdr.tamSS > 0)
        {
            int segSS = (Registros[29] >> 16) & 0xFFFF;
            int baseSS = TablaSegmentos[segSS].base;
            int tamSS = TablaSegmentos[segSS].size;

            // SP en direccion logica (segmento : offset)
            int offsetSP = tamSS;
            Registros[7] = (segSS << 16) | (offsetSP & 0xFFFF);

            // calcular puntero a *argv en Param Segment
            int ptrArgv = -1;
            if (cantParams > 0 && hdr.tamPS > 0)
            {
                int bytesStrings = 0;
                for (int p = 0; p < cantParams; p++)
                    bytesStrings += strlen(params[p]) + 1;

                ptrArgv = (0 << 16) | (bytesStrings & 0xFFFF);
            }

            int argcVal = cantParams;

            // apilar *argv
            offsetSP -= 4;
            int dir = baseSS + offsetSP;
            MemoriaPrincipal[dir + 0] = (ptrArgv >> 24) & 0xFF;
            MemoriaPrincipal[dir + 1] = (ptrArgv >> 16) & 0xFF;
            MemoriaPrincipal[dir + 2] = (ptrArgv >> 8) & 0xFF;
            MemoriaPrincipal[dir + 3] = ptrArgv & 0xFF;

            // apilar argc
            offsetSP -= 4;
            dir = baseSS + offsetSP;
            MemoriaPrincipal[dir + 0] = (argcVal >> 24) & 0xFF;
            MemoriaPrincipal[dir + 1] = (argcVal >> 16) & 0xFF;
            MemoriaPrincipal[dir + 2] = (argcVal >> 8) & 0xFF;
            MemoriaPrincipal[dir + 3] = argcVal & 0xFF;

            // apilar RET (-1 = 0xFFFFFFFF)
            offsetSP -= 4;
            dir = baseSS + offsetSP;
            MemoriaPrincipal[dir + 0] = 0xFF;
            MemoriaPrincipal[dir + 1] = 0xFF;
            MemoriaPrincipal[dir + 2] = 0xFF;
            MemoriaPrincipal[dir + 3] = 0xFF;

            // SP final
            Registros[7] = (segSS << 16) | (offsetSP & 0xFFFF);
        }
    }
    else if (filenameVMI)
    {
        FILE *archVMI = fopen(filenameVMI, "rb");
        if (!archVMI)
        {
            printf("[ERROR] No se pudo abrir el archivo de imagen %s\n", filenameVMI);
            free(MemoriaPrincipal);
            return 1;
        }

        // validar firma VMI26
        char firmaVMI[5];
        if (fread(firmaVMI, 1, 5, archVMI) != 5 || strncmp(firmaVMI, "VMI26", 5) != 0)
        {
            printf("[ERROR] Formato de imagen invalido (no es VMI26)\n");
            fclose(archVMI);
            free(MemoriaPrincipal);
            return 1;
        }

        // leer version y tamaño de memoria en KiB
        unsigned char verVMI;
        fread(&verVMI, 1, 1, archVMI);
        uint16_t memKiBArchivo = LeerWordBE(archVMI);

        int tamEsperadoBytes = memKiBArchivo * 1024;
        if (tamEsperadoBytes != tamMemoriaBytes)
        {
            // ajustar si la memoria difiere del parametro m=M
            unsigned char *memRealloc = (unsigned char *)realloc(MemoriaPrincipal, tamEsperadoBytes);
            if (!memRealloc)
            {
                printf("[ERROR] No se pudo reasignar memoria a %d KiB\n", memKiBArchivo);
                fclose(archVMI);
                free(MemoriaPrincipal);
                return 1;
            }
            MemoriaPrincipal = memRealloc;
            tamMemoriaBytes = tamEsperadoBytes;
        }

        // leer los 32 registros (Big-Endian de 4 bytes)
        for (int i = 0; i < 32; i++)
        {
            unsigned char b[4];
            fread(b, 1, 4, archVMI);
            Registros[i] = ((int)b[0] << 24) | ((int)b[1] << 16) | ((int)b[2] << 8) | (int)b[3];
        }

        // leer la tabla de descriptores de segmentos (8 entradas x 4 bytes)
        for (int i = 0; i < 8; i++)
        {
            TablaSegmentos[i].base = (short)LeerWordBE(archVMI);
            TablaSegmentos[i].size = (short)LeerWordBE(archVMI);
        }

        // leer la memoria principal completa
        fread(MemoriaPrincipal, 1, tamMemoriaBytes, archVMI);
        fclose(archVMI);
    }

    gArchivoVMI = filenameVMI;
    gTamMemoriaBytes = tamMemoriaBytes;

    if (mostrar_disassembler)
    {
        int segCS = (Registros[26] >> 16) & 0xFFFF;
        int baseCS = TablaSegmentos[segCS].base;
        int tamCS = TablaSegmentos[segCS].size;

        printf("--- INICIO DISASSEMBLER (segCS: %d, baseCS: %d, tamCS: %d) ---\n", segCS, baseCS, tamCS);
        fflush(stdout);

        MostrarCodigo(MemoriaPrincipal + baseCS, "VMX26", tamCS);
        fflush(stdout);

        printf("--- FIN DISASSEMBLER ---\n");
        fflush(stdout);

        free(MemoriaPrincipal);
        return 0;
    }

    EjecutarMaquina(MemoriaPrincipal, TablaSegmentos, Registros);

    // liberar memoria
    free(MemoriaPrincipal);
    return 0;
}