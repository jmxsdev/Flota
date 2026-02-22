#include <stdio.h>
#include <string.h>
#include <ctype.h>

// Constantes para límites (memoria estática)
#define MAX_FERRIES 3
#define MAX_VEHICULOS_POR_FERRY 20  // Capacidad máxima de vehículos en un ferry
#define MAX_VEHICULOS_COLA 100       // Máximo en cola de espera
#define MAX_PASAJEROS_POR_VEHICULO 20
#define MAX_LARGO_PLACA 10
#define NOMBRE_FERRY_MAX 30

// Constantes para tipos de ferry
#define TIPO_EXPRESS 0
#define TIPO_TRADICIONAL 1

// Constantes para estados del ferry
#define ESTADO_CARGA 1
#define ESTADO_VIAJE 2
#define ESTADO_ESPERA 3

// Estructura para representar un vehículo
struct Vehiculo {
    // Datos del archivo de entrada
    int codigo;                 // Código del vehículo (define el tipo)
    int num_adultos;            // Número de pasajeros adultos
    int num_tercera_edad;       // Número de pasajeros tercera edad
    int tipo_pasaje_adultos;    // 0=Primera/VIP, 1=Turista/Ejecutiva
    int tipo_pasaje_tercera_edad; // 0=Primera/VIP, 1=Turista/Ejecutiva
    int peso;                   // Peso en kg (excepto carga que es toneladas)
    int hora_llegada;           // Hora militar (ej: 830 = 8:30)
    char placa[MAX_LARGO_PLACA]; // Placa del vehículo
    int tipo_ferry;             // 0=Express, 1=Tradicional
    
    // Campos calculados/deducidos
    int tipo_vehiculo;           // 0=liviano, 1=rústico, 2=van, 3=carga, 
                                 // 4=ambulancia, 5=bomberos, 6=policía
    int es_emergencia;           // 1 si es ambulancia, bomberos o policía
    float peso_toneladas;        // Peso convertido a toneladas
    int total_pasajeros;         // adultos + tercera edad + conductor?
};

// Explicación de cómo deducir el tipo de vehículo del código:
// - Códigos 0-99: liviano
// - Códigos 100-199: rústico
// - Códigos 200-299: microbus/van
// - Códigos 300-399: carga
// - Códigos 400-499: ambulancia (emergencia)
// - Códigos 500-599: bomberos (emergencia)
// - Códigos 600-699: policía (emergencia)

// Estructura para representar un ferry
struct Ferry {
    // Datos fijos del ferry
    int id;                         // 1, 2 o 3
    char nombre[NOMBRE_FERRY_MAX];  // Nombre del ferry
    int tipo;                        // TIPO_EXPRESS o TIPO_TRADICIONAL
    
    // Capacidades
    int capacidad_vehiculos;         // Máximo número de vehículos
    float peso_maximo_toneladas;     // Peso máximo que soporta
    
    // Capacidad de pasajeros por clase
    int capacidad_clase1;            // Primera clase (Tradicional) o VIP (Express)
    int capacidad_clase2;            // Turista (Tradicional) o Ejecutiva (Express)
    
    // Tiempos
    int tiempo_viaje;                // Minutos que tarda en viajar (65 o 35)
    
    // Estado actual
    int estado;                      // 1=carga, 2=viaje, 3=espera
    int tiempo_restante;              // Minutos restantes para terminar carga/viaje
    
    // Vehículos a bordo (cola interna del ferry)
    struct Vehiculo vehiculos_a_bordo[MAX_VEHICULOS_POR_FERRY];
    int num_vehiculos_abordo;        // Cantidad actual
    
    // Estadísticas del viaje actual
    float peso_actual_toneladas;     // Peso total a bordo
    int pasajeros_actuales;           // Total pasajeros a bordo
    int pasajeros_mayores_actuales;   // Mayores de 60 a bordo
    
    // Estadísticas acumuladas
    int total_viajes_realizados;
    int total_vehiculos_transportados;
    int total_pasajeros_transportados;
    int total_pasajeros_mayores;
    float total_ingresos;
};

// Explicación de capacidades según el enunciado:
// Ferry 1 - Lilia Concepción (Express):
//   - 16 vehículos, 50 pasajeros (30 ejecutiva, 20 VIP)
//   - 60 toneladas máximo
//   - 35 minutos de viaje
//
// Ferry 2 - La Isabela (Tradicional):
//   - 20 vehículos, 70 pasajeros (50 turista, 20 primera)
//   - 80 toneladas máximo
//   - 65 minutos de viaje
//
// Ferry 3 - La Margariteña (Tradicional):
//   - 18 vehículos, 60 pasajeros (40 turista, 20 primera)
//   - 80 toneladas máximo
//   - 65 minutos de viaje

// Estructura para cola de vehículos (para espera en terminal)
struct ColaVehiculos {
    struct Vehiculo elementos[MAX_VEHICULOS_COLA];
    int frente;     // Índice del primer elemento (para desencolar)
    int final;      // Índice del último elemento (para encolar)
    int cantidad;   // Número actual de elementos en la cola
};

// Estructura principal que contiene todo el estado de la simulación
struct Simulacion {
    // Ferrys
    struct Ferry ferrys[MAX_FERRIES];
    
    // Colas de espera en terminal
    struct ColaVehiculos cola_express;
    struct ColaVehiculos cola_tradicional;
    
    // Orden de carga (cola de ferrys esperando para cargar)
    int orden_carga[MAX_FERRIES];     // Array con IDs en orden
    int indice_orden_actual;           // Índice del ferry que está cargando ahora
    
    // Tiempo de simulación
    int tiempo_actual_minutos;         // Minutos desde inicio
    int hora_inicio;                   // Hora del primer vehículo
    
    // Estadísticas globales
    int total_vehiculos_dia;
    int total_pasajeros_dia;
    int total_pasajeros_no_trasladados;
    float total_ingresos_dia;
    
    // Para estadística de frecuencia máxima en espera
    int max_vehiculos_espera;
    int hora_max_vehiculos_espera;
};


// =============================================================================
//                            PROTOTIPOS DE FUNCIONES
// =============================================================================

// --- Funciones de Inicialización ---
void inicializarSimulacion(struct Simulacion *sim);
void inicializarTodosFerrys(struct Ferry ferrys[MAX_FERRIES]);
void inicializarFerry(struct Ferry *ferry, int id);
void inicializarCola(struct ColaVehiculos *cola);

// --- Funciones de Cola de Vehículos ---
int colaVacia(struct ColaVehiculos *cola);
int colaLlena(struct ColaVehiculos *cola);
int encolar(struct ColaVehiculos *cola, struct Vehiculo v);
struct Vehiculo desencolar(struct ColaVehiculos *cola);
struct Vehiculo verFrente(struct ColaVehiculos *cola);

// --- Funciones de Procesamiento de Archivo y Datos ---
int procesarArchivoCompleto(const char *nombre_archivo, 
                            struct ColaVehiculos *cola_express,
                            struct ColaVehiculos *cola_tradicional,
                            int orden_carga[MAX_FERRIES],
                            int *hora_inicio);
void procesarVehiculo(struct Vehiculo *v);

// --- Funciones de Validación ---
int validarVehiculo(struct Vehiculo *v);
int validarHoraMilitar(int hora);
int validarPasajeros(int num_adultos, int num_tercera_edad);
int validarTipoPasaje(int tpa, int tpt);
int validarPeso(int codigo, int peso);
int validarTipoFerry(int tipo_ferry);
int validarPlaca(const char *placa);

// --- Funciones Utilitarias de Tiempo ---
int horaMilitarAMinutos(int hora_militar);
int minutosAHoraMilitar(int minutos);


// =============================================================================
//                               FUNCIÓN PRINCIPAL
// =============================================================================
int main() {
    
    // Inicializar simulación
    struct Simulacion simulacion;
    inicializarSimulacion(&simulacion);
    
    // Procesar archivo completo (lectura, validación y carga en colas)
    int total_vehiculos = procesarArchivoCompleto("proy1.in", 
                                                &simulacion.cola_express, 
                                                &simulacion.cola_tradicional, 
                                                simulacion.orden_carga, 
                                                &simulacion.hora_inicio);

    if (total_vehiculos > 0) {
        simulacion.tiempo_actual_minutos = horaMilitarAMinutos(simulacion.hora_inicio);
    }
    
    // Aquí continuaría la simulación...
    
    return 0;
}

// =============================================================================
//                      IMPLEMENTACIÓN DE FUNCIONES
// =============================================================================

// -----------------------------------------------------------------------------
//                      Funciones de Inicialización
// -----------------------------------------------------------------------------

// Función para inicializar toda la simulación
void inicializarSimulacion(struct Simulacion *sim) {
    // Inicializar todos los ferrys
    inicializarTodosFerrys(sim->ferrys);
    
    // Inicializar colas
    inicializarCola(&sim->cola_express);
    inicializarCola(&sim->cola_tradicional);
    
    sim->indice_orden_actual = 0;
    
    // Tiempo (se actualizará al leer primer vehículo)
    sim->tiempo_actual_minutos = 0;
    sim->hora_inicio = 0;
    
    // Inicializar estadísticas
    sim->total_vehiculos_dia = 0;
    sim->total_pasajeros_dia = 0;
    sim->total_pasajeros_no_trasladados = 0;
    sim->total_ingresos_dia = 0.0;
    sim->max_vehiculos_espera = 0;
    sim->hora_max_vehiculos_espera = 0;
}

// Función para inicializar todos los ferrys
void inicializarTodosFerrys(struct Ferry ferrys[MAX_FERRIES]) {
    for (int i = 0; i < MAX_FERRIES; i++) {
        inicializarFerry(&ferrys[i], i + 1);  // i+1 porque los IDs son 1,2,3
    }
}

// Función para inicializar un ferry con sus datos específicos
void inicializarFerry(struct Ferry *ferry, int id) {
    // Primero, limpiar toda la estructura (poner en 0)
    memset(ferry, 0, sizeof(struct Ferry));
    
    // Asignar ID
    ferry->id = id;
    
    // Configurar según el ID
    switch(id) {
        case 1: // Lilia Concepción - Express
            strcpy(ferry->nombre, "Lilia Concepcion");
            ferry->tipo = TIPO_EXPRESS;
            ferry->capacidad_vehiculos = 16;
            ferry->peso_maximo_toneladas = 60.0;
            ferry->capacidad_clase1 = 20;  // VIP
            ferry->capacidad_clase2 = 30;  // Ejecutiva
            ferry->tiempo_viaje = 35;
            break;
            
        case 2: // La Isabela - Tradicional
            strcpy(ferry->nombre, "La Isabela");
            ferry->tipo = TIPO_TRADICIONAL;
            ferry->capacidad_vehiculos = 20;
            ferry->peso_maximo_toneladas = 80.0;
            ferry->capacidad_clase1 = 20;  // Primera clase
            ferry->capacidad_clase2 = 50;  // Turista
            ferry->tiempo_viaje = 65;
            break;
            
        case 3: // La Margariteña - Tradicional
            strcpy(ferry->nombre, "La Margaritenia");
            ferry->tipo = TIPO_TRADICIONAL;
            ferry->capacidad_vehiculos = 18;
            ferry->peso_maximo_toneladas = 80.0;
            ferry->capacidad_clase1 = 20;  // Primera clase
            ferry->capacidad_clase2 = 40;  // Turista
            ferry->tiempo_viaje = 65;
            break;
    }
    
    // Estado inicial: todos en espera (3) hasta que empiece la simulación
    ferry->estado = ESTADO_ESPERA;
    ferry->tiempo_restante = 0;
    
    // No hay vehículos a bordo inicialmente
    ferry->num_vehiculos_abordo = 0;
    ferry->peso_actual_toneladas = 0;
    ferry->pasajeros_actuales = 0;
    ferry->pasajeros_mayores_actuales = 0;
    
    // Estadísticas en cero
    ferry->total_viajes_realizados = 0;
    ferry->total_vehiculos_transportados = 0;
    ferry->total_pasajeros_transportados = 0;
    ferry->total_pasajeros_mayores = 0;
    ferry->total_ingresos = 0.0;
}

// Función para inicializar una cola vacía
void inicializarCola(struct ColaVehiculos *cola) {
    cola->frente = 0;
    cola->final = -1;  // -1 indica que no hay elementos
    cola->cantidad = 0;
}

// -----------------------------------------------------------------------------
//                      Funciones de Cola de Vehículos
// -----------------------------------------------------------------------------

// Función para verificar si la cola está vacía
int colaVacia(struct ColaVehiculos *cola) {
    return cola->cantidad == 0;
}

// Función para verificar si la cola está llena
int colaLlena(struct ColaVehiculos *cola) {
    return cola->cantidad >= MAX_VEHICULOS_COLA;
}

// Función para encolar (agregar al final)
int encolar(struct ColaVehiculos *cola, struct Vehiculo v) {
    if (colaLlena(cola)) {
        printf("Error: Cola llena, no se puede encolar\n");
        return 0;  // Error
    }
    
    // Actualizar índice final (circular)
    cola->final = (cola->final + 1) % MAX_VEHICULOS_COLA;
    cola->elementos[cola->final] = v;
    cola->cantidad++;
    return 1;  // Éxito
}

// Función para desencolar (quitar del frente)
struct Vehiculo desencolar(struct ColaVehiculos *cola) {
    struct Vehiculo vacio = {0};  // Vehículo vacío para retornar si hay error
    
    if (colaVacia(cola)) {
        printf("Error: Cola vacía, no se puede desencolar\n");
        return vacio;
    }
    
    struct Vehiculo resultado = cola->elementos[cola->frente];
    cola->frente = (cola->frente + 1) % MAX_VEHICULOS_COLA;
    cola->cantidad--;
    
    // Si la cola queda vacía, resetear índices
    if (cola->cantidad == 0) {
        cola->frente = 0;
        cola->final = -1;
    }
    
    return resultado;
}

// Función para ver el frente sin desencolar
struct Vehiculo verFrente(struct ColaVehiculos *cola) {
    struct Vehiculo vacio = {0};
    
    if (colaVacia(cola)) {
        return vacio;
    }
    
    return cola->elementos[cola->frente];
}

// -----------------------------------------------------------------------------
//                 Funciones de Procesamiento de Archivo y Datos
// -----------------------------------------------------------------------------

// 2.1 FUNCIÓN COMPLETA PARA PROCESAR ARCHIVO
int procesarArchivoCompleto(const char *nombre_archivo, 
                            struct ColaVehiculos *cola_express,
                            struct ColaVehiculos *cola_tradicional,
                            int orden_carga[MAX_FERRIES],
                            int *hora_inicio) {
    
    FILE *archivo = fopen(nombre_archivo, "r");
    if (archivo == NULL) {
        printf("ERROR: No se pudo abrir el archivo %s\n", nombre_archivo);
        return 0;  // Error
    }
    
    // PASO 1: Leer la primera línea (orden de carga)
    if (fscanf(archivo, "%d %d %d", 
               &orden_carga[0], 
               &orden_carga[1], 
               &orden_carga[2]) != 3) {
        printf("ERROR: Formato incorrecto en primera línea\n");
        fclose(archivo);
        return 0;
    }

    //Imprime el orden de carga segun el archivo
    printf("Orden de carga inicial: ");
    for (int i = 0; i < MAX_FERRIES; i++) {
        printf("%d ", orden_carga[i]);
    }
    printf("\n");
    
    // Validar que los números sean 1,2,3 en algún orden
    int validar_orden[4] = {0};  // Índices 1,2,3 para verificar
    for (int i = 0; i < MAX_FERRIES; i++) {
        if (orden_carga[i] < 1 || orden_carga[i] > 3) {
            printf("ERROR: Número de ferry inválido: %d\n", orden_carga[i]);
            fclose(archivo);
            return 0;
        }
        validar_orden[orden_carga[i]]++;
    }
    // Verificar que cada ferry aparece exactamente una vez
    for (int i = 1; i <= 3; i++) {
        if (validar_orden[i] != 1) {
            printf("ERROR: El ferry %d debe aparecer exactamente una vez\n", i);
            fclose(archivo);
            return 0;
        }
    }
    
    // PASO 2: Leer todos los vehículos
    struct Vehiculo v;
    int primer_vehiculo = 1;
    int contador_vehiculos = 0;
    
    while (1) {
        // Leer una línea completa
        int leidos = fscanf(archivo, "%d %d %d %d %d %d %d %s %d", 
                           &v.codigo, 
                           &v.num_adultos, 
                           &v.num_tercera_edad,
                           &v.tipo_pasaje_adultos, 
                           &v.tipo_pasaje_tercera_edad,
                           &v.peso, 
                           &v.hora_llegada, 
                           v.placa, 
                           &v.tipo_ferry);
        
        // Si llegamos al final del archivo, salir
        if (leidos == EOF) break;
        
        // Verificar que leímos todos los campos
        if (leidos != 9) {
            printf("ERROR: Línea con formato incorrecto (vehículo %d)\n", 
                   contador_vehiculos + 1);
            fclose(archivo);
            return 0;
        }
        
        // PASO 3: Validar el vehículo (llamada a función de validación)
        if (!validarVehiculo(&v)) {
            printf("ERROR: Vehículo con datos inválidos (placa: %s)\n", v.placa);
            fclose(archivo);
            return 0;
        }
        
        // PASO 4: Procesar el vehículo (calcular campos derivados)
        procesarVehiculo(&v);
        
        // Si es el primer vehículo, establecer hora de inicio
        if (primer_vehiculo) {
            *hora_inicio = v.hora_llegada;
            primer_vehiculo = 0;
        }
        
        // PASO 5: Colocar en la cola correspondiente
        if (v.tipo_ferry == 0) {  // Express
            if (!encolar(cola_express, v)) {
                printf("ERROR: Cola Express llena\n");
                fclose(archivo);
                return 0;
            }
        } else {  // Tradicional
            if (!encolar(cola_tradicional, v)) {
                printf("ERROR: Cola Tradicional llena\n");
                fclose(archivo);
                return 0;
            }
        }
        
        contador_vehiculos++;
        
        // Opcional: imprimir progreso
        printf("Vehículo %d leído: %s a las %d\n", 
               contador_vehiculos, v.placa, v.hora_llegada);
    }
    
    fclose(archivo);
    
    printf("Archivo procesado exitosamente. %d vehículos cargados.\n", 
           contador_vehiculos);
    
    return contador_vehiculos;  // Retornar número de vehículos leídos
}

// Función para convertir hora militar (ej: 830) a minutos desde medianoche
int horaMilitarAMinutos(int hora_militar) {
    int horas = hora_militar / 100;
    int minutos = hora_militar % 100;
    return horas * 60 + minutos;
}

// -----------------------------------------------------------------------------
//                      Funciones Utilitarias de Tiempo
// -----------------------------------------------------------------------------

// Función para convertir minutos a hora militar
int minutosAHoraMilitar(int minutos) {
    int horas = minutos / 60;
    int mins = minutos % 60;
    return horas * 100 + mins;
}

// Función para procesar un vehículo leído del archivo
void procesarVehiculo(struct Vehiculo *v) {
    // Calcular total de pasajeros (conductor + adultos + tercera edad)
    // El conductor cuenta como adulto si no se especifica lo contrario
    v->total_pasajeros = v->num_adultos + v->num_tercera_edad + 1; // +1 por conductor
    
    // Determinar tipo de vehículo por el código
    if (v->codigo >= 0 && v->codigo <= 99) {
        v->tipo_vehiculo = 0;  // liviano
        v->es_emergencia = 0;
    } else if (v->codigo >= 100 && v->codigo <= 199) {
        v->tipo_vehiculo = 1;  // rústico
        v->es_emergencia = 0;
    } else if (v->codigo >= 200 && v->codigo <= 299) {
        v->tipo_vehiculo = 2;  // microbus/van
        v->es_emergencia = 0;
    } else if (v->codigo >= 300 && v->codigo <= 399) {
        v->tipo_vehiculo = 3;  // carga
        v->es_emergencia = 0;
        // Para carga, el peso ya viene en toneladas
        v->peso_toneladas = (float)v->peso;
    } else if (v->codigo >= 400 && v->codigo <= 499) {
        v->tipo_vehiculo = 4;  // ambulancia
        v->es_emergencia = 1;
    } else if (v->codigo >= 500 && v->codigo <= 599) {
        v->tipo_vehiculo = 5;  // bomberos
        v->es_emergencia = 1;
    } else if (v->codigo >= 600 && v->codigo <= 699) {
        v->tipo_vehiculo = 6;  // policía
        v->es_emergencia = 1;
    }
    
    // Convertir peso a toneladas si no es carga
    if (v->tipo_vehiculo != 3) {  // No es carga
        v->peso_toneladas = (float)v->peso / 1000.0;  // kg a toneladas
    }
}

// -----------------------------------------------------------------------------
//                      Funciones de Validación
// -----------------------------------------------------------------------------

// 2.2 FUNCIONES DE VALIDACIÓN COMPLETAS

// Validar que la hora militar sea correcta (0000 a 2359)
int validarHoraMilitar(int hora) {
    int horas = hora / 100;
    int minutos = hora % 100;
    
    // Validar horas (0-23)
    if (horas < 0 || horas > 23) {
        printf("ERROR: Hora inválida (%d): horas deben ser 0-23\n", hora);
        return 0;
    }
    
    // Validar minutos (0-59)
    if (minutos < 0 || minutos > 59) {
        printf("ERROR: Hora inválida (%d): minutos deben ser 0-59\n", hora);
        return 0;
    }
    
    return 1;
}

// Validar rangos de pasajeros
int validarPasajeros(int num_adultos, int num_tercera_edad) {
    // Según enunciado: rango [1..20] para cada uno
    if (num_adultos < 1 || num_adultos > 20) {
        printf("ERROR: Número de adultos (%d) debe ser 1-20\n", num_adultos);
        return 0;
    }
    
    if (num_tercera_edad < 0 || num_tercera_edad > 20) {
        printf("ERROR: Número de tercera edad (%d) debe ser 1-20\n", num_tercera_edad);
        return 0;
    }
    
    // Validar que no excedan capacidad máxima por vehículo
    if (num_adultos + num_tercera_edad > 20) {
        printf("ERROR: Total pasajeros (%d) excede máximo 20\n", 
               num_adultos + num_tercera_edad);
        return 0;
    }
    
    return 1;
}

// Validar tipo de pasaje
int validarTipoPasaje(int tpa, int tpt) {
    // Según enunciado: 0 = primera clase/VIP, 1 = turista/ejecutiva
    if (tpa != 0 && tpa != 1) {
        printf("ERROR: Tipo pasaje adultos (%d) debe ser 0 o 1\n", tpa);
        return 0;
    }
    
    if (tpt != 0 && tpt != 1) {
        printf("ERROR: Tipo pasaje tercera edad (%d) debe ser 0 o 1\n", tpt);
        return 0;
    }
    
    return 1;
}

// Validar peso según tipo de vehículo
int validarPeso(int codigo, int peso) { 
    // Primero determinar tipo por código
    int tipo = codigo / 100;  // 0,1,2,3,4,5,6
    
    switch(tipo) {
        case 0: // Liviano
        case 1: // Rústico
        case 2: // Microbus/Van
            if (peso < 500 || peso > 5000) {
                printf("ERROR: Peso vehículo liviano (%d kg) fuera de rango\n", peso);
                return 0;
            }
            break;
            
        case 3: // Carga (en toneladas)
            if (peso < 1 || peso > 30) {
                printf("ERROR: Peso vehículo carga (%d ton) fuera de rango\n", peso);
                return 0;
            }
            break;
            
        case 4: // Ambulancia
        case 5: // Bomberos
        case 6: // Policía
            if (peso < 1500 || peso > 8000) {
                printf("ERROR: Peso vehículo emergencia (%d kg) fuera de rango\n", peso);
                return 0;
            }
            break;
            
        default:
            printf("ERROR: Código de vehículo inválido: %d\n", codigo);
            return 0;
    }
    
    return 1;
}

// Validar tipo de ferry
int validarTipoFerry(int tipo_ferry) {
    // Según enunciado: 0 = Express, 1 = Tradicional
    if (tipo_ferry != 0 && tipo_ferry != 1) {
        printf("ERROR: Tipo de ferry (%d) debe ser 0 o 1\n", tipo_ferry);
        return 0;
    }
    return 1;
}

// Validar formato de placa (formato venezolano típico: letras y números)
int validarPlaca(const char *placa) {
    int longitud = strlen(placa);
    
    // Longitud típica de placa venezolana: 6-7 caracteres
    if (longitud < 6 || longitud > 7) {
        printf("ERROR: Longitud de placa inválida: %s\n", placa);
        return 0;
    }
    
    // Verificar que sean caracteres alfanuméricos
    for (int i = 0; i < longitud; i++) {
        if (!isalnum(placa[i])) {
            printf("ERROR: Carácter inválido en placa: %s\n", placa);
            return 0;
        }
    }
    
    return 1;
}

// FUNCIÓN PRINCIPAL DE VALIDACIÓN (integra todas las anteriores)
int validarVehiculo(struct Vehiculo *v) {
    // Validar hora de llegada
    if (!validarHoraMilitar(v->hora_llegada)) {
        return 0;
    }
    
    // Validar pasajeros
    if (!validarPasajeros(v->num_adultos, v->num_tercera_edad)) {
        return 0;
    }
    
    // Validar tipo de pasaje
    if (!validarTipoPasaje(v->tipo_pasaje_adultos, v->tipo_pasaje_tercera_edad)) {
        return 0;
    }
    
    // Validar peso según código
    if (!validarPeso(v->codigo, v->peso)) {
        return 0;
    }
    
    // Validar tipo de ferry
    if (!validarTipoFerry(v->tipo_ferry)) {
        return 0;
    }
    
    // Validar placa
    if (!validarPlaca(v->placa)) {
        return 0;
    }
    
    // Validar que el código sea consistente con el tipo de ferry
    // (los vehículos de emergencia pueden ir en cualquier ferry, pero los demás no tienen restricción explícita)
    
    return 1;  // Todo válido
}