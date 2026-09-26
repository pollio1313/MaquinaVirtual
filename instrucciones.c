#include <stdlib.h>
#include "instrucciones.h"
#include <limits.h>

// tres funciones auxiliares

int CalcularDireccionFisica(int operandoMemoria, char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[])
{
    short offset = (short)((operandoMemoria >> 8) & 0xFFFF); // Offset de 16 bits podria ser short o int16_t
    int codReg = operandoMemoria & 0x1F;                     // codigo registro 5 bits
    int numSeg = (Registros[codReg] >> 16) & 0xFFFF;
    int offsetFinal = (Registros[codReg] & 0xFFFF) + offset;

    // validar segmento
    if (numSeg < 0 || numSeg >= 8 || TablaSegmentos[numSeg].size == 0xFFFF || (short)TablaSegmentos[numSeg].base == -1)
    {
        printf("\n [ERROR] Segmento invalido %d\n", numSeg);
        exit(1);
    }
    // validar limite que no se pase o se caiga
    if (offsetFinal < 0 || (offsetFinal + 4) > TablaSegmentos[numSeg].size)
    {
        printf("\n[ERROR] Fuera del limite de segmento.\n");
        exit(1);
    }

    Registros[4] = (numSeg << 16) | (offsetFinal & 0xFFFF); // LAR: direccion logica
    int dirFisica = TablaSegmentos[numSeg].base + offsetFinal;
    Registros[5] = (4 << 16) | (dirFisica & 0xFFFF); // MAR dir fisica
    return dirFisica;
}

int ObtenerValorOperando(int operando, char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[]) // extrae el valor real del op
{
    int tipo = (operando >> 24) & 0xFF; // tipo 1,2 y 3
    int valor = operando & 0x00FFFFFF;  // valor crudo

    if (tipo == 1)                      // registro
        return Registros[valor & 0x1F]; // lo que contiene el registro de indice valor aca tambien me habia olvidado que eran 5bits nada mas
    else if (tipo == 2)                 // inmediato
        return (short)(valor & 0xFFFF); // 16bits
    else if (tipo == 3)                 // en memoria
    {
        int dirFisica = CalcularDireccionFisica(valor, MemoriaPrincipal, Registros, TablaSegmentos); // Ya guarda en el MAR
        // reconstruimos directamente en el MBR
        Registros[6] = (MemoriaPrincipal[dirFisica] << 24) |
                       (MemoriaPrincipal[dirFisica + 1] << 16) |
                       (MemoriaPrincipal[dirFisica + 2] << 8) |
                       MemoriaPrincipal[dirFisica + 3];
        return Registros[6]; // devolvemos MBR
    }
    return 0; // error o defecto, habria que ver despues como tratamos con los errores
              // porque seguro los codigos que nos van a dar a probar van a tener errores aproposito
}
// guarda el resultado en el destino (OP1)
void GuardarDestino(int operandoDestino, int resultado, char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[])
{
    int tipo = (operandoDestino >> 24) & 0xFF; // tipo 1 o 3
    int valor = operandoDestino & 0x00FFFFFF;  // la ubicacion fisica

    if (tipo == 1)                           // registro
        Registros[valor & 0x1F] = resultado; // pisamos el resultado en el indice valor, 5 bits
    else if (tipo == 3)                      // memoria
    {
        int dirFisica = CalcularDireccionFisica(valor, MemoriaPrincipal, Registros, TablaSegmentos);
        Registros[6] = resultado; // MBR
        MemoriaPrincipal[dirFisica] = (resultado >> 24) & 0xFF;
        MemoriaPrincipal[dirFisica + 1] = (resultado >> 16) & 0xFF;
        MemoriaPrincipal[dirFisica + 2] = (resultado >> 8) & 0xFF;
        MemoriaPrincipal[dirFisica + 3] = resultado & 0xFF;
    }
    else
    {
        printf("Error: Operando de destino invalido\n");
    }
}

void ImprimirBinario(int valor, int tamByte)
{
    print("0b");
    for (int i = (tamByte * 8) - 1; i >= 0; i--)
        printf("%d", (valor >> i) & 1);
}

// todas las operaciones desarrolladas
void op_SYS(char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[])
{
    int subcodigo = ObtenerValorOperando(Registros[2], MemoriaPrincipal, Registros, TablaSegmentos);
    int modo = Registros[10];                      // EAX formato
    int cantCeldas = Registros[12] & 0xFFFF;       // ecx bajo cantidad
    int tamBytes = (Registros[12] >> 16) & 0xFFFF; // ecx alto tamaño bytes
    int dirLogica = Registros[13];                 // edx puntero de inicio

    if (tamBytes <= 0)
        tamBytes = 4; // por defecto 4 bytes

    for (int i = 0; i < cantCeldas; i++)
    {
        int numSeg = (dirLogica >> 16) & 0xFFFF;
        int offset = dirLogica & 0xFFFF;

        // validar segmento
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
        Registros[4] = dirLogica;                               // LAR
        Registros[5] = (tamBytes << 16) | (dirFisica & 0xFFFF); // MAR

        printf("[%04X]: ", dirFisica); //[XXXX]
        if (subcodigo == 1)            // read
        {
            int dato = 0;
            if (modo & 0x02) // caracter
            {
                char c;
                scanf(" %c", &c);
                dato = c;
            }
            else if (modo & 0x08) // hexa
                scanf("%x", &dato);
            else if (modo & 0x04) // octal
                scanf("%o", &dato);
            else if (modo & 0x10) // bin
            {
                char binStr[35]; // 0b + 32 bits + \0
                scanf("%s", binStr);
                int inicio = 0;
                if (binStr[0] == '0' && (binStr[1] == 'b' || binStr[1] == 'B'))
                {
                    inicio = 2;
                }
                for (int j = inicio; binStr[j] != '\0'; j++)
                    if (binStr[j] == '0' || binStr[j] == '1')
                        dato = (dato << 1) | (binStr[j] - '0'); // convierto en ascii el numero
            }
            else // decima por defecto (0x01)
                scanf("%d", &dato);

            Registros[6] = dato; // MBR
            for (int j = 0; j < tamBytes; j++)
                MemoriaPrincipal[dirFisica + j] = (dato >> (8 * (tamBytes - 1 - j))) & 0xFF;
        }
        else if (subcodigo == 2) // write
        {
            int valor = 0;
            for (int j = 0; j < tamBytes; j++)
                valor = (valor << 8) | MemoriaPrincipal[dirFisica + j];

            Registros[6] = valor; // MBR
            if (modo & 0x10)      // bin
                ImprimirBinario(valor, tamBytes);
            else if (modo & 0x08) // hexa
                printf("0x%X ", valor);
            else if (modo & 0x04) // octal
                printf("0o%o ", valor);
            else if (modo & 0x02) // caract
            {
                for (int j = 0; j < tamBytes; j++)
                {
                    char c = (valor >> (8 * (tamBytes - 1 - j))) & 0xFF;
                    print("%c", c);
                }
                printf(" ");
            }
            else
                printf("%d", valor);
            printf("\n");
        }
        dirLogica += tamBytes; // siguiente celda
    }
}
void op_STOP(char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[])
{
    Registros[0] = -1;
}
void op_MOV(char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[])
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
// creo aca las funciones pero vacias, para q compile(pollio)
void op_ADD(char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[])
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
    if ((unsigned int)valorA + (unsigned int)valorB < (unsigned int)valorA)                          // aca pedi ayuda a mi amigo gemini
        Registros[17] = Registros[17] | (1 << 29);                                                   // Prende C
    GuardarDestino(Registros[2], resultado, MemoriaPrincipal, Registros, TablaSegmentos);            // guardamos en op1
}

void op_SUB(char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[])
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

void op_MUL(char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[])
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

void op_DIV(char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[])
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

void op_AND(char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[])
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

void op_OR(char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[])
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
void op_XOR(char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[])
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

void op_SHL(char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[])
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
void op_SHR(char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[])
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
void op_SAR(char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[])
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

void op_CMP(char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[])
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

void op_JC(char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[])
{
    int valorA = ObtenerValorOperando(Registros[2], MemoriaPrincipal, Registros, TablaSegmentos);
    if ((Registros[17] >> 29) & 1) // esta prendido C?
        Registros[0] = valorA;     // IP a valorA
}
void op_JMP(char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[])
{
    Registros[0] = ObtenerValorOperando(Registros[2], MemoriaPrincipal, Registros, TablaSegmentos);
}
void op_JN(char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[])
{
    int valorA = ObtenerValorOperando(Registros[2], MemoriaPrincipal, Registros, TablaSegmentos);
    if ((Registros[17] >> 31) & 1) // esta prendido N?
        Registros[0] = valorA;
}
void op_JNP(char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[])
{
    int valorA = ObtenerValorOperando(Registros[2], MemoriaPrincipal, Registros, TablaSegmentos);
    if (((Registros[17] >> 31) & 1) || ((Registros[17] >> 30) & 1)) // esta prendido N y Z?
        Registros[0] = valorA;
}
void op_JNN(char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[])
{
    int valorA = ObtenerValorOperando(Registros[2], MemoriaPrincipal, Registros, TablaSegmentos);
    if (!((Registros[17] >> 31) & 1))
        Registros[0] = valorA;
}
void op_JNZ(char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[])
{
    int valorA = ObtenerValorOperando(Registros[2], MemoriaPrincipal, Registros, TablaSegmentos);
    if (!((Registros[17] >> 30) & 1))
        Registros[0] = valorA;
}
void op_JP(char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[])
{
    int valorA = ObtenerValorOperando(Registros[2], MemoriaPrincipal, Registros, TablaSegmentos);
    if (!((Registros[17] >> 31) & 1) && !((Registros[17] >> 30) & 1))
        Registros[0] = valorA;
}
void op_JV(char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[])
{
    int valorA = ObtenerValorOperando(Registros[2], MemoriaPrincipal, Registros, TablaSegmentos);
    if ((Registros[17] >> 28) & 1)
        Registros[0] = valorA;
}
void op_JZ(char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[])
{
    int valorA = ObtenerValorOperando(Registros[2], MemoriaPrincipal, Registros, TablaSegmentos);
    if ((Registros[17] >> 30) & 1)
        Registros[0] = valorA;
}

void op_LDH(char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[])
{
    int valorA = ObtenerValorOperando(Registros[2], MemoriaPrincipal, Registros, TablaSegmentos);
    int destino = Registros[2];

    Registros[destino] = (Registros[destino] & 0xFFFF) | ((valorA & 0XFFFF) << 16);
}
void op_LDL(char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[])
{
    int valorA = ObtenerValorOperando(Registros[2], MemoriaPrincipal, Registros, TablaSegmentos);
    int registroDestino = Registros[2];

    Registros[registroDestino] = (Registros[registroDestino] & 0xFFFF0000) | (valorA & 0xFFFF);
}
void op_NOT(char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[])
{
    int valorA = ObtenerValorOperando(Registros[2], MemoriaPrincipal, Registros, TablaSegmentos);
    int resultado = ~valorA; // inverso bit a bit

    Registros[17] = 0; // iniciamos CC
    if (resultado == 0)
        Registros[17] = Registros[17] | (1 << 30); // prende Z
    if (resultado < 0)
        Registros[17] = Registros[17] | (1 << 31); // prende N

    GuardarDestino(Registros[2], resultado, MemoriaPrincipal, Registros, TablaSegmentos);
}
void op_RND(char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[])
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
void op_SWAP(char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[])
{
    int valorA = ObtenerValorOperando(Registros[2], MemoriaPrincipal, Registros, TablaSegmentos);
    int valorB = ObtenerValorOperando(Registros[3], MemoriaPrincipal, Registros, TablaSegmentos);

    GuardarDestino(Registros[2], valorB, MemoriaPrincipal, Registros, TablaSegmentos);
    GuardarDestino(Registros[3], valorA, MemoriaPrincipal, Registros, TablaSegmentos);
}
