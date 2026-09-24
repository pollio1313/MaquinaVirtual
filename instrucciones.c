#include <instrucciones.h>
#include <limits.h>

// dos funciones auxiliares
int ObtenerValorOperando(int operando, char MemoriaPrincipal[], int Registros[]) // extrae el valor real del op
{
    int tipo = (operando >> 24) & 0xFF; // tipo 1,2 y 3
    int valor = operando & 0x00FFFFFF;  // valor crudo

    if (tipo == 1)               // registro
        return Registros[valor]; // lo que contiene el registro de indice valor
    else if (tipo == 2)          // inmediato
        return valor;
    else if (tipo == 3) // en memoria
    {
        Registros[5] = valor; // direccion en el MAR
        // reconstruimos directamente en el MBR
        Registros[6] = (MemoriaPrincipal[valor] << 24) |
                       (MemoriaPrincipal[valor + 1] << 16) |
                       (MemoriaPrincipal[valor + 2] << 8) |
                       MemoriaPrincipal[valor + 3];
        return Registros[6]; // devolvemos MBR
    }
    return 0; // error o defecto, habria que ver despues como tratamos con los errores
              // porque seguro los codigos que nos van a dar a probar van a tener errores aproposito
}
// guarda el resultado en el destino (OP1)
void GuardarDestino(int operandoDestino, int resultado, char MemoriaPrincipal[], int Registros[])
{
    int tipo = (operandoDestino >> 24) & 0xFF; // tipo 1 o 3
    int valor = operandoDestino & 0x00FFFFFF;  // la ubicacion fisica

    if (tipo == 1)                    // registro
        Registros[valor] = resultado; // pisamos el resultado en el indice valor
    else if (tipo == 3)               // memoria
    {
        Registros[5] = valor;     // la direccion de destino en el MAR
        Registros[6] = resultado; // el dato a escribir en el MBR
        MemoriaPrincipal[valor] = (resultado >> 24) & 0xFF;
        MemoriaPrincipal[valor + 1] = (resultado >> 16) & 0xFF;
        MemoriaPrincipal[valor + 2] = (resultado >> 8) & 0xFF;
        MemoriaPrincipal[valor + 3] = resultado & 0xFF;
    }
}

// todas las operaciones desarrolladas
void op_STOP(char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[])
{
    Registros[0] = -1;
}
void op_MOV(char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[])
{
    int valorOrigen = ObtenerValorOperando(Registros[3], MemoriaPrincipal, Registros);
    Registros[17] = 0;
    GuardarDestino(Registros[2], valorOrigen, MemoriaPrincipal, Registros);
}
// despues en add y otras funciones volvemos a usar las auxiliares,
// creo aca las funciones pero vacias, para q compile(pollio)
void op_ADD(char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[])
{
    int valorA = ObtenerValorOperando(Registros[2], MemoriaPrincipal, Registros);
    int valorB = ObtenerValorOperando(Registros[3], MemoriaPrincipal, Registros);
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
    GuardarDestino(Registros[2], resultado, MemoriaPrincipal, Registros);                            // guardamos en op1
}

void op_SUB(char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[])
{
    int valorA = ObtenerValorOperando(Registros[2], MemoriaPrincipal, Registros);
    int valorB = ObtenerValorOperando(Registros[3], MemoriaPrincipal, Registros);
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
    GuardarDestino(Registros[2], resultado, MemoriaPrincipal, Registros);
}

void op_MUL(char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[])
{
    int valorA = ObtenerValorOperando(Registros[2], MemoriaPrincipal, Registros);
    int valorB = ObtenerValorOperando(Registros[3], MemoriaPrincipal, Registros);
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
    GuardarDestino(Registros[2], resultado, MemoriaPrincipal, Registros);
}

void op_DIV(char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[])
{
    int valorA = ObtenerValorOperando(Registros[2], MemoriaPrincipal, Registros);
    int valorB = ObtenerValorOperando(Registros[3], MemoriaPrincipal, Registros);
    Registros[17] = 0; // iniciamos CC
    if (valorB != 0)
    {
        int resultado = valorA / valorB;
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
        GuardarDestino(Registros[2], resultado, MemoriaPrincipal, Registros);
    }
}

void op_AND(char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[])
{
    int valorA = ObtenerValorOperando(Registros[2], MemoriaPrincipal, Registros);
    int valorB = ObtenerValorOperando(Registros[3], MemoriaPrincipal, Registros);
    int resultado = valorA & valorB;
    Registros[17] = 0; // iniciamos CC
    // ayudamemoria xd
    //  bit 32 N bit 31 Z bit 30 C (acarreo) bit 29 V (desbordamiento) demas 28 bits reservados
    if (resultado == 0)
        Registros[17] = Registros[17] | (1 << 30); // prende Z
    if (resultado < 0)
        Registros[17] = Registros[17] | (1 << 31); // prende N

    GuardarDestino(Registros[2], resultado, MemoriaPrincipal, Registros);
}

void op_OR(char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[])
{
    int valorA = ObtenerValorOperando(Registros[2], MemoriaPrincipal, Registros);
    int valorB = ObtenerValorOperando(Registros[3], MemoriaPrincipal, Registros);
    int resultado = valorA | valorB;
    Registros[17] = 0; // iniciamos CC
    // ayudamemoria xd
    //  bit 32 N bit 31 Z bit 30 C (acarreo) bit 29 V (desbordamiento) demas 28 bits reservados
    if (resultado == 0)
        Registros[17] = Registros[17] | (1 << 30); // prende Z
    if (resultado < 0)
        Registros[17] = Registros[17] | (1 << 31); // prende N

    GuardarDestino(Registros[2], resultado, MemoriaPrincipal, Registros);
}
void op_XOR(char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[])
{
    int valorA = ObtenerValorOperando(Registros[2], MemoriaPrincipal, Registros);
    int valorB = ObtenerValorOperando(Registros[3], MemoriaPrincipal, Registros);
    int resultado = valorA ^ valorB;
    Registros[17] = 0; // iniciamos CC
    // ayudamemoria xd
    //  bit 32 N bit 31 Z bit 30 C (acarreo) bit 29 V (desbordamiento) demas 28 bits reservados
    if (resultado == 0)
        Registros[17] = Registros[17] | (1 << 30); // prende Z
    if (resultado < 0)
        Registros[17] = Registros[17] | (1 << 31); // prende N

    GuardarDestino(Registros[2], resultado, MemoriaPrincipal, Registros);
}

void op_SHL(char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[])
{
    int valorA = ObtenerValorOperando(Registros[2], MemoriaPrincipal, Registros);
    int valorB = ObtenerValorOperando(Registros[3], MemoriaPrincipal, Registros);
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

        GuardarDestino(Registros[2], resultado, MemoriaPrincipal, Registros);
    }
}
void op_SHR(char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[])
{
    int valorA = ObtenerValorOperando(Registros[2], MemoriaPrincipal, Registros);
    int valorB = ObtenerValorOperando(Registros[3], MemoriaPrincipal, Registros);
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

        GuardarDestino(Registros[2], resultado, MemoriaPrincipal, Registros);
    }
}
void op_SAR(char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[])
{
    int valorA = ObtenerValorOperando(Registros[2], MemoriaPrincipal, Registros);
    int valorB = ObtenerValorOperando(Registros[3], MemoriaPrincipal, Registros);
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
        GuardarDestino(Registros[2], resultado, MemoriaPrincipal, Registros);
    }
}

void op_CMP(char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[])
{
    int valorA = ObtenerValorOperando(Registros[2], MemoriaPrincipal, Registros);
    int valorB = ObtenerValorOperando(Registros[3], MemoriaPrincipal, Registros);
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
    int valorA = ObtenerValorOperando(Registros[2], MemoriaPrincipal, Registros);
    if ((Registros[17] >> 29) & 1) // esta prendido C?
        Registros[0] = valorA;     // IP a valorA
}
void op_JMP(char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[])
{
    Registros[0] = ObtenerValorOperando(Registros[2], MemoriaPrincipal, Registros);
}
void op_JN(char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[])
{
    int valorA = ObtenerValorOperando(Registros[2], MemoriaPrincipal, Registros);
    if ((Registros[17] >> 31) & 1) // esta prendido N?
        Registros[0] = valorA;
}
void op_JNP(char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[])
{
    int valorA = ObtenerValorOperando(Registros[2], MemoriaPrincipal, Registros);
    if (((Registros[17] >> 31) & 1) || ((Registros[17] >> 30) & 1)) // esta prendido N y Z?
        Registros[0] = valorA;
}
void op_JNN(char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[])
{
    int valorA = ObtenerValorOperando(Registros[2], MemoriaPrincipal, Registros);
    if (!((Registros[17] >> 31) & 1))
        Registros[0] = valorA;
}
void op_JNZ(char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[])
{
    int valorA = ObtenerValorOperando(Registros[2], MemoriaPrincipal, Registros);
    if (!((Registros[17] >> 30) & 1))
        Registros[0] = valorA;
}
void op_JP(char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[])
{
    int valorA = ObtenerValorOperando(Registros[2], MemoriaPrincipal, Registros);
    if (!((Registros[17] >> 31) & 1) && !((Registros[17] >> 30) & 1))
        Registros[0] = valorA;
}
void op_JV(char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[])
{
    int valorA = ObtenerValorOperando(Registros[2], MemoriaPrincipal, Registros);
    if ((Registros[17] >> 28) & 1)
        Registros[0] = valorA;
}
void op_JZ(char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[])
{
    int valorA = ObtenerValorOperando(Registros[2], MemoriaPrincipal, Registros);
    if ((Registros[17] >> 30) & 1)
        Registros[0] = valorA;
}

void op_LDH(char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[])
{
    int valorA = ObtenerValorOperando(Registros[2], MemoriaPrincipal, Registros);
    int destino = Registros[2];

    Registros[destino] = (Registros[destino] & 0xFFFF) | ((valorA & 0XFFFF) << 16);
}
void op_LDL(char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[])
{
    int valorA = ObtenerValorOperando(Registros[2], MemoriaPrincipal, Registros);
    int registroDestino = Registros[2];

    Registros[registroDestino] = (Registros[registroDestino] & 0xFFFF0000) | (valorA & 0xFFFF);
}
void op_NOT(char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[])
{
    int valorA = ObtenerValorOperando(Registros[2], MemoriaPrincipal, Registros);
    int resultado = ~valorA; // inverso bit a bit

    Registros[17] = 0; // iniciamos CC
    if (resultado == 0)
        Registros[17] = Registros[17] | (1 << 30); // prende Z
    if (resultado < 0)
        Registros[17] = Registros[17] | (1 << 31); // prende N

    GuardarDestino(Registros[2], resultado, MemoriaPrincipal, Registros);
}
void op_RND(char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[])
{
    int resultado = rand(); // numero aleatorio

    Registros[17] = 0; // iniciamos CC
    if (resultado == 0)
        Registros[17] = Registros[17] | (1 << 30); // prende Z
    if (resultado < 0)
        Registros[17] = Registros[17] | (1 << 31); // prende N

    GuardarDestino(Registros[2], resultado, MemoriaPrincipal, Registros);
}
void op_SWAP(char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[])
{
    int valorA = ObtenerValorOperando(Registros[2], MemoriaPrincipal, Registros);
    int valorB = ObtenerValorOperando(Registros[3], MemoriaPrincipal, Registros);

    GuardarDestino(Registros[2], valorB, MemoriaPrincipal, Registros);
    GuardarDestino(Registros[3], valorA, MemoriaPrincipal, Registros);
}
