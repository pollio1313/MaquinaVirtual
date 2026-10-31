#include <stdlib.h>
#include <stdio.h>
#include "instrucciones.h"
#include "imagen.h"
#include <limits.h>

// tres funciones auxiliares

int CalcularDireccionFisica(int operandoMemoria, unsigned char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[], int tamLecturaEscritura)
{
    short offset = (short)((operandoMemoria >> 8) & 0xFFFF);
    int codReg = operandoMemoria & 0x1F;
    int numSeg = (Registros[codReg] >> 16) & 0xFFFF;
    int offsetFinal = (Registros[codReg] & 0xFFFF) + offset;

    // validar existencia de segmento
    if (numSeg < 0 || numSeg >= 8 || TablaSegmentos[numSeg].size == 0xFFFF || (short)TablaSegmentos[numSeg].base == -1)
    {
        printf("\n[ERROR] Segmento invalido %d\n", numSeg);
        exit(1);
    }

    // validar limite respetando la cantidad de bytes a leer/escribir
    if (offsetFinal < 0 || (offsetFinal + tamLecturaEscritura) > TablaSegmentos[numSeg].size)
    {
        printf("\n[ERROR] Fuera del limite de segmento.\n");
        exit(1);
    }

    Registros[4] = (numSeg << 16) | (offsetFinal & 0xFFFF); // LAR
    int dirFisica = TablaSegmentos[numSeg].base + offsetFinal;
    Registros[5] = (tamLecturaEscritura << 16) | (dirFisica & 0xFFFF); // MAR
    return dirFisica;
}

int ObtenerValorOperando(int operando, unsigned char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[])
{
    int tipo = (operando >> 24) & 0xFF;
    int valor = operando & 0x00FFFFFF;

    if (tipo == 1) // registro
    {
        int codReg = valor & 0x1F;
        int sector = (valor >> 5) & 0x03;
        int valCompleto = Registros[codReg];

        if (sector == 0) // EAX (4 bytes)
            return valCompleto;
        else if (sector == 1) // AL (1 byte bajo con signo propagado a 32 bits)
            return (signed char)(valCompleto & 0xFF);
        else if (sector == 2) // AH (1 byte alto con signo propagado a 32 bits)
            return (signed char)((valCompleto >> 8) & 0xFF);
        else if (sector == 3) // AX (2 bytes con signo propagado a 32 bits)
            return (short)(valCompleto & 0xFFFF);
    }
    else if (tipo == 2) // inmediato (16 bits con signo)
    {
        return (short)(valor & 0xFFFF);
    }
    else if (tipo == 3) // Memoria
    {
        // los 3 bits más significativos del byte 0 indican tamaño (bits 5 a 7)
        int cantMod = (valor >> 5) & 0x07;
        int tamBytes = 4;
        if (cantMod == 1)
            tamBytes = 1; // b[...]
        else if (cantMod == 2)
            tamBytes = 2; // w[...]
        else if (cantMod == 4)
            tamBytes = 4; // l[...]

        int dirFisica = CalcularDireccionFisica(valor, MemoriaPrincipal, Registros, TablaSegmentos, tamBytes);

        if (tamBytes == 1)
        {
            Registros[6] = (unsigned char)MemoriaPrincipal[dirFisica];
            return (signed char)MemoriaPrincipal[dirFisica]; // propagación de signo a 32 bits
        }
        else if (tamBytes == 2)
        {
            Registros[6] = ((unsigned char)MemoriaPrincipal[dirFisica] << 8) |
                           (unsigned char)MemoriaPrincipal[dirFisica + 1];
            return (short)Registros[6]; // propagación de signo a 32 bits
        }
        else
        {
            Registros[6] = ((unsigned char)MemoriaPrincipal[dirFisica] << 24) |
                           ((unsigned char)MemoriaPrincipal[dirFisica + 1] << 16) |
                           ((unsigned char)MemoriaPrincipal[dirFisica + 2] << 8) |
                           (unsigned char)MemoriaPrincipal[dirFisica + 3];
            return Registros[6];
        }
    }
    return 0;
}

void GuardarDestino(int operandoDestino, int resultado, unsigned char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[])
{
    int tipo = (operandoDestino >> 24) & 0xFF;
    int valor = operandoDestino & 0x00FFFFFF;

    if (tipo == 1) // registro
    {
        int codReg = valor & 0x1F;
        int sector = (valor >> 5) & 0x03;

        if (sector == 0) // EAX completo
        {
            Registros[codReg] = resultado;
        }
        else if (sector == 1) // AL (modifica solo el byte 4, conservando los primeros 3)
        {
            Registros[codReg] = (Registros[codReg] & 0xFFFFFF00) | (resultado & 0xFF);
        }
        else if (sector == 2) // AH (modifica solo el byte 3)
        {
            Registros[codReg] = (Registros[codReg] & 0xFFFF00FF) | ((resultado & 0xFF) << 8);
        }
        else if (sector == 3) // AX (modifica los 2 bytes bajos)
        {
            Registros[codReg] = (Registros[codReg] & 0xFFFF0000) | (resultado & 0xFFFF);
        }
    }
    else if (tipo == 3) // memoria
    {
        int cantMod = (valor >> 5) & 0x07;
        int tamBytes = 4;
        if (cantMod == 1)
            tamBytes = 1;
        else if (cantMod == 2)
            tamBytes = 2;
        else if (cantMod == 4)
            tamBytes = 4;

        int dirFisica = CalcularDireccionFisica(valor, MemoriaPrincipal, Registros, TablaSegmentos, tamBytes);
        Registros[6] = resultado; // MBR

        if (tamBytes == 1)
        {
            MemoriaPrincipal[dirFisica] = resultado & 0xFF;
        }
        else if (tamBytes == 2)
        {
            MemoriaPrincipal[dirFisica + 0] = (resultado >> 8) & 0xFF;
            MemoriaPrincipal[dirFisica + 1] = resultado & 0xFF;
        }
        else
        {
            MemoriaPrincipal[dirFisica + 0] = (resultado >> 24) & 0xFF;
            MemoriaPrincipal[dirFisica + 1] = (resultado >> 16) & 0xFF;
            MemoriaPrincipal[dirFisica + 2] = (resultado >> 8) & 0xFF;
            MemoriaPrincipal[dirFisica + 3] = resultado & 0xFF;
        }
    }
    else
    {
        printf("\n[ERROR] Operando de destino invalido: tipo %d\n", tipo);
        exit(1);
    }
}

void ImprimirBinario(int valor, int tamByte)
{
    printf("0b");
    for (int i = (tamByte * 8) - 1; i >= 0; i--)
        printf("%d", (valor >> i) & 1);
}

static void SaltarA(int Registros[], int destino)
{
    Registros[0] = (Registros[26] & 0xFFFF0000) | (destino & 0xFFFF);
}

// todas las operaciones desarrolladas
void op_SYS(unsigned char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[])
{
    int dirLogica = Registros[13]; // EDX: puntero de inicio / dirección lógica
    int subcodigo = ObtenerValorOperando(Registros[2], MemoriaPrincipal, Registros, TablaSegmentos);

    // 1. SYS 0xF: BREAKPOINT
    if (subcodigo == 0xF)
    {
        Breakpoint(MemoriaPrincipal, Registros, TablaSegmentos);
        return;
    }

    // 2. SYS 0x00: CLEAR SCREEN
    if (subcodigo == 0x00)
    {
        printf("\033[H\033[J");
        return;
    }

    // 3. SYS 0x03: STRING READ
    if (subcodigo == 0x03)
    {
        int numSeg = (dirLogica >> 16) & 0xFFFF;
        int offset = dirLogica & 0xFFFF;

        if (numSeg < 0 || numSeg >= 8 || TablaSegmentos[numSeg].size == 0xFFFF || (short)TablaSegmentos[numSeg].base == -1)
        {
            printf("\n[ERROR] Segmento invalido en SYS 0x03: %d\n", numSeg);
            exit(1);
        }

        int maxChars = Registros[12]; // ECX: limite
        int leidos = 0;
        char buffer[1024];

        if (fgets(buffer, sizeof(buffer), stdin) != NULL)
        {
            for (int k = 0; buffer[k] != '\0' && buffer[k] != '\n'; k++)
            {
                if (maxChars != -1 && leidos >= maxChars)
                    break;

                if ((offset + leidos) >= TablaSegmentos[numSeg].size)
                {
                    printf("\n[ERROR] Fuera del limite de segmento en SYS 0x03\n");
                    exit(1);
                }

                int dirFisica = TablaSegmentos[numSeg].base + offset + leidos;
                MemoriaPrincipal[dirFisica] = buffer[k];
                leidos++;
            }
        }

        if ((offset + leidos) >= TablaSegmentos[numSeg].size)
        {
            printf("\n[ERROR] Fuera del limite de segmento en SYS 0x03 al escribir terminador\n");
            exit(1);
        }
        MemoriaPrincipal[TablaSegmentos[numSeg].base + offset + leidos] = '\0';
        return;
    }

    // 4. SYS 0x04: STRING WRITE
    if (subcodigo == 0x04)
    {
        int numSeg = (dirLogica >> 16) & 0xFFFF;
        int offset = dirLogica & 0xFFFF;

        if (numSeg < 0 || numSeg >= 8 || TablaSegmentos[numSeg].size == 0xFFFF || (short)TablaSegmentos[numSeg].base == -1)
        {
            printf("\n[ERROR] Segmento invalido en SYS 0x04: %d\n", numSeg);
            exit(1);
        }

        char buffer[1024];
        int indice = 0;

        while (1)
        {
            if (offset >= TablaSegmentos[numSeg].size)
            {
                printf("\n[ERROR] Fuera del limite de segmento en SYS 0x04\n");
                exit(1);
            }

            int dirFisica = TablaSegmentos[numSeg].base + offset;
            char c = (char)MemoriaPrincipal[dirFisica];

            if (c == '\0')
                break;

            buffer[indice++] = c;
            offset++;

            // si se llena el buffer local, lo vaciamos a pantalla con printf
            if (indice >= (int)(sizeof(buffer) - 1))
            {
                buffer[indice] = '\0';
                printf("%s", buffer);
                indice = 0;
            }
        }

        // imprimir lo que haya quedado
        if (indice > 0)
        {
            buffer[indice] = '\0';
            printf("%s", buffer);
        }

        return;
    }

    // SYS 0x01 y 0x02: READ y WRITE numerico / celdas multiples (Parte I)
    int modo = Registros[10];                      // EAX: formato
    int cantCeldas = Registros[12] & 0xFFFF;       // ECX bajo: cantidad
    int tamBytes = (Registros[12] >> 16) & 0xFFFF; // ECX alto: tamaño en bytes

    if (tamBytes <= 0)
        tamBytes = 4;

    for (int i = 0; i < cantCeldas; i++)
    {
        int numSeg = (dirLogica >> 16) & 0xFFFF;
        int offset = (dirLogica & 0xFFFF) + (i * tamBytes);

        if (numSeg < 0 || numSeg >= 8 || TablaSegmentos[numSeg].size == 0xFFFF || (short)TablaSegmentos[numSeg].base == -1)
        {
            printf("\n[ERROR] Fallo de segmento en SYS (Segmento invalido %d)\n", numSeg);
            exit(1);
        }
        if (offset < 0 || (offset + tamBytes) > TablaSegmentos[numSeg].size)
        {
            printf("\n[ERROR] Fallo de segmento en SYS (Fuera de limite)\n");
            exit(1);
        }

        int dirFisica = TablaSegmentos[numSeg].base + offset;
        Registros[4] = (numSeg << 16) | (offset & 0xFFFF);      // LAR
        Registros[5] = (tamBytes << 16) | (dirFisica & 0xFFFF); // MAR

        printf("[%04X]: ", dirFisica);

        if (subcodigo == 1) // READ
        {
            int dato = 0;
            if (modo & 0x02) // Caracter
            {
                char c;
                scanf(" %c", &c);
                dato = c;
            }
            else if (modo & 0x08) // Hexa
                scanf("%x", &dato);
            else if (modo & 0x04) // Octal
                scanf("%o", &dato);
            else if (modo & 0x10) // Binario
            {
                char binStr[35];
                scanf("%s", binStr);
                int inicio = 0;
                if (binStr[0] == '0' && (binStr[1] == 'b' || binStr[1] == 'B'))
                    inicio = 2;
                for (int j = inicio; binStr[j] != '\0'; j++)
                    if (binStr[j] == '0' || binStr[j] == '1')
                        dato = (dato << 1) | (binStr[j] - '0');
            }
            else // Decimal
                scanf("%d", &dato);

            Registros[6] = dato; // MBR

            for (int j = 0; j < tamBytes; j++)
                MemoriaPrincipal[dirFisica + j] = (dato >> (8 * (tamBytes - 1 - j))) & 0xFF;
        }
        else if (subcodigo == 2) // WRITE
        {
            int valor = 0;
            for (int j = 0; j < tamBytes; j++)
                valor = (valor << 8) | (unsigned char)MemoriaPrincipal[dirFisica + j];

            Registros[6] = valor; // MBR

            if (modo & 0x10)
                ImprimirBinario(valor, tamBytes);
            if (modo & 0x08)
                printf(" 0x%X ", valor);
            if (modo & 0x04)
                printf(" 0o%o ", valor);
            if (modo & 0x02)
            {
                for (int j = 0; j < tamBytes; j++)
                {
                    char c = (valor >> (8 * (tamBytes - 1 - j))) & 0xFF;
                    printf(" %c ", c);
                }
            }
            if (modo & 0x01)
            {
                if (tamBytes == 1 && (valor & 0x80))
                    valor |= 0xFFFFFF00;
                else if (tamBytes == 2 && (valor & 0x8000))
                    valor |= 0xFFFF0000;

                printf(" %d ", valor);
            }
            printf("\n");
        }
    }
}

void op_STOP(unsigned char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[])
{
    Registros[0] = -1;
}
void op_MOV(unsigned char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[])
{
    int valorOrigen = ObtenerValorOperando(Registros[3], MemoriaPrincipal, Registros, TablaSegmentos);
    Registros[17] = 0;
    if (valorOrigen == 0)
        Registros[17] = Registros[17] | (1 << 30); // prende Z
    if (valorOrigen < 0)
        Registros[17] = Registros[17] | (1 << 31); // prende N
    GuardarDestino(Registros[2], valorOrigen, MemoriaPrincipal, Registros, TablaSegmentos);
}
// despues en add y otras funciones volvemos a usar las auxiliares,

void op_ADD(unsigned char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[])
{
    int valorA = ObtenerValorOperando(Registros[2], MemoriaPrincipal, Registros, TablaSegmentos);
    int valorB = ObtenerValorOperando(Registros[3], MemoriaPrincipal, Registros, TablaSegmentos);
    int resultado = valorA + valorB;

    Registros[17] = 0; // iniciamos CC
    // ayudamemoria xd
    //  bit 32 N bit 31 Z bit 30 C (acarreo) bit 29 V (desbordamiento) demas 28 bits reservados
    if (resultado == 0)
        Registros[17] = Registros[17] | (1 << 30); // prende Z
    if (resultado < 0)
        Registros[17] = Registros[17] | (1 << 31);                                                   // prende N
    if ((valorA > 0 && valorB > 0 && resultado < 0) || (valorA < 0 && valorB < 0 && resultado >= 0)) // casos de desbordamiento
        Registros[17] = Registros[17] | (1 << 28);                                                   // Prende V
    if ((unsigned int)valorA + (unsigned int)valorB < (unsigned int)valorA)
        Registros[17] = Registros[17] | (1 << 29);                                        // Prende C
    GuardarDestino(Registros[2], resultado, MemoriaPrincipal, Registros, TablaSegmentos); // guardamos en op1
}

void op_SUB(unsigned char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[])
{
    int valorA = ObtenerValorOperando(Registros[2], MemoriaPrincipal, Registros, TablaSegmentos);
    int valorB = ObtenerValorOperando(Registros[3], MemoriaPrincipal, Registros, TablaSegmentos);
    int resultado = valorA - valorB;
    Registros[17] = 0; // iniciamos CC
    //  bit 32 N bit 31 Z bit 30 C (acarreo) bit 29 V (desbordamiento) demas 28 bits reservados
    if (resultado == 0)
        Registros[17] = Registros[17] | (1 << 30); // prende Z
    if (resultado < 0)
        Registros[17] = Registros[17] | (1 << 31);                                                   // prende N
    if ((valorA > 0 && valorB < 0 && resultado < 0) || (valorA < 0 && valorB > 0 && resultado >= 0)) // casos de desbordamiento
        Registros[17] = Registros[17] | (1 << 28);                                                   // Prende V
    if ((unsigned int)valorA < (unsigned int)valorB)
        Registros[17] = Registros[17] | (1 << 29); // Prende C
    GuardarDestino(Registros[2], resultado, MemoriaPrincipal, Registros, TablaSegmentos);
}

void op_MUL(unsigned char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[])
{
    int valorA = ObtenerValorOperando(Registros[2], MemoriaPrincipal, Registros, TablaSegmentos);
    int valorB = ObtenerValorOperando(Registros[3], MemoriaPrincipal, Registros, TablaSegmentos);
    long long resultadoOF = (long long)valorA * (long long)valorB; // long long maneja 64 bits
    int resultado = (int)resultadoOF;                              // truncado
    Registros[17] = 0;                                             // iniciamos CC
    //  bit 32 N bit 31 Z bit 30 C (acarreo) bit 29 V (desbordamiento) demas 28 bits reservados
    if (resultado == 0)
        Registros[17] = Registros[17] | (1 << 30); // prende Z
    if (resultado < 0)
        Registros[17] = Registros[17] | (1 << 31); // prende N
    if (resultadoOF != (long long)resultado)       // si el de 64bits es != del de 32 desbordo
    {
        Registros[17] = Registros[17] | (1 << 28); // Prende V
        Registros[17] = Registros[17] | (1 << 29); // Prende C
    }
    GuardarDestino(Registros[2], resultado, MemoriaPrincipal, Registros, TablaSegmentos);
}

void op_DIV(unsigned char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[])
{
    int valorA = ObtenerValorOperando(Registros[2], MemoriaPrincipal, Registros, TablaSegmentos);
    int valorB = ObtenerValorOperando(Registros[3], MemoriaPrincipal, Registros, TablaSegmentos);
    Registros[17] = 0; // iniciamos CC
    if (valorB != 0)
    {
        int resultado = valorA / valorB;
        Registros[16] = valorA % valorB; // guardar el resto en AC
        //  bit 32 N bit 31 Z bit 30 C (acarreo) bit 29 V (desbordamiento) demas 28 bits reservados
        if (resultado == 0)
            Registros[17] = Registros[17] | (1 << 30); // prende Z
        if (resultado < 0)
            Registros[17] = Registros[17] | (1 << 31); // prende N
        if (valorA == INT_MIN && valorB == -1)
        {
            Registros[17] = Registros[17] | (1 << 28); // Prende V
            Registros[17] = Registros[17] | (1 << 29); // Prende C
        }
        GuardarDestino(Registros[2], resultado, MemoriaPrincipal, Registros, TablaSegmentos);
    }
    else
    {
        printf("\n[ERROR] Division por cero\n");
        exit(1);
    }
}

void op_AND(unsigned char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[])
{
    int valorA = ObtenerValorOperando(Registros[2], MemoriaPrincipal, Registros, TablaSegmentos);
    int valorB = ObtenerValorOperando(Registros[3], MemoriaPrincipal, Registros, TablaSegmentos);
    int resultado = valorA & valorB;
    Registros[17] = 0; // iniciamos CC
    // ayudamemoria xd
    //  bit 32 N bit 31 Z bit 30 C (acarreo) bit 29 V (desbordamiento) demas 28 bits reservados
    if (resultado == 0)
        Registros[17] = Registros[17] | (1 << 30); // prende Z
    if (resultado < 0)
        Registros[17] = Registros[17] | (1 << 31); // prende N

    GuardarDestino(Registros[2], resultado, MemoriaPrincipal, Registros, TablaSegmentos);
}

void op_OR(unsigned char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[])
{
    int valorA = ObtenerValorOperando(Registros[2], MemoriaPrincipal, Registros, TablaSegmentos);
    int valorB = ObtenerValorOperando(Registros[3], MemoriaPrincipal, Registros, TablaSegmentos);
    int resultado = valorA | valorB;
    Registros[17] = 0; // iniciamos CC
    // ayudamemoria xd
    //  bit 32 N bit 31 Z bit 30 C (acarreo) bit 29 V (desbordamiento) demas 28 bits reservados
    if (resultado == 0)
        Registros[17] = Registros[17] | (1 << 30); // prende Z
    if (resultado < 0)
        Registros[17] = Registros[17] | (1 << 31); // prende N

    GuardarDestino(Registros[2], resultado, MemoriaPrincipal, Registros, TablaSegmentos);
}
void op_XOR(unsigned char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[])
{
    int valorA = ObtenerValorOperando(Registros[2], MemoriaPrincipal, Registros, TablaSegmentos);
    int valorB = ObtenerValorOperando(Registros[3], MemoriaPrincipal, Registros, TablaSegmentos);
    int resultado = valorA ^ valorB;
    Registros[17] = 0; // iniciamos CC
    // ayudamemoria xd
    //  bit 32 N bit 31 Z bit 30 C (acarreo) bit 29 V (desbordamiento) demas 28 bits reservados
    if (resultado == 0)
        Registros[17] = Registros[17] | (1 << 30); // prende Z
    if (resultado < 0)
        Registros[17] = Registros[17] | (1 << 31); // prende N
    GuardarDestino(Registros[2], resultado, MemoriaPrincipal, Registros, TablaSegmentos);
}

void op_SHL(unsigned char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[])
{
    int valorA = ObtenerValorOperando(Registros[2], MemoriaPrincipal, Registros, TablaSegmentos);
    int valorB = ObtenerValorOperando(Registros[3], MemoriaPrincipal, Registros, TablaSegmentos);
    Registros[17] = 0; // iniciamos CC

    if (valorB >= 0 && valorB <= 31)
    {
        if (valorB > 0)
        {
            int bitCaido = ((unsigned int)valorA >> (32 - valorB)) & 1;
            if (bitCaido)                                  // acarreo
                Registros[17] = Registros[17] | (1 << 29); // prende C
        }

        int resultado = valorA << valorB;
        //  bit 32 N bit 31 Z bit 30 C (acarreo) bit 29 V (desbordamiento) demas 28 bits reservados
        if (resultado == 0)
            Registros[17] = Registros[17] | (1 << 30); // prende Z
        if (resultado < 0)
            Registros[17] = Registros[17] | (1 << 31); // prende N

        GuardarDestino(Registros[2], resultado, MemoriaPrincipal, Registros, TablaSegmentos);
    }
}
void op_SHR(unsigned char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[])
{
    int valorA = ObtenerValorOperando(Registros[2], MemoriaPrincipal, Registros, TablaSegmentos);
    int valorB = ObtenerValorOperando(Registros[3], MemoriaPrincipal, Registros, TablaSegmentos);
    Registros[17] = 0; // iniciamos CC

    if (valorB >= 0 && valorB <= 31)
    {
        if (valorB > 0)
        {
            int bitCaido = ((unsigned int)valorA >> (valorB - 1)) & 1;
            if (bitCaido)                                  // acarreo
                Registros[17] = Registros[17] | (1 << 29); // prende C
        }
        int resultado = (unsigned int)valorA >> valorB;
        if (resultado == 0)
            Registros[17] = Registros[17] | (1 << 30); // prende Z
        if (resultado < 0)
            Registros[17] = Registros[17] | (1 << 31); // prende N

        GuardarDestino(Registros[2], resultado, MemoriaPrincipal, Registros, TablaSegmentos);
    }
}
void op_SAR(unsigned char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[])
{
    int valorA = ObtenerValorOperando(Registros[2], MemoriaPrincipal, Registros, TablaSegmentos);
    int valorB = ObtenerValorOperando(Registros[3], MemoriaPrincipal, Registros, TablaSegmentos);
    Registros[17] = 0; // iniciamos CC
    if (valorB > 0 && valorB < 32)
    {
        if (valorB > 0)
        {
            int bitCaido = ((unsigned int)valorA >> (valorB - 1)) & 1;
            if (bitCaido)                                  // acarreo
                Registros[17] = Registros[17] | (1 << 29); // prende C
        }
        int resultado = valorA >> valorB;
        if (resultado == 0)
            Registros[17] = Registros[17] | (1 << 30); // prende Z
        if (resultado < 0)
            Registros[17] = Registros[17] | (1 << 31); // prende N
        GuardarDestino(Registros[2], resultado, MemoriaPrincipal, Registros, TablaSegmentos);
    }
}

void op_CMP(unsigned char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[])
{
    int valorA = ObtenerValorOperando(Registros[2], MemoriaPrincipal, Registros, TablaSegmentos);
    int valorB = ObtenerValorOperando(Registros[3], MemoriaPrincipal, Registros, TablaSegmentos);
    int resultado = valorA - valorB;
    Registros[17] = 0; // iniciamos CC
    if (resultado == 0)
        Registros[17] = Registros[17] | (1 << 30); // prende Z
    if (resultado < 0)
        Registros[17] = Registros[17] | (1 << 31);                                                   // prende N
    if ((valorA > 0 && valorB < 0 && resultado < 0) || (valorA < 0 && valorB > 0 && resultado >= 0)) // casos de desbordamiento
        Registros[17] = Registros[17] | (1 << 28);                                                   // Prende V
    if ((unsigned int)valorA < (unsigned int)valorB)
        Registros[17] = Registros[17] | (1 << 29); // Prende C
}

void op_JC(unsigned char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[])
{
    int valorA = ObtenerValorOperando(Registros[2], MemoriaPrincipal, Registros, TablaSegmentos);
    if ((Registros[17] >> 29) & 1)
        SaltarA(Registros, valorA);
}

void op_JMP(unsigned char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[])
{
    SaltarA(Registros, ObtenerValorOperando(Registros[2], MemoriaPrincipal, Registros, TablaSegmentos));
}

void op_JN(unsigned char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[])
{
    int valorA = ObtenerValorOperando(Registros[2], MemoriaPrincipal, Registros, TablaSegmentos);
    if ((Registros[17] >> 31) & 1) // esta prendido N?
        SaltarA(Registros, valorA);
}

void op_JNP(unsigned char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[])
{
    int valorA = ObtenerValorOperando(Registros[2], MemoriaPrincipal, Registros, TablaSegmentos);
    if (((Registros[17] >> 31) & 1) || ((Registros[17] >> 30) & 1)) // esta prendido N y Z?
        SaltarA(Registros, valorA);
}

void op_JNN(unsigned char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[])
{
    int valorA = ObtenerValorOperando(Registros[2], MemoriaPrincipal, Registros, TablaSegmentos);
    if (!((Registros[17] >> 31) & 1))
        SaltarA(Registros, valorA);
}
void op_JNZ(unsigned char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[])
{
    int valorA = ObtenerValorOperando(Registros[2], MemoriaPrincipal, Registros, TablaSegmentos);
    if (!((Registros[17] >> 30) & 1))
        SaltarA(Registros, valorA);
}
void op_JP(unsigned char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[])
{
    int valorA = ObtenerValorOperando(Registros[2], MemoriaPrincipal, Registros, TablaSegmentos);
    if (!((Registros[17] >> 31) & 1) && !((Registros[17] >> 30) & 1))
        SaltarA(Registros, valorA);
}
void op_JV(unsigned char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[])
{
    int valorA = ObtenerValorOperando(Registros[2], MemoriaPrincipal, Registros, TablaSegmentos);
    if ((Registros[17] >> 28) & 1)
        SaltarA(Registros, valorA);
}
void op_JZ(unsigned char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[])
{
    int valorA = ObtenerValorOperando(Registros[2], MemoriaPrincipal, Registros, TablaSegmentos);
    if ((Registros[17] >> 30) & 1)
        SaltarA(Registros, valorA);
}

void op_LDH(unsigned char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[])
{
    int valorA = ObtenerValorOperando(Registros[3], MemoriaPrincipal, Registros, TablaSegmentos);
    int tipoDestino = (Registros[2] >> 24) & 0xFF;

    if (tipoDestino == 1) // Registro
    {
        int destino = Registros[2] & 0x1F;
        Registros[destino] = (Registros[destino] & 0xFFFF) | ((valorA & 0xFFFF) << 16);
    }
    else if (tipoDestino == 3) // Memoria
    {
        int valorActual = ObtenerValorOperando(Registros[2], MemoriaPrincipal, Registros, TablaSegmentos);
        int nuevoValor = (valorActual & 0xFFFF) | ((valorA & 0xFFFF) << 16);
        GuardarDestino(Registros[2], nuevoValor, MemoriaPrincipal, Registros, TablaSegmentos);
    }
}

void op_LDL(unsigned char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[])
{
    int valorA = ObtenerValorOperando(Registros[3], MemoriaPrincipal, Registros, TablaSegmentos);
    int tipoDestino = (Registros[2] >> 24) & 0xFF;

    if (tipoDestino == 1) // Registro
    {
        int destino = Registros[2] & 0x1F;
        Registros[destino] = (Registros[destino] & 0xFFFF0000) | (valorA & 0xFFFF);
    }
    else if (tipoDestino == 3) // Memoria
    {
        int valorActual = ObtenerValorOperando(Registros[2], MemoriaPrincipal, Registros, TablaSegmentos);
        int nuevoValor = (valorActual & 0xFFFF0000) | (valorA & 0xFFFF);
        GuardarDestino(Registros[2], nuevoValor, MemoriaPrincipal, Registros, TablaSegmentos);
    }
}
void op_NOT(unsigned char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[])
{
    int valorA = ObtenerValorOperando(Registros[2], MemoriaPrincipal, Registros, TablaSegmentos);
    int resultado = ~valorA; // inverso bit a bit
    Registros[17] = 0;       // iniciamos CC
    if (resultado == 0)
        Registros[17] = Registros[17] | (1 << 30); // prende Z
    if (resultado < 0)
        Registros[17] = Registros[17] | (1 << 31); // prende N
    GuardarDestino(Registros[2], resultado, MemoriaPrincipal, Registros, TablaSegmentos);
}
void op_RND(unsigned char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[])
{
    int limite = ObtenerValorOperando(Registros[3], MemoriaPrincipal, Registros, TablaSegmentos); // el segundo operando es el limite
    int resultado = rand();                                                                       // numero aleatorio

    if (limite > 0)
        resultado = rand() % (limite + 1);

    Registros[17] = 0; // iniciamos CC
    if (resultado == 0)
        Registros[17] = Registros[17] | (1 << 30); // prende Z
    if (resultado < 0)
        Registros[17] = Registros[17] | (1 << 31); // prende N

    GuardarDestino(Registros[2], resultado, MemoriaPrincipal, Registros, TablaSegmentos);
}
void op_SWAP(unsigned char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[])
{
    int valorA = ObtenerValorOperando(Registros[2], MemoriaPrincipal, Registros, TablaSegmentos);
    int valorB = ObtenerValorOperando(Registros[3], MemoriaPrincipal, Registros, TablaSegmentos);

    if (valorB == 0)
        Registros[17] = Registros[17] | (1 << 30); // prende Z
    if (valorA < 0)
        Registros[17] = Registros[17] | (1 << 31); // prende N

    GuardarDestino(Registros[2], valorB, MemoriaPrincipal, Registros, TablaSegmentos);
    GuardarDestino(Registros[3], valorA, MemoriaPrincipal, Registros, TablaSegmentos);
}

void op_PUSH(unsigned char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[])
{
    int segSS = (Registros[7] >> 16) & 0xFFFF;
    int offsetSP = (Registros[7] & 0xFFFF) - 4;

    // Chequeo de STACK OVERFLOW: offset menor a 0
    if (offsetSP < 0)
    {
        printf("\n[ERROR] STACK OVERFLOW\n");
        exit(1);
    }

    Registros[7] = (segSS << 16) | (offsetSP & 0xFFFF);

    int valor = ObtenerValorOperando(Registros[2], MemoriaPrincipal, Registros, TablaSegmentos);
    int dirFisica = TablaSegmentos[segSS].base + offsetSP;

    MemoriaPrincipal[dirFisica + 0] = (valor >> 24) & 0xFF;
    MemoriaPrincipal[dirFisica + 1] = (valor >> 16) & 0xFF;
    MemoriaPrincipal[dirFisica + 2] = (valor >> 8) & 0xFF;
    MemoriaPrincipal[dirFisica + 3] = valor & 0xFF;
}

void op_POP(unsigned char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[])
{
    int segSS = (Registros[7] >> 16) & 0xFFFF;
    int offsetSP = Registros[7] & 0xFFFF;
    int tamSS = TablaSegmentos[segSS].size;

    // Chequeo de STACK UNDERFLOW: pila vacia
    if (offsetSP + 4 > tamSS)
    {
        printf("\n[ERROR] STACK UNDERFLOW\n");
        exit(1);
    }

    int dirFisica = TablaSegmentos[segSS].base + offsetSP;
    int valorExtraido = ((unsigned char)MemoriaPrincipal[dirFisica + 0] << 24) |
                        ((unsigned char)MemoriaPrincipal[dirFisica + 1] << 16) |
                        ((unsigned char)MemoriaPrincipal[dirFisica + 2] << 8) |
                        ((unsigned char)MemoriaPrincipal[dirFisica + 3]);

    GuardarDestino(Registros[2], valorExtraido, MemoriaPrincipal, Registros, TablaSegmentos);

    Registros[7] = (segSS << 16) | ((offsetSP + 4) & 0xFFFF);
}

void op_CALL(unsigned char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[])
{
    // CALL equivale a PUSH IP y luego JMP destino
    int segSS = (Registros[7] >> 16) & 0xFFFF;
    int offsetSP = (Registros[7] & 0xFFFF) - 4;

    // Chequeo de STACK OVERFLOW: offset dentro del segmento menor a 0
    if (offsetSP < 0)
    {
        printf("\n[ERROR] STACK OVERFLOW\n");
        exit(1);
    }

    // Actualizar SP con el nuevo offset
    Registros[7] = (segSS << 16) | (offsetSP & 0xFFFF);

    // Guardar la dirección de retorno (el IP ya apunta a la siguiente instrucción)
    int ipRetorno = Registros[0];
    int dirFisica = TablaSegmentos[segSS].base + offsetSP;

    MemoriaPrincipal[dirFisica + 0] = (ipRetorno >> 24) & 0xFF;
    MemoriaPrincipal[dirFisica + 1] = (ipRetorno >> 16) & 0xFF;
    MemoriaPrincipal[dirFisica + 2] = (ipRetorno >> 8) & 0xFF;
    MemoriaPrincipal[dirFisica + 3] = ipRetorno & 0xFF;

    // Salto al destino indicado por OP1
    int destino = ObtenerValorOperando(Registros[2], MemoriaPrincipal, Registros, TablaSegmentos);
    SaltarA(Registros, destino);
}

void op_RET(unsigned char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[])
{
    // RET equivale a POP IP
    int segSS = (Registros[7] >> 16) & 0xFFFF;
    int offsetSP = Registros[7] & 0xFFFF;
    int tamSS = TablaSegmentos[segSS].size;

    // Chequeo de STACK UNDERFLOW: intentar desapilar más allá del límite del segmento
    if (offsetSP + 4 > tamSS)
    {
        printf("\n[ERROR] STACK UNDERFLOW\n");
        exit(1);
    }

    int dirFisica = TablaSegmentos[segSS].base + offsetSP;
    int ipExtraido = ((unsigned char)MemoriaPrincipal[dirFisica + 0] << 24) |
                     ((unsigned char)MemoriaPrincipal[dirFisica + 1] << 16) |
                     ((unsigned char)MemoriaPrincipal[dirFisica + 2] << 8) |
                     ((unsigned char)MemoriaPrincipal[dirFisica + 3]);

    // Avanzar SP y restaurar IP
    Registros[7] = (segSS << 16) | ((offsetSP + 4) & 0xFFFF);
    Registros[0] = ipExtraido;
}