#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <math.h>

#define MAX_PLAZAS       50
#define LONG_PATENTE     8
#define ARCHIVO_ESTADO   "cochera_state.bin"
#define ARCHIVO_CONFIG   "cochera_cfg.txt"

/* ------ Tipos de datos ------ */
typedef enum {AUTO = 0, MOTO = 1, CAMION = 2} TipoVehiculo;

typedef struct {
    char patente[LONG_PATENTE];
    TipoVehiculo tipo;
    time_t horaEntrada;
    bool ocupada;
} Plaza;

/* Estructura empaquetada para persistir */
typedef struct {
    Plaza plazas[MAX_PLAZAS];
    double recaudacionTotal;
} EstadoPersistente;

/* ------ Variables globales ------ */
static Plaza cochera[MAX_PLAZAS];
static double tarifaPrimeraHora[3] = {500.0, 300.0, 800.0};
static double tarifaHoraExtra[3]   = {300.0, 150.0, 500.0};
static double recaudacionTotal = 0.0;

/* ------ Prototipos ------ */
void cargarEstado(void);
void guardarEstado(void);
void cargarConfiguracion(void);
void guardarConfiguracion(void);

void inicializarCochera(void);
void menu(void);
void ingresarVehiculo(void);
void retirarVehiculo(void);
void mostrarEstado(void);
void buscarVehiculo(void);
void modificarTarifas(void);
void mostrarRecaudacion(void);

int encontrarPlazaLibre(void);
int buscarPorPatente(const char *patente);
double calcularImporte(TipoVehiculo tipo, time_t horaEntrada, time_t horaSalida);
const char* tipoVehiculoStr(TipoVehiculo tipo);

/* ======================================================== */
int main(void)
{
    inicializarCochera();       /* Array en memoria limpio */
    cargarEstado();             /* Intenta recuperar ocupación + recaudación */
    cargarConfiguracion();      /* Intenta recuperar tarifas personalizadas */

    menu();                     /* Bucle principal */

    guardarEstado();            /* Persistir plazas + recaudación */
    guardarConfiguracion();     /* Persistir tarifas */

    return 0;
}

/* ---------------- Persistencia ---------------- */
void cargarEstado(void)
{
    FILE *fp = fopen(ARCHIVO_ESTADO, "rb");
    if (!fp) return; /* No existe => primera vez */

    EstadoPersistente st;
    if (fread(&st, sizeof(EstadoPersistente), 1, fp) == 1) {
        memcpy(cochera, st.plazas, sizeof(cochera));
        recaudacionTotal = st.recaudacionTotal;
    }
    fclose(fp);
}

void guardarEstado(void)
{
    EstadoPersistente st;
    memcpy(st.plazas, cochera, sizeof(cochera));
    st.recaudacionTotal = recaudacionTotal;

    FILE *fp = fopen(ARCHIVO_ESTADO, "wb");
    if (!fp) {
        perror("Error al guardar estado");
        return;
    }
    fwrite(&st, sizeof(EstadoPersistente), 1, fp);
    fclose(fp);
}

void cargarConfiguracion(void)
{
    FILE *fp = fopen(ARCHIVO_CONFIG, "r");
    if (!fp) return; /* Valores por defecto */

    for (int i = 0; i < 3; ++i) {
        if (fscanf(fp, "%lf %lf", &tarifaPrimeraHora[i], &tarifaHoraExtra[i]) != 2) {
            break; /* Cortar si el archivo está malformado */
        }
    }
    fclose(fp);
}

void guardarConfiguracion(void)
{
    FILE *fp = fopen(ARCHIVO_CONFIG, "w");
    if (!fp) {
        perror("Error al guardar configuración");
        return;
    }
    for (int i = 0; i < 3; ++i) {
        fprintf(fp, "%lf %lf\n", tarifaPrimeraHora[i], tarifaHoraExtra[i]);
    }
    fclose(fp);
}

/* --------------- Inicialización ---------------- */
void inicializarCochera(void)
{
    for (int i = 0; i < MAX_PLAZAS; ++i) {
        cochera[i].ocupada = false;
    }
}

/* ------------------ Auxiliares ----------------- */
const char* tipoVehiculoStr(TipoVehiculo tipo)
{
    switch (tipo) {
        case AUTO:   return "Auto";
        case MOTO:   return "Moto";
        case CAMION: return "Camión";
        default:     return "Desconocido";
    }
}

int encontrarPlazaLibre(void)
{
    for (int i = 0; i < MAX_PLAZAS; ++i) {
        if (!cochera[i].ocupada) return i;
    }
    return -1;
}

int buscarPorPatente(const char *patente)
{
    for (int i = 0; i < MAX_PLAZAS; ++i) {
        if (cochera[i].ocupada && strcmp(cochera[i].patente, patente) == 0) {
            return i;
        }
    }
    return -1;
}

double calcularImporte(TipoVehiculo tipo, time_t horaEntrada, time_t horaSalida)
{
    double horas = difftime(horaSalida, horaEntrada) / 3600.0; /* en horas */
    int horasRedondeadas = (int)ceil(horas);
    if (horasRedondeadas <= 1) {
        return tarifaPrimeraHora[tipo];
    }
    return tarifaPrimeraHora[tipo] + (horasRedondeadas - 1) * tarifaHoraExtra[tipo];
}

/* ------------------- Menú ---------------------- */
void menu(void)
{
    int opcion;
    do {
        printf("\n===== ADMINISTRADOR DE COCHERA =====\n");
        printf("1. Ingresar vehículo\n");
        printf("2. Retirar vehículo\n");
        printf("3. Mostrar estado de plazas\n");
        printf("4. Buscar vehículo por patente\n");
        printf("5. Modificar tarifas\n");
        printf("6. Ver recaudación total\n");
        printf("7. Salir\n");
        printf("Seleccione una opción: ");

        if (scanf("%d", &opcion) != 1) {
            fprintf(stderr, "\nEntrada inválida. Intente de nuevo.\n");
            while (getchar() != '\n');
            continue;
        }
        switch (opcion) {
            case 1: ingresarVehiculo(); break;
            case 2: retirarVehiculo(); break;
            case 3: mostrarEstado(); break;
            case 4: buscarVehiculo(); break;
            case 5: modificarTarifas(); break;
            case 6: mostrarRecaudacion(); break;
            case 7: printf("Guardando y saliendo...\n"); break;
            default: printf("Opción no válida.\n");
        }
    } while (opcion != 7);
}

/* --------------- Funciones de menú ------------- */
void ingresarVehiculo(void)
{
    int plazaLibre = encontrarPlazaLibre();
    if (plazaLibre == -1) {
        printf("\nNo hay plazas disponibles.\n");
        return;
    }

    char patente[LONG_PATENTE];
    printf("Ingrese la patente (ej.: ABC123): ");
    scanf("%7s", patente);

    if (buscarPorPatente(patente) != -1) {
        printf("\nEse vehículo ya se encuentra en la cochera.\n");
        return;
    }

    printf("Seleccione tipo de vehículo:\n  0. Auto\n  1. Moto\n  2. Camión\n  > ");
    int tipoInt;
    if (scanf("%d", &tipoInt) != 1 || tipoInt < 0 || tipoInt > 2) {
        printf("Tipo inválido. Operación cancelada.\n");
        return;
    }

    cochera[plazaLibre].tipo = (TipoVehiculo)tipoInt;
    strcpy(cochera[plazaLibre].patente, patente);
    cochera[plazaLibre].horaEntrada = time(NULL);
    cochera[plazaLibre].ocupada = true;

    printf("\nVehículo ingresado en plaza %d (%s) a las %s", plazaLibre + 1,
           tipoVehiculoStr(cochera[plazaLibre].tipo), ctime(&cochera[plazaLibre].horaEntrada));
}

void retirarVehiculo(void)
{
    char patente[LONG_PATENTE];
    printf("Ingrese la patente del vehículo a retirar: ");
    scanf("%7s", patente);

    int idx = buscarPorPatente(patente);
    if (idx == -1) {
        printf("Vehículo no encontrado.\n");
        return;
    }

    time_t horaSalida = time(NULL);
    double importe = calcularImporte(cochera[idx].tipo, cochera[idx].horaEntrada, horaSalida);

    printf("\n------ Ticket de salida ------\n");
    printf("Plaza           : %d\n", idx + 1);
    printf("Patente         : %s\n", cochera[idx].patente);
    printf("Tipo            : %s\n", tipoVehiculoStr(cochera[idx].tipo));
    printf("Hora de entrada : %s", ctime(&cochera[idx].horaEntrada));
    printf("Hora de salida  : %s", ctime(&horaSalida));
    printf("Importe a pagar : $%.2f\n", importe);
    printf("------------------------------\n");

    recaudacionTotal += importe;
    cochera[idx].ocupada = false;
}

void mostrarEstado(void)
{
    printf("\n---- Estado de las plazas ----\n");
    printf("Plaza | Estado  | Patente | Tipo  | Hora Entrada\n");
    printf("------+---------+---------+-------+---------------------------\n");
    for (int i = 0; i < MAX_PLAZAS; ++i) {
        if (cochera[i].ocupada) {
            char horaStr[26];
            ctime_r(&cochera[i].horaEntrada, horaStr);
            horaStr[strcspn(horaStr, "\n")] = '\0';
            printf("%5d | Ocupada | %-7s | %-5s | %s\n", i + 1, cochera[i].patente,
                   tipoVehiculoStr(cochera[i].tipo), horaStr);
        } else {
            printf("%5d | Libre   | ------- | ----- | ---------------------------\n", i + 1);
        }
    }
}

void buscarVehiculo(void)
{
    char patente[LONG_PATENTE];
    printf("Ingrese la patente a buscar: ");
    scanf("%7s", patente);

    int idx = buscarPorPatente(patente);
    if (idx == -1) {
        printf("Vehículo no encontrado.\n");
        return;
    }

    char horaStr[26];
    ctime_r(&cochera[idx].horaEntrada, horaStr);
    horaStr[strcspn(horaStr, "\n")] = '\0';

    printf("\nVehículo encontrado en plaza %d\n", idx + 1);
    printf("Tipo           : %s\n", tipoVehiculoStr(cochera[idx].tipo));
    printf("Hora de entrada: %s\n", horaStr);
}

void modificarTarifas(void)
{
    printf("\n---- Modificar tarifas ----\n");
    for (int i = 0; i < 3; ++i) {
        printf("%s:\n", tipoVehiculoStr(i));
        printf("  Tarifa primera hora actual: $%.2f | Nueva: ", tarifaPrimeraHora[i]);
        scanf("%lf", &tarifaPrimeraHora[i]);
        printf("  Tarifa hora extra actual  : $%.2f | Nueva: ", tarifaHoraExtra[i]);
        scanf("%lf", &tarifaHoraExtra[i]);
    }
    printf("Tarifas actualizadas con éxito.\n");
}

void mostrarRecaudacion(void)
{
    printf("\nRecaudación total histórica: $%.2f\n", recaudacionTotal);
}
