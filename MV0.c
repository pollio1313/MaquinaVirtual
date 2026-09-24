#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
typedef struct
{ // para organizar un segmento necesitamos anotar dos datos: donde empieza y cuanto mide
    int base;
    int size;
} Segmento;

// esta funcion necesita que le llegue
void MOV(char MemoriaPrincipal[], char Registros[][4], char OPA, char OPB, char tipoOPB)
{
    char plato;

    if (tipoOPB == 1)
    {
        Registros[OPB][3] = OPA; // habria q revisar cuantos bytes ocupa el OPA
    }
    else if (tipoOPB == 3)
    {
        MemoriaPrincipal[OPB] = OPA;
    }
}

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

typedef void (*InstruccionFunc)( char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[]); // puntero a funcion
struct Instruccion
{
    char *nombre; // esta sirve por ahrao, pero en realidad tendria que ser punteros a funciones con cada metodo
    char codigo;
    int cantOP;
    InstruccionFunc ejecutar;
};

// prototipos de las funciones de ejecucion
void op_STOP( char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[]);
void op_MOV( char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[]);
void op_ADD( char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[]);
// faltan todas las otras xd, las voy a poenr asi compila el programa, pero sean libres de borrarlas(pollio)
void op_JMP( char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[]);
void op_JP( char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[]);
void op_JN( char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[]);
void op_JZ( char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[]);
void op_JC( char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[]);
void op_JV( char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[]);
void op_JNP( char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[]);
void op_JNN( char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[]);
void op_JNZ( char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[]);
void op_NOT( char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[]);
void op_SUB( char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[]);
void op_MUL( char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[]);
void op_DIV( char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[]);
void op_CMP( char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[]);
void op_AND( char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[]);
void op_OR( char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[]);
void op_XOR( char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[]);
void op_SWAP( char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[]);
void op_SHL( char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[]);
void op_SHR( char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[]);
void op_SAR( char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[]);
void op_LDL( char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[]);
void op_LDH( char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[]);
void op_RND( char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[]);


const struct Instruccion tablaInstrucciones[] = {
    {"SYS", 0x00, 1, NULL},
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

void Lectura( char MemoriaPrincipal[16384], const char *filename, char *cabecera, uint16_t *LargoCod)
{
    FILE *archivoVMX;
    int i = 0, j = 0, valor;
    archivoVMX = fopen(filename, "rb"); // abro el archivo vmx //aclarar que es binario
    if (archivoVMX == NULL)
    {
        printf("NO SE ABRIO CAPO");
    }
    else
    {
        fread(cabecera, sizeof(char), 6, archivoVMX); // leo la parte q dice version etc
        char largo[2];
        fread(largo, sizeof(char), 2, archivoVMX);

        *LargoCod = (largo[0] << 8) | largo[1]; // habia un problema con la lectura y ahroa lee los dos bytes y aca los une
        printf("  Largo:%d",LargoCod);
        if ((strncmp(cabecera, "VMX26", 5) == 0) && (cabecera[5] == 1))
        {                                                                     // solo corroboro al version
            fread(&MemoriaPrincipal[0], sizeof(char), *LargoCod, archivoVMX); // cargo en memoria solo el codigo en adelante
            
        }
        else
        {
            printf("no se acepta esa version %s", cabecera);
        }
        fclose(archivoVMX);
    }
}

void MostrarBinario(char byte)
{ // esto es solo para hacer pruebas, esta prompeado
    for (int i = 7; i >= 0; i--)
    {
        printf("%d", (byte >> i) & 1);
    }
}

void MostrarCodigo( char MemoriaPrincipal[16384], char *cabecera, uint16_t LargoCod) // ya que tenemos LargoCod no es necesario volver a calcularlo
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

        printf("\n %s   ", tablaInstrucciones[codOperacion].nombre);
        if (cant == 01)
        {
            OperandoA = operacion >> 6 & 0x03;
            for (int q = 0; q < OperandoA; q++)
            { // while apra consumir los valores de operandos, si es 0 sigeun de largo ,uso operandoB y no cantBytes porq valen lo mismo
                j++;
                valorOPA[q] = MemoriaPrincipal[j];
                printf("%02X", valorOPA[q]);
            }
        }
        else if (cant == 02)
        { // se fija cuantos bytes chupa cada operando
            OperandoB = (operacion >> 6) & 0x03;
            for (int q = 0; q < OperandoB; q++)
            {
                j++; // parece qprimero viene el byte mas signfiquitaivo
                valorOPB[q] = MemoriaPrincipal[j];
            }
            OperandoA = (operacion >> 4) & 0x03;
            for (int q = 0; q < OperandoA; q++)
            {
                j++;
                valorOPA[q] = MemoriaPrincipal[j];
            }
            if (OperandoA == 01)
            {
                printf("%s", tablaRegistros[valorOPA[0]]);
            }
            else
            {
                for (int q = 0; q < OperandoA; q++)
                {
                    printf("%02X", valorOPA[q]);
                }
            }
            printf(",");
            if (OperandoB == 01)
            {
                printf("%s", tablaRegistros[valorOPB[0]]);
            }
            else
            {
                for (int q = 0; q < OperandoB; q++)
                {
                    printf("%02X", valorOPB[q]);
                }
            }
        }
        else if (cant == 00)
        { // esta linea esta de mas, porq ya deberian valer 0 de antes
            OperandoA = OperandoB = 0;
        }
        j++;

        printf("\nj:%d", j);
    }
}
// dos funciones auxiliares
int ObtenerValorOperando(int operando,char MemoriaPrincipal[], int Registros[]) // extrae el valor real del op
{
    int tipo = (operando >> 24) & 0xFF; // tipo 1,2 y 3
    int valor = operando & 0x00FFFFFF;  // valor crudo

    if (tipo == 1)               // registro
        return Registros[valor]; // lo que contiene el registro de indice valor
    else if (tipo == 2)          // inmediato
        return valor;
    else if (tipo == 3) // en memoria
        return (MemoriaPrincipal[valor] << 24) |
               (MemoriaPrincipal[valor + 1] << 16) |
               (MemoriaPrincipal[valor + 2] << 8) |
               MemoriaPrincipal[valor + 3];
    return 0; // error o defecto, habria que ver despues como tratamos con los errores
              // porque seguro los codigos que nos van a dar a probar van a tener errores aproposito
}
// guarda el resultado en el destino (OP1)
void GuardarDestino(int operandoDestino, int resultado,char MemoriaPrincipal[], int Registros[])
{
    int tipo = (operandoDestino >> 24) & 0xFF; // tipo 1 o 3
    int valor = operandoDestino & 0x00FFFFFF;  // la ubicacion fisica

    if (tipo == 1)                    // registro
        Registros[valor] = resultado; // pisamos el resultado en el indice valor
    else if (tipo == 3)               // memoria
    {
        MemoriaPrincipal[valor] = (resultado >> 24) & 0xFF;
        MemoriaPrincipal[valor + 1] = (resultado >> 16) & 0xFF;
        MemoriaPrincipal[valor + 2] = (resultado >> 8) & 0xFF;
        MemoriaPrincipal[valor + 3] = resultado & 0xFF;
    }
}
void AsignarSegmentos(int LargoCod, Segmento TablaSegmentos[], int Registros[])
{

    TablaSegmentos[0].base = 0;
    TablaSegmentos[0].size = LargoCod;

    TablaSegmentos[1].base = LargoCod;
    TablaSegmentos[1].size = 16384 - LargoCod;

    Registros[26] = 0; // CS apuntan a la posicion 0
    Registros[27] = 1; // DS apunta a la posicion 1
}
// todas las operaciones desarrolladas
void op_STOP( char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[])
{
    Registros[0] = -1;
}
void op_MOV( char MemoriaPrincipal[], int Registros[], Segmento TablaSegmentos[])
{
    int valorOrigen = ObtenerValorOperando(Registros[3], MemoriaPrincipal, Registros);
    GuardarDestino(Registros[2], valorOrigen, MemoriaPrincipal, Registros);
}
// despues en add y otras funciones volvemos a usar las auxiliares


void EjecutarMaquina(char MemoriaPrincipal[], Segmento TablaSegmentos[], int Registros[])
{
    char codOperacion, operandoA, operandoB;
    int pos, cantOP;
    Registros[0] = Registros[26]; // IP se inicia aputando a la primera instruccion del codigo, mismo valor que CS

    while (Registros[0] >= 0 && Registros[0] < TablaSegmentos[0].size)
    { // Se itera hasta que IP apunte fuera del limite del segmento de codigo
        pos = Registros[0];
        codOperacion = Registros[1] = MemoriaPrincipal[pos] & 0x1F; // decodifico y guardo el codigo limpio en OPC
        cantOP = tablaInstrucciones[codOperacion].cantOP;
        if (cantOP == 0)
        {
            Registros[2] = Registros[3] = 0;
        }
        if (cantOP == 1)
        {
            operandoA = (MemoriaPrincipal[pos] >> 6) & 0x03;
            Registros[2] = operandoA << 24; // guardamos OP A en OP1 son de 32 bits

            if (operandoA == 1)
            {
                Registros[2] = Registros[2] | MemoriaPrincipal[++pos];
            }
            else if (operandoA == 2)
            {
                Registros[2] = Registros[2] | (MemoriaPrincipal[++pos] << 8);
                Registros[2] = Registros[2] | MemoriaPrincipal[++pos];
            }
            else
            {
                Registros[2] = Registros[2] | (MemoriaPrincipal[++pos] << 16);
                Registros[2] = Registros[2] | (MemoriaPrincipal[++pos] << 8);
                Registros[2] = Registros[2] | MemoriaPrincipal[++pos];
            }
        }

        if (cantOP == 2)
        {
            operandoB = (MemoriaPrincipal[pos] >> 6) & 0x03;
            operandoA = (MemoriaPrincipal[pos] >> 4) & 0x03;
            Registros[2] = operandoA << 24; // A en OP1
            Registros[3] = operandoB << 24; // B en OP2

            if (operandoB == 1)
            {
                Registros[3] = Registros[3] | MemoriaPrincipal[++pos];
            }
            else if (operandoB == 2)
            {
                Registros[3] = Registros[3] | (MemoriaPrincipal[++pos] << 8);
                Registros[3] = Registros[3] | MemoriaPrincipal[++pos];
            }
            else
            {
                Registros[3] = Registros[3] | (MemoriaPrincipal[++pos] << 16);
                Registros[3] = Registros[3] | (MemoriaPrincipal[++pos] << 8);
                Registros[3] = Registros[3] | MemoriaPrincipal[++pos];
            }
            if (operandoA == 1)
            {
                Registros[2] = Registros[2] | MemoriaPrincipal[++pos];
            }
            else if (operandoA == 2)
            {
                Registros[2] = Registros[2] | (MemoriaPrincipal[++pos] << 8);
                Registros[2] = Registros[2] | MemoriaPrincipal[++pos];
            }
            else
            {
                Registros[2] = Registros[2] | (MemoriaPrincipal[++pos] << 16);
                Registros[2] = Registros[2] | (MemoriaPrincipal[++pos] << 8);
                Registros[2] = Registros[2] | MemoriaPrincipal[++pos];
            }
        }

        // ejecucion
        if (tablaInstrucciones[codOperacion].ejecutar != NULL)
        {
            tablaInstrucciones[codOperacion].ejecutar(MemoriaPrincipal, Registros, TablaSegmentos);
        }
        // avanzamos el ip si no fue STOP
        if (codOperacion != 0x0F)
            Registros[0] = pos + 1;
    }
}

int main(int argc, char *argv[])
{

    int Registros[32] = {0};               // Int ya ocupa 4bytes
    char MemoriaPrincipal[16384]; // La ram es unidimensional un byte tras otro
    Segmento TablaSegmentos[8];            // 0 cs, 1 ds,
    uint16_t LargoCod;
    char cabecera[6];

    const char *filename;
    // argv[0] = nombre del propio programa (ej: "./vmx"), siempre est�
    // argv[1] = filename.vmx (obligatorio)
    // argv[2] = "-d" (opcional)
    // hay q compilar el maquina antes de usarla asi esta la ultima version
    // en mi mac es         chmod +x vmx        gcc MV0.c -o vmx    ./vmx prueba1.vmx -d
    if (argc < 2)
    {
        printf("Uso: vmx filename.vmx [-d]\n");
    }
    else
    {
        filename = argv[1];
        int mostrar_disassembler = 0;

        if (argc >= 3 && strcmp(argv[2], "-d") == 0)
        {
            mostrar_disassembler = 1;
        }

        printf("Archivo a ejecutar: %s\n", filename);
        printf("Modo disassembler: %s\n", mostrar_disassembler ? "SI" : "NO");

        // Ac� seguir�a: abrir el archivo, leer la cabecera, cargar en memoria,
        // ejecutar (y si mostrar_disassembler, imprimir el disassembler)
        Lectura(MemoriaPrincipal, filename, cabecera, &LargoCod);
        AsignarSegmentos(LargoCod, TablaSegmentos, Registros);

        if (mostrar_disassembler)
            MostrarCodigo(MemoriaPrincipal, cabecera, LargoCod);
    }
    EjecutarMaquina(MemoriaPrincipal, TablaSegmentos, Registros);
    return 0;
}
