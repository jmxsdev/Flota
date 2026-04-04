#include <stdio.h>
#include <string.h>
#include <ctype.h>

// Constantes para límites (memoria estática)
#define MAX_FERRIES 3
#define MAX_VEHICULOS_POR_FERRY 20  // Capacidad máxima de vehículos en un ferry
#define TIEMPO_DE_CARGA_VEHICULO 3                                    // 
#define MAX_VEHICULOS_COLA 10000       // Máximo en cola de espera
#define MAX_PASAJEROS_POR_VEHICULO 20
#define MAX_LARGO_PLACA 10
#define NOMBRE_FERRY_MAX 30
#define MAX_VEHICULOS_ESPERA 200

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
    int region;                 // Es de nueva esparta, anzoategui u otra region
    int tiene_pasajeros;        // Si tiene un chofer pero no pasajeros 0,
                                // si el pasajero es el chofer 1
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
    int tiempo_restante_viaje;              // Minutos restantes para terminar carga/viaje
    int tiempo_restante_carga;
    
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

// Estructura con estado
struct VehiculoConEstado {
    struct Vehiculo datos;
    int activo;              // 1 = en cola de espera, 0 = ya asignado
    int timestamp_asignacion; // Para seguimiento en simulación
};

// Estructura para cola de vehículos (para espera en terminal)
struct ColaVehiculos {
    struct VehiculoConEstado elementos[MAX_VEHICULOS_COLA];
    int frente;              // Siempre apunta al primer vehículo que llegó
    int final;               // Siempre apunta a la última posición insertada
    int cantidad;            // Número TOTAL de vehículos (activos + inactivos)
};


// Estructura principal que contiene todo el estado de la simulación
struct Simulacion {
    // Ferrys
    struct Ferry ferrys[MAX_FERRIES];
    
    // Colas de espera en terminal
    struct ColaVehiculos cola_espera;
    struct ColaVehiculos cola_todos_vehiculos;
    
    // Orden de carga (cola de ferrys esperando para cargar)
    int orden_carga[MAX_FERRIES];     // Array con IDs en orden
    int indice_orden_actual;           // Índice del ferry que está cargando ahora
    
    // Tiempo de simulación
    int tiempo_actual_minutos;         // Minutos desde inicio
    int hora_inicio;                   // Hora del primer vehículo
    int tiempo_carga_restante;
    
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
int insertarEnColaEspera(struct ColaVehiculos *cola, struct Vehiculo v, struct Simulacion *sim, int ferry_idx);
//Cola de todos los vehiculos
int encolar_nuevo(struct ColaVehiculos *cola, struct Vehiculo nuevo);
int buscar_proximo_vehiculo(struct ColaVehiculos *cola, int tipo_ferry_buscado, int prioridad_emergencia);
int asignar_a_ferry(struct ColaVehiculos *cola, int tipo_ferry, struct Vehiculo *asignado, int timesatmp);
int reinsertar_vehiculo(struct Simulacion *sim, struct Vehiculo vehiculo, int origen);
// --- Funciones de Procesamiento de Archivo y Datos ---
int procesarArchivoCompleto(const char *nombre_archivo, 
                            struct ColaVehiculos *cola_todos_vehiculos,
                            int orden_carga[MAX_FERRIES],
                            int *hora_inicio);
void procesarVehiculo(struct Vehiculo *v);

// --- Funciones Utilitarias de Tiempo ---
int horaMilitarAMinutos(int hora_militar);
int minutosAHoraMilitar(int minutos);

// --- Funciones de Validación ---
int validarCod(int codigo);
int validarHoraMilitar(int hora);
int validarPasajeros(int num_adultos, int num_tercera_edad);
int validarTipoPasaje(int tpa, int tpt);
int validarPeso(int codigo, int peso);
int validarTipoFerry(int tipo_ferry);
int validarPlaca(const char *placa);
int validarVehiculo(struct Vehiculo *v);

// --- Funciones de Simulaciòn ---
void iniciarSimulacion(FILE *in, struct Simulacion *sim);
int cabeEnFerry(struct Simulacion *sim, int ferry_idx, struct Vehiculo v);
int cabeVehiculo(struct Ferry *ferry, struct Simulacion *sim);
void cargarVehiculoDesdeEspera(struct Simulacion *sim, int ferry_idx);
void actualizarColaEsperaDesdeOrigenes(struct Simulacion *sim, int ferry_idx);
int hayVehiculosEnCola(struct Simulacion *sim, int ferry_idx, int hora_carga);
void manejarEmergenciaPrioritaria(struct Simulacion *sim, int ferry_idx, struct Vehiculo emergencia);
void marcarVehiculoComoAsignado(struct Simulacion *sim, char *placa);
int puedeViajar(struct Simulacion *sim, int ferry_idx, int hora_actual);
void iniciarViaje(FILE *in, struct Simulacion *sim, int ferry_idx);
int terminarSimulacion(struct Simulacion *sim, int ferry_idx, int hora_actual);
void actualizarEstadosFerrys(struct Simulacion *sim);
// --- Funciones de Estadisticas ---
void generarReporteViaje(struct Simulacion *sim, int ferry_idx);
void generarReporteViajeArchivo(FILE *in, struct Simulacion *sim, int ferry_idx);
void actualizarEstadisticasEspera(struct Simulacion *sim);
void imprimirEstadisticasArchivo(FILE *in, struct Simulacion *sim);
void imprimirEstadisticas(struct Simulacion *sim);
void calcularVehiculoMasFrecuente(struct Simulacion *sim, char *tipo_str);
void calcularEstadisticasFinales(struct Simulacion *sim, int ferry_idx);
float calcularIngresoVehiculo(struct Vehiculo v, int tipoFerry);
void actualizarEstadisticasBajada(struct Ferry *ferry, struct Vehiculo *bajado);



// =============================================================================
//                               FUNCIÓN PRINCIPAL
// =============================================================================
int main() {
    
    // Inicializar simulación
    FILE *in = fopen("proy_1.out", "w");
    struct Simulacion simulacion;
    inicializarSimulacion(&simulacion);
    
    // Procesar archivo completo (lectura, validación y carga en colas)
    int total_vehiculos = procesarArchivoCompleto("proy1.in", 
                                                &simulacion.cola_todos_vehiculos, 
                                                simulacion.orden_carga, 
                                                &simulacion.hora_inicio);

    if (total_vehiculos > 0) {
        simulacion.tiempo_actual_minutos = horaMilitarAMinutos(simulacion.hora_inicio);
        iniciarSimulacion(in,&simulacion);

    }

    
    fclose(in);
    // Aquí continuaría la simulación...
    
    return 0;
}

// =============================================================================
//                      IMPLEMENTACIÓN DE FUNCIONES
// =============================================================================

// -----------------------------------------------------------------------------
//                      Funciones de Inicialización
// -----------------------------------------------------------------------------

/**
 * @brief Inicializa la estructura principal de la simulación.
 * @param sim Puntero a la estructura `Simulacion` a inicializar.
 * @return void
 */
void inicializarSimulacion(struct Simulacion *sim) {
    // Inicializar todos los ferrys
    inicializarTodosFerrys(sim->ferrys);
    
    // Inicializar colas
    inicializarCola(&sim->cola_todos_vehiculos);
    inicializarCola(&sim->cola_espera);
        
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

/**
 * @brief Inicializa todos los `Ferry` del arreglo.
 * @param ferrys Arreglo de `Ferry` de tamaño `MAX_FERRIES`.
 * @return void
 */
void inicializarTodosFerrys(struct Ferry ferrys[MAX_FERRIES]) {
    for (int i = 0; i < MAX_FERRIES; i++) {
        inicializarFerry(&ferrys[i], i + 1);  // i+1 porque los IDs son 1,2,3
    }
}

/**
 * @brief Inicializa un `Ferry` con parámetros predeterminados según su id.
 * @param ferry Puntero al `Ferry` a inicializar.
 * @param id Identificador del ferry (1..3).
 * @return void
 */
void inicializarFerry(struct Ferry *ferry, int id) {
    // Primero, limpiar toda la estructura (poner en 0)
    //memset(ferry, 0, sizeof(struct Ferry));
    
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
            strcpy(ferry->nombre, "La Margariteña");
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
    ferry->tiempo_restante_viaje = 0;
    ferry->tiempo_restante_carga = 0;
    
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

/**
 * @brief Inicializa una `ColaVehiculos` en estado vacío.
 * @param cola Puntero a la cola a inicializar.
 * @return void
 */
void inicializarCola(struct ColaVehiculos *cola) {
    cola->frente = 0;
    cola->final = 0;  // -1 indica que no hay elementos
    cola->cantidad = 0;
}

// -----------------------------------------------------------------------------
//                      Funciones de Cola de Vehículos
// -----------------------------------------------------------------------------

/**
 * @brief Comprueba si la `ColaVehiculos` está vacía.
 * @param cola Puntero a la cola a comprobar.
 * @return 1 si la cola está vacía, 0 en caso contrario.
 */
int colaVacia(struct ColaVehiculos *cola) {
    return cola->cantidad == 0;
}

/**
 * @brief Comprueba si la `ColaVehiculos` está llena.
 * @param cola Puntero a la cola a comprobar.
 * @return 1 si la cola está llena, 0 en caso contrario.
 */
int colaLlena(struct ColaVehiculos *cola) {
    return cola->cantidad >= MAX_VEHICULOS_COLA;
}

//================Cola de todos los vehiculos=====================

/**
 * @brief Encola un nuevo vehículo al final de la cola general.
 * @param cola Puntero a la `ColaVehiculos`.
 * @param nuevo Vehículo a encolar.
 * @return 1 si se encoló correctamente, 0 si la cola estaba llena.
 */
int encolar_nuevo(struct ColaVehiculos *cola, struct Vehiculo nuevo) {
    if (cola->cantidad == MAX_VEHICULOS_COLA) {
        return 0;  // Cola llena
    }

     
    cola->elementos[cola->final].datos = nuevo;
    cola->elementos[cola->final].activo = 1;  // Activo por defecto
    cola->elementos[cola->final].timestamp_asignacion = 0;
    cola->final = (cola->final + 1) % MAX_VEHICULOS_COLA;
    cola->cantidad++;
    
    return 1;
}

/**
 * @brief Busca el índice del próximo vehículo activo en una cola según criterios.
 * @param cola Puntero a la `ColaVehiculos` donde se busca.
 * @param tipo_ferry_buscado Tipo de ferry buscado (-1 para cualquiera).
 * @param prioridad_emergencia Si es 1, prioriza vehículos de emergencia.
 * @return Índice dentro del arreglo `cola->elementos` del vehículo encontrado,
 *         o -1 si no hay vehículos activos que cumplan los criterios.
 *
 * Descripción (español): Recorre la cola circular respetando el orden sin
 * modificar `frente` o `final`. Primero busca emergencias (si aplica), luego
 * vehículos del tipo solicitado y finalmente cualquier vehículo activo.
 */
int buscar_proximo_vehiculo(struct ColaVehiculos *cola, int tipo_ferry_buscado, int prioridad_emergencia) {
    if (cola->cantidad == 0) return -1;
    
    int indice_actual = cola->frente;
    int elementos_revisados = 0;
    
    // Caso 1: Buscar emergencias primero si tienen prioridad
    if (prioridad_emergencia) {
        for (int i = 0; i < cola->cantidad; i++) {
            int idx = (cola->frente + i) % MAX_VEHICULOS_COLA;
            if (cola->elementos[idx].activo && 
                cola->elementos[idx].datos.es_emergencia == 1 &&
                (tipo_ferry_buscado == -1 || cola->elementos[idx].datos.tipo_ferry == tipo_ferry_buscado)) {
                return idx;
            }
        }
    }
    
    // Caso 2: Buscar vehículo del tipo específico
    if (tipo_ferry_buscado != -1) {
        for (int i = 0; i < cola->cantidad; i++) {
            int idx = (cola->frente + i) % MAX_VEHICULOS_COLA;
            if (cola->elementos[idx].activo && 
                cola->elementos[idx].datos.tipo_ferry == tipo_ferry_buscado) {
                return idx;
            }
        }
    }
    
    // Caso 3: Buscar cualquier vehículo activo
    for (int i = 0; i < cola->cantidad; i++) {
        int idx = (cola->frente + i) % MAX_VEHICULOS_COLA;
        if (cola->elementos[idx].activo) {
            return idx;
        }
    }
    
    return -1;  // No hay vehículos activos
}

// Función para asignar un vehículo a un ferry (desencolar virtual)
/**
 * @brief Asigna (virtualmente) un vehículo de la cola a un ferry.
 * @param cola Puntero a la `ColaVehiculos` de donde se tomará el vehículo.
 * @param tipo_ferry Tipo de ferry para el que se busca el vehículo.
 * @param asignado Puntero donde se copiará el vehículo asignado.
 * @param timestamp Marca de tiempo (minutos) de la asignación.
 * @return 1 si se asignó un vehículo, 0 si no había vehículos disponibles.
 *
 * Descripción (español): Busca el próximo vehículo (priorizando emergencias)
 * y marca su `activo` como 0 para indicar que fue asignado, además de fijar
 * `timestamp_asignacion`.
 */
int asignar_a_ferry(struct ColaVehiculos *cola, int tipo_ferry, struct Vehiculo *asignado, int timestamp) {
    int indice = buscar_proximo_vehiculo(cola, tipo_ferry, 1);  // Prioridad emergencias
    
    if (indice != -1) {
        *asignado = cola->elementos[indice].datos;
        cola->elementos[indice].activo = 0;  // Marcar como asignado
        cola->elementos[indice].timestamp_asignacion = timestamp;
        return 1;
    }
    
    return 0;  // No hay vehículos disponibles
}


// -----------------------------------------------------------------------------
//                 Funciones de Procesamiento de Archivo y Datos
// -----------------------------------------------------------------------------

/**
 * @brief Procesa el archivo de entrada completo y encola los vehículos leídos.
 * @param nombre_archivo Ruta al archivo de entrada (ej. "proy1.in").
 * @param cola_todos_vehiculos Puntero a la cola donde se almacenan todos los vehículos.
 * @param orden_carga Array (3 elementos) que recibirá el orden de carga de ferrys.
 * @param hora_inicio Puntero donde se almacenará la hora del primer vehículo leído.
 * @return Número de vehículos leídos y encolados, 0 en caso de error.
 */
int procesarArchivoCompleto(const char *nombre_archivo, 
                            struct ColaVehiculos *cola_todos_vehiculos,
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
       // printf("validar_orden: %d %d %d %d\n", validar_orden[0], validar_orden[1], validar_orden[2], validar_orden[3]);
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
        if (!encolar_nuevo(cola_todos_vehiculos, v)) {
            printf("ERROR: Cola Prioridad llena\n");
            fclose(archivo);
            return 0;
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

/**
 * @brief Calcula campos derivados de un `Vehiculo` leído (tipo, emergencia, peso_toneladas, total_pasajeros).
 * @param v Puntero al `Vehiculo` a procesar.
 * @return void
 */
void procesarVehiculo(struct Vehiculo *v) {
    int temp = v->codigo;
    int c1 = temp / 100;
  
    temp %= 100;
    int c2 = temp / 10;

    int c3 = temp % 10;

    v->tiene_pasajeros = c3;
    v->region = c2;

    // Determinar tipo de vehículo por el código
    switch(c1){
      case 1:
           v->tipo_vehiculo = 1;  // liviano
           v->es_emergencia = 0;
           break;
      case 2:
           v->tipo_vehiculo = 2;  // rustico
           v->es_emergencia = 0;
           break;
      case 3:
           v->tipo_vehiculo = 3;  // microbus / van
           v->es_emergencia = 0;
           break;
      case 4:
           v->tipo_vehiculo = 4;  //Carga
           v->es_emergencia = 0;
           v->peso_toneladas = (float)v->peso;
           break;
      case 5:
           v->tipo_vehiculo = 5;  //Ambulancia
           v->es_emergencia = 1;
           break;
      case 6:
           v->tipo_vehiculo = 6;  //Bombero
           v->es_emergencia = 1;
      case 7:
           v->tipo_vehiculo = 7;  //Policia
           v->es_emergencia = 1;
           break;
    }
    if (v->tipo_vehiculo != 4) {  // No es carga
        v->peso_toneladas = (float)v->peso / 1000.0;  // kg a toneladas
    }
         // El conductor cuenta como adulto si no se especifica lo contrario
    v->total_pasajeros = v->num_adultos + v->num_tercera_edad + 1;
    
}

// -----------------------------------------------------------------------------
//                      Funciones Utilitarias de Tiempo
// -----------------------------------------------------------------------------

/**
 * @brief Convierte una hora en formato militar (HHMM) a minutos desde medianoche.
 * @param hora_militar Hora en formato HHMM (ej: 830 para 08:30).
 * @return Minutos desde medianoche.
 */
int horaMilitarAMinutos(int hora_militar) {
    int horas = hora_militar / 100;
    int minutos = hora_militar % 100;
    return horas * 60 + minutos;
}

/**
 * @brief Convierte minutos desde medianoche a hora en formato militar (HHMM).
 * @param minutos Minutos desde medianoche.
 * @return Hora en formato HHMM.
 */
int minutosAHoraMilitar(int minutos) {
    int horas = minutos / 60;
    int mins = minutos % 60;
    return horas * 100 + mins;
}

// -----------------------------------------------------------------------------
//                      Funciones de Validación
// -----------------------------------------------------------------------------

// 2.2 FUNCIONES DE VALIDACIÓN COMPLETAS

/**
 * @brief Valida la estructura del código del vehículo (3 dígitos: tipo, región, flag).
 * @param codigo Código entero de 3 dígitos.
 * @return 1 si el código es válido, 0 si es inválido.
 */
int validarCod(int codigo){
  
  int temp = codigo;
  int c1 = temp / 100;
  
  temp %= 100;
  int c2 = temp / 10;

  int c3 = temp % 10;

  if (c1 > 7 || c1 < 0) {
    printf("ERROR: El codigo Cod debe contener tres digitos\n");
    return 0;
  }

  if (c2 > 2 || c2 < 0){
    printf("ERROR: El segundo digito (c2) del codigo Cod debe estar ente 0 - 2\n");
    return 0;
  }

  if (c3 < 0 || c3 > 1) {
    printf("ERROR: El tercer digito (c3) del codigo Cod debe ser 0 o 1\n");
    return 0;
  }

  return 1;
}

/**
 * @brief Valida que una hora militar esté en el rango 0000-2359.
 * @param hora Hora en formato HHMM.
 * @return 1 si válida, 0 si inválida.
 */
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

/**
 * @brief Valida los rangos de pasajeros por vehículo.
 * @param num_adultos Número de pasajeros adultos.
 * @param num_tercera_edad Número de pasajeros de tercera edad.
 * @return 1 si los valores son válidos, 0 si hay error.
 */
int validarPasajeros(int num_adultos, int num_tercera_edad) {
    // Según enunciado: rango [1..20] para cada uno
    if (num_adultos < 0 || num_adultos > 20) {
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

/**
 * @brief Valida los tipos de pasaje para adultos y tercera edad.
 * @param tpa Tipo de pasaje adultos (0 o 1).
 * @param tpt Tipo de pasaje tercera edad (0 o 1).
 * @return 1 si válidos, 0 si inválidos.
 */
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

/**
 * @brief Valida el peso según el tipo de vehículo deducido del código.
 * @param codigo Código del vehículo (para inferir el tipo).
 * @param peso Peso ingresado (kg o toneladas según el tipo).
 * @return 1 si el peso está en el rango válido, 0 si no.
 */
int validarPeso(int codigo, int peso) { 
    // Primero determinar tipo por código
    int tipo = codigo / 100;  // 0,1,2,3,4,5,6
    
    switch(tipo) {
        case 1: // Liviano
        case 2: // Rústico
        case 3: // Microbus/Van
            if (peso < 500 || peso > 5000) {
                printf("ERROR: Peso vehículo liviano (%d kg) fuera de rango\n", peso);
                return 0;
            }
            break;
            
        case 4: // Carga (en toneladas)
            if (peso < 1 || peso > 15) {
                printf("ERROR: Peso vehículo carga (%d ton) fuera de rango\n", peso);
                return 0;
            }
            break;
            
        case 5: // Ambulancia
        case 6: // Bomberos
        case 7: // Policía
            if (peso < 500 || peso > 8000) {
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

/**
 * @brief Valida el campo `tipo_ferry` (0=Express, 1=Tradicional).
 * @param tipo_ferry Entero esperado 0 o 1.
 * @return 1 si válido, 0 si inválido.
 */
int validarTipoFerry(int tipo_ferry) {
    // Según enunciado: 0 = Express, 1 = Tradicional
    if (tipo_ferry != 0 && tipo_ferry != 1) {
        printf("ERROR: Tipo de ferry (%d) debe ser 0 o 1\n", tipo_ferry);
        return 0;
    }
    return 1;
}

/**
 * @brief Valida el formato de la placa (alfanumérica, longitud típica 6-7).
 * @param placa Cadena con la placa a validar.
 * @return 1 si válida, 0 si inválida.
 */
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

/**
 * @brief Valida un `Vehiculo` completo utilizando todas las validaciones auxiliares.
 * @param v Puntero al vehículo a validar.
 * @return 1 si el vehículo es válido, 0 si hay alguna validación que falla.
 */
int validarVehiculo(struct Vehiculo *v) {
  
    //Validar codigo
    if (!validarCod(v->codigo)) {
        return 0;
    }
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
        
    return 1;  // Todo válido
}

/**
 * @brief Bucle principal de la simulación que gestiona carga, viaje y espera.
 * @param in Puntero a archivo de salida donde se escribirán reportes de viaje.
 * @param sim Puntero a la estructura de simulación que contiene colas y ferrys.
 * @return void
 *
 * Descripción (español): Ejecuta el ciclo principal de la simulación. Gestiona
 * la cola de espera, intenta cargar vehículos, verifica condiciones de zarpe
 * mediante `puedeViajar`, inicia viajes con `iniciarViaje` y actualiza el tiempo
 * y estados de ferrys. Mantiene métricas como máxima espera y tiempo de carga.
 */
//=============================================================================
// FUNCIÓN PRINCIPAL DE SIMULACIÓN REFACTORIZADA
//=============================================================================
void iniciarSimulacion(FILE *in, struct Simulacion *sim) {
    int band = 1;
    sim->ferrys[sim->orden_carga[sim->indice_orden_actual] - 1].estado = ESTADO_CARGA;

    while (band) {
    
        int hora_carga = minutosAHoraMilitar(sim->tiempo_actual_minutos);
        int ferry_actual_idx = sim->orden_carga[sim->indice_orden_actual] - 1;
        // CASO 1: El ferry está en estado de CARGA
        if (sim->ferrys[ferry_actual_idx].estado == ESTADO_CARGA) {
            
            actualizarColaEsperaDesdeOrigenes(sim, ferry_actual_idx);
            // Actualizar estadísticas de espera (tamaño de cola de espera)
            if (sim->cola_espera.cantidad > sim->max_vehiculos_espera) {
                sim->max_vehiculos_espera = sim->cola_espera.cantidad;
                sim->hora_max_vehiculos_espera = hora_carga;
            }
            // Si hay tiempo de carga restante, decrementar
            if (sim->tiempo_carga_restante > 0) {
                sim->tiempo_carga_restante--;
                if (sim->tiempo_carga_restante > 0) {
                    printf("⏳ Cargando... tiempo restante: %d min\n", sim->tiempo_carga_restante);
                }
            }
            // Si no está cargando actualmente, intentar cargar un vehículo
            else if (sim->tiempo_carga_restante == 0) {
                // Intentar cargar desde la cola de espera

               // printf("Intentando cargar...\n");
                cargarVehiculoDesdeEspera(sim, ferry_actual_idx);
            }
            // Verificar si puede zarpar
            if (puedeViajar(sim, ferry_actual_idx, hora_carga)) {
                iniciarViaje(in, sim, ferry_actual_idx);
                printf("🚢 Ferry %s ZARPÓ a las %d\n", 
                       sim->ferrys[ferry_actual_idx].nombre, hora_carga);
                sim->tiempo_carga_restante = 0;                

                inicializarCola(&sim->cola_espera); // Limpiar cola de espera al zarpar

            }  
            // Si no puede zarpar, intentar cargar vehículos
            
        } 
        // CASO 2: El ferry NO está en carga (VIAJE o ESPERA)
        else {
            // Si está en espera y es su turno, ponerlo en carga
            if (sim->ferrys[ferry_actual_idx].estado == ESTADO_ESPERA) {
                sim->ferrys[ferry_actual_idx].estado = ESTADO_CARGA;
                printf("🔵 Ferry %s iniciando CARGA a las %d\n", 
                       sim->ferrys[ferry_actual_idx].nombre, hora_carga);
            }
            // Si está en viaje, no hacer nada (solo esperar)
        }

        // Avanzar el tiempo de si simulación (1 minutos por operación)
        sim->tiempo_actual_minutos += 1;

        // Actualizar estados de ferrys en viaje
        actualizarEstadosFerrys(sim);
        
        // Verificar si termina la simulación
        band = terminarSimulacion(sim, ferry_actual_idx, hora_carga);

        if (!band) {
            //Calcular estadisticas finales
            calcularEstadisticasFinales(sim, ferry_actual_idx);
        }
    }
    

    imprimirEstadisticas(sim);
    // Al finalizar, calcular e imprimir estadísticas finales

    imprimirEstadisticasArchivo(in, sim);
}


/**
 * @brief Mueve vehículos desde la cola general a la `cola_espera` del ferry según capacidad y hora.
 * @param sim Puntero a la simulación.
 * @param ferry_idx Índice del ferry que está cargando.
 * @return void
 */
void actualizarColaEsperaDesdeOrigenes(struct Simulacion *sim, int ferry_idx) {
    // Calcular capacidad restante del ferry
    int capacidad_restante = sim->ferrys[ferry_idx].capacidad_vehiculos - 
                             sim->ferrys[ferry_idx].num_vehiculos_abordo;
    int espacio_disponible = capacidad_restante - sim->cola_espera.cantidad;
    
    int hora_actual = minutosAHoraMilitar(sim->tiempo_actual_minutos);
    int movidos = 0;
    int tipo_ferry_actual = sim->ferrys[ferry_idx].tipo;  // TIPO_EXPRESS o TIPO_TRADICIONAL

    
    if (espacio_disponible <= 0) {
        return;  // No hay espacio para más vehículos en espera
    }
    
    
    
    // Recorrer la cola total en orden de llegada (desde frente hasta final)
    int indice_actual = sim->cola_todos_vehiculos.frente;
    int vehiculos_procesados = 0;

    while (vehiculos_procesados < sim->cola_todos_vehiculos.cantidad && 
           movidos < espacio_disponible) {
        
        struct VehiculoConEstado *vehiculo_estado = &sim->cola_todos_vehiculos.elementos[indice_actual];
   
        
        if (vehiculo_estado->activo == 1 && 
            vehiculo_estado->datos.hora_llegada <= hora_actual) {
            
            int es_apto = 0;
            if (vehiculo_estado->datos.es_emergencia == 1) {
                es_apto = 1;
            } else {
                es_apto = (vehiculo_estado->datos.tipo_ferry == tipo_ferry_actual);
            }
            
            if (es_apto) {
                // Verificar si cabe físicamente en el ferry
                if (cabeEnFerry(sim, ferry_idx, vehiculo_estado->datos)) {
                    // Insertar en cola de espera normalmente
                    int tipo_cola_original = vehiculo_estado->datos.es_emergencia ? 2 : 
                                             (vehiculo_estado->datos.tipo_ferry == TIPO_EXPRESS ? 0 : 1);
                    
                    if (insertarEnColaEspera(&sim->cola_espera, vehiculo_estado->datos, sim, ferry_idx)) {
                        vehiculo_estado->activo = 0;
                        movidos++;
                    }
                } 
                // Si es emergencia y NO cabe en el ferry, manejar prioridad
                else if (vehiculo_estado->datos.es_emergencia == 1) {
                    // Intentar hacer espacio para la emergencia
                    manejarEmergenciaPrioritaria(sim, ferry_idx, vehiculo_estado->datos);
                    vehiculo_estado->activo = 0;  // Ya fue procesada
                    movidos++;  // Considerar como movida (aunque no fue a cola_espera)
                }
            }
        }
        
        // Avanzar al siguiente
        indice_actual = (indice_actual + 1) % MAX_VEHICULOS_COLA;
        vehiculos_procesados++;
    }

    
    if (movidos > 0) {
        printf("  🔄 Movidos %d vehículos a cola de espera (Ferry %s)\n", 
               movidos,
               tipo_ferry_actual == TIPO_EXPRESS ? "Express" : "Tradicional");
    }
}

/**
 * @brief Determina si un vehículo cabe en un ferry según espacio, peso y capacidad de pasajeros.
 * @param sim Puntero a la simulación.
 * @param ferry_idx Índice del ferry a evaluar.
 * @param v Vehículo a evaluar.
 * @return 1 si cabe, 0 si no.
 */
int cabeEnFerry(struct Simulacion *sim, int ferry_idx, struct Vehiculo v) {
    struct Ferry ferry = sim->ferrys[ferry_idx];
    
    // Verificar capacidad de vehículos
    if (ferry.num_vehiculos_abordo >= ferry.capacidad_vehiculos) {
        return 0;
    }
    
    // Verificar peso
    if (ferry.peso_actual_toneladas + v.peso_toneladas > ferry.peso_maximo_toneladas) {
        return 0;
    }
    
    // Verificar capacidad de pasajeros
    int total_pasajeros_nuevos = v.num_adultos + v.num_tercera_edad;
    if (ferry.pasajeros_actuales + total_pasajeros_nuevos > 
        ferry.capacidad_clase1 + ferry.capacidad_clase2) {
        return 0;
    }
    
    return 1;
}
/**
 * @brief Baja un vehículo del ferry en la posición indicada y actualiza estadísticas.
 * @param sim Puntero a la simulación.
 * @param ferry_idx Índice del ferry.
 * @param posicion_vehiculo Posición del vehículo en `vehiculos_a_bordo`.
 * @return 1 si se bajó correctamente, 0 si hubo error.
 */
int bajarVehiculoDelFerry(struct Simulacion *sim, int ferry_idx, int posicion_vehiculo) {
    struct Ferry *ferry = &sim->ferrys[ferry_idx];
    
    if (posicion_vehiculo < 0 || posicion_vehiculo >= ferry->num_vehiculos_abordo) {
        return 0;
    }
    
    // Obtener el vehículo a bajar
    struct Vehiculo vehiculo_bajado = ferry->vehiculos_a_bordo[posicion_vehiculo];
    
    // Actualizar estadísticas del ferry (revertir)
    ferry->num_vehiculos_abordo--;
    ferry->peso_actual_toneladas -= vehiculo_bajado.peso_toneladas;
    int total_pasajeros = vehiculo_bajado.num_adultos + vehiculo_bajado.num_tercera_edad;
    ferry->pasajeros_actuales -= total_pasajeros;
    ferry->pasajeros_mayores_actuales -= vehiculo_bajado.num_tercera_edad;
    
    // Reorganizar el array de vehículos a bordo (desplazar hacia la izquierda)
    for (int i = posicion_vehiculo; i < ferry->num_vehiculos_abordo; i++) {
        ferry->vehiculos_a_bordo[i] = ferry->vehiculos_a_bordo[i + 1];
    }
    
    printf("  🔻 BAJADO: %s del ferry %s\n", 
           vehiculo_bajado.placa, ferry->nombre);
    
    // Reinsertar en cola total
    reinsertar_vehiculo(sim, vehiculo_bajado, 0);
    
    return 1;
}

/**
 * @brief Maneja la llegada de un vehículo de emergencia, intentando cargarlo o liberar espacio.
 * @param sim Puntero a la simulación.
 * @param ferry_idx Índice del ferry que se está evaluando.
 * @param emergencia Vehículo de emergencia que requiere prioridad.
 * @return void
 */
void manejarEmergenciaPrioritaria(struct Simulacion *sim, int ferry_idx, struct Vehiculo emergencia) {
    struct Ferry *ferry = &sim->ferrys[ferry_idx];
    int espacio_disponible = ferry->capacidad_vehiculos - ferry->num_vehiculos_abordo;
    
    // Caso 1: Hay espacio en el ferry para la emergencia
    if (espacio_disponible > 0) {
        // Verificar si cabe físicamente
        if (cabeEnFerry(sim, ferry_idx, emergencia)) {
            // Cargar la emergencia directamente
            int num = ferry->num_vehiculos_abordo;
            ferry->vehiculos_a_bordo[num] = emergencia;
            ferry->num_vehiculos_abordo++;
            ferry->peso_actual_toneladas += emergencia.peso_toneladas;
            ferry->pasajeros_actuales += (emergencia.num_adultos + emergencia.num_tercera_edad);
            
            printf("  🚨 EMERGENCIA %s cargada directamente en ferry %s\n", 
                   emergencia.placa, ferry->nombre);
            
            // Marcar como inactivo en cola total
            marcarVehiculoComoAsignado(sim, emergencia.placa);
            return;
        }
    }
    
    // Caso 2: No hay espacio, hay que hacer espacio
    printf("  ⚠️ No hay espacio para emergencia %s, buscando vehículo para bajar...\n", 
           emergencia.placa);
    
    // Buscar un vehículo normal (no emergencia) en el ferry para bajar
    int posicion_a_bajar = -1;
    for (int i = 0; i < ferry->num_vehiculos_abordo; i++) {
        struct Vehiculo *v = &ferry->vehiculos_a_bordo[i];
        if (v->es_emergencia == 0) {
            posicion_a_bajar = i;
            break;
        }
    }
    
    if (posicion_a_bajar != -1) {
        // Bajar vehículo normal
        bajarVehiculoDelFerry(sim, ferry_idx, posicion_a_bajar);
        
        // Ahora cargar la emergencia
        int num = ferry->num_vehiculos_abordo;
        ferry->vehiculos_a_bordo[num] = emergencia;
        ferry->num_vehiculos_abordo++;
        ferry->peso_actual_toneladas += emergencia.peso_toneladas;
        ferry->pasajeros_actuales += (emergencia.num_adultos + emergencia.num_tercera_edad);
        
        printf("  🚨 EMERGENCIA %s cargada en lugar del vehículo bajado\n", 
               emergencia.placa);
        
        // Marcar como inactivo en cola total
        marcarVehiculoComoAsignado(sim, emergencia.placa);
    } else {
        printf("  ❌ No se pudo hacer espacio para emergencia %s (solo hay emergencias en ferry)\n", 
               emergencia.placa);
    }
}

/**
 * @brief Marca un vehículo de la cola total como asignado para evitar procesarlo de nuevo.
 * @param sim Puntero a la simulación.
 * @param placa Placa del vehículo a marcar.
 * @return void
 */
void marcarVehiculoComoAsignado(struct Simulacion *sim, char *placa) {
    for (int i = 0; i < sim->cola_todos_vehiculos.cantidad; i++) {
        int idx = (sim->cola_todos_vehiculos.frente + i) % MAX_VEHICULOS_COLA;
        struct VehiculoConEstado *v_estado = &sim->cola_todos_vehiculos.elementos[idx];
        
        if (v_estado->activo == 1 && strcmp(v_estado->datos.placa, placa) == 0) {
            v_estado->activo = 0;
            v_estado->timestamp_asignacion = sim->tiempo_actual_minutos;
            break;
        }
    }
}

/**
 * @brief Encola un nuevo vehículo en la cola general con timestamp.
 * @param cola Puntero a la cola general.
 * @param v Vehículo a encolar.
 * @param timestamp Marca de tiempo (minutos) de llegada.
 * @return void
 */
void encolarNuevoVehiculo(struct ColaVehiculos *cola, struct Vehiculo v, int timestamp) {
    if (cola->cantidad >= MAX_VEHICULOS_COLA) {
        printf("Error: Cola total llena\n");
        return;
    }
    
    struct VehiculoConEstado nuevo;
    nuevo.datos = v;
    nuevo.activo = 1;
    nuevo.timestamp_asignacion = timestamp;
    
    cola->elementos[cola->final] = nuevo;
    cola->final = (cola->final + 1) % MAX_VEHICULOS_COLA;
    cola->cantidad++;
}

/**
 * @brief Inserta un vehículo en la `cola_espera` manteniendo orden por hora.
 * @param cola_espera Puntero a la `ColaVehiculos` que actúa como cola de espera.
 * @param v Vehículo a insertar en la cola de espera.
 * @param sim Puntero a la simulación (se usa para consultar capacidad del ferry).
 * @param ferry_idx Índice del ferry cuyo espacio se está reservando.
 * @return 1 si la inserción fue exitosa, 0 si no se pudo insertar (cola llena o falta de espacio).
 *
 * Descripción (español): Inserta el vehículo en `cola_espera` ordenando por
 * `hora_llegada`. Comprueba límites: capacidad de la cola y espacio disponible
 * en el ferry destino para evitar sobre-reservas.
 */
int insertarEnColaEspera(struct ColaVehiculos *cola_espera, struct Vehiculo v, 
                         struct Simulacion *sim, int ferry_idx) {
    // Verificar límite de la cola
    if (cola_espera->cantidad >= MAX_VEHICULOS_COLA) {
        return 0;
    }
    
    // Verificar si el ferry actual tiene capacidad para más vehículos
    int capacidad_restante_ferry = sim->ferrys[ferry_idx].capacidad_vehiculos - 
                                   sim->ferrys[ferry_idx].num_vehiculos_abordo;
    if (capacidad_restante_ferry <= 0) {
        return 0;
    }
    
    // La cola de espera no puede tener más vehículos que la capacidad restante del ferry
    if (cola_espera->cantidad >= capacidad_restante_ferry) {
        return 0;
    }
    
    // Crear el elemento con estado para la cola de espera
    struct VehiculoConEstado nuevo;
    nuevo.datos = v;
    nuevo.activo = 1;  // En cola de espera
    nuevo.timestamp_asignacion = sim->tiempo_actual_minutos;
    
    // Insertar manteniendo orden: por hora de llegada
    // Los vehículos de emergencia ya tienen su hora de llegada, 
    // por lo que se intercalan naturalmente según orden de llegada
    int i = cola_espera->cantidad - 1;
    
    // Buscar posición de inserción (ordenado por hora de llegada)
    while (i >= 0) {
        int idx = (cola_espera->frente + i) % MAX_VEHICULOS_COLA;
        if (v.hora_llegada >= cola_espera->elementos[idx].datos.hora_llegada) {
            break;
        }
        i--;
    }
    
    // Insertar en la posición encontrada
    int pos_insercion = (cola_espera->frente + i + 1) % MAX_VEHICULOS_COLA;
    
    // Si la inserción no es al final, necesitamos desplazar
    if (pos_insercion != cola_espera->final) {
        // Desplazar elementos hacia la derecha
        int indice_actual = cola_espera->final;
        int indice_anterior;
        
        for (int j = 0; j < cola_espera->cantidad - i - 1; j++) {
            indice_anterior = (indice_actual - 1 + MAX_VEHICULOS_COLA) % MAX_VEHICULOS_COLA;
            cola_espera->elementos[indice_actual] = cola_espera->elementos[indice_anterior];
            indice_actual = indice_anterior;
        }
    }
    
    cola_espera->elementos[pos_insercion] = nuevo;
    cola_espera->final = (cola_espera->final + 1) % MAX_VEHICULOS_COLA;
    cola_espera->cantidad++;
    
    return 1;
}
/**
 * @brief Carga el primer vehículo de la `cola_espera` en el ferry si cumple condiciones.
 * @param sim Puntero a la simulación.
 * @param ferry_idx Índice del ferry destino.
 * @return void
 */
void cargarVehiculoDesdeEspera(struct Simulacion *sim, int ferry_idx) {
    // Verificar si hay vehículos en cola de espera (ahora es ColaVehiculos)
    if (sim->cola_espera.cantidad == 0) return;
    
    // Obtener el primer vehículo de la cola de espera (orden de llegada)
    int idx_primero = sim->cola_espera.frente;
    struct VehiculoConEstado *ve_primero = &sim->cola_espera.elementos[idx_primero];
    
    // Verificar si es un vehículo válido y ya ha llegado
    int hora_actual = minutosAHoraMilitar(sim->tiempo_actual_minutos);
    if (ve_primero->datos.hora_llegada > hora_actual) return;
    
    // Verificar si cabe en el ferry
    if (cabeEnFerry(sim, ferry_idx, ve_primero->datos)) {
        // Cargar el vehículo al ferry
        int num = sim->ferrys[ferry_idx].num_vehiculos_abordo;
        sim->ferrys[ferry_idx].vehiculos_a_bordo[num] = ve_primero->datos;
        sim->ferrys[ferry_idx].num_vehiculos_abordo++;
        sim->ferrys[ferry_idx].peso_actual_toneladas += ve_primero->datos.peso_toneladas;
        
        int total_pasajeros = ve_primero->datos.num_adultos + ve_primero->datos.num_tercera_edad + 1;
        sim->ferrys[ferry_idx].pasajeros_actuales += total_pasajeros;
        sim->ferrys[ferry_idx].pasajeros_mayores_actuales += ve_primero->datos.num_tercera_edad;
        sim->ferrys[ferry_idx].total_ingresos += calcularIngresoVehiculo(ve_primero->datos, 
                                                                          sim->ferrys[ferry_idx].tipo);
        
        // Eliminar el primer elemento de la cola de espera (avanzar frente)
        sim->cola_espera.frente = (sim->cola_espera.frente + 1) % MAX_VEHICULOS_COLA;
        sim->cola_espera.cantidad--;
        
        printf("  ✅ CARGADO: %s en %s %s\n", 
               ve_primero->datos.placa, 
               sim->ferrys[ferry_idx].nombre,
               ve_primero->datos.es_emergencia ? "(EMERGENCIA)" : "");
        
        // Establecer tiempo de carga
        sim->tiempo_carga_restante = TIEMPO_DE_CARGA_VEHICULO;
    } 
    // Si es emergencia y NO cabe, intentar hacer espacio bajando vehículos normales
    else if (ve_primero->datos.es_emergencia == 1) {
        printf("  🚨 Emergencia %s no cabe, intentando hacer espacio bajando vehículos...\n", 
               ve_primero->datos.placa);
        
        // Array para almacenar los vehículos que vamos a bajar
        struct Vehiculo vehiculos_bajados[MAX_VEHICULOS_POR_FERRY];
        int num_bajados = 0;
        
        // Mientras la emergencia no quepa y haya vehículos normales en el ferry
        while (!cabeEnFerry(sim, ferry_idx, ve_primero->datos)) {
            // Buscar si hay algún vehículo normal en el ferry
            int hay_normal = 0;
            for (int i = 0; i < sim->ferrys[ferry_idx].num_vehiculos_abordo; i++) {
                if (sim->ferrys[ferry_idx].vehiculos_a_bordo[i].es_emergencia == 0) {
                    hay_normal = 1;
                    break;
                }
            }
            
            if (!hay_normal) {
                break;  // No hay más vehículos normales para bajar
            }
            
            // Buscar el ÚLTIMO vehículo normal en el ferry (para mantener orden)
            int posicion_a_bajar = -1;
            for (int i = sim->ferrys[ferry_idx].num_vehiculos_abordo - 1; i >= 0; i--) {
                if (sim->ferrys[ferry_idx].vehiculos_a_bordo[i].es_emergencia == 0) {
                    posicion_a_bajar = i;
                    break;
                }
            }
            
            if (posicion_a_bajar != -1) {
                struct Vehiculo bajado = sim->ferrys[ferry_idx].vehiculos_a_bordo[posicion_a_bajar];
                vehiculos_bajados[num_bajados] = bajado;
                num_bajados++;
                
                printf("    🔻 Bajando %s del ferry (%.2f ton, %d pasajeros)\n", 
                       bajado.placa, bajado.peso_toneladas, 
                       bajado.num_adultos + bajado.num_tercera_edad);
                
                // Actualizar estadísticas del ferry (revertir)
                sim->ferrys[ferry_idx].num_vehiculos_abordo--;
                sim->ferrys[ferry_idx].peso_actual_toneladas -= bajado.peso_toneladas;
                int pasajeros_bajados = bajado.num_adultos + bajado.num_tercera_edad;
                sim->ferrys[ferry_idx].pasajeros_actuales -= pasajeros_bajados;
                sim->ferrys[ferry_idx].pasajeros_mayores_actuales -= bajado.num_tercera_edad;
                
                // Reorganizar array (desplazar hacia la izquierda)
                for (int i = posicion_a_bajar; i < sim->ferrys[ferry_idx].num_vehiculos_abordo; i++) {
                    sim->ferrys[ferry_idx].vehiculos_a_bordo[i] = sim->ferrys[ferry_idx].vehiculos_a_bordo[i + 1];
                }
            }
        }
        
        // Después de bajar vehículos, verificar si ahora cabe la emergencia
        if (cabeEnFerry(sim, ferry_idx, ve_primero->datos)) {
            // Cargar la emergencia
            int num = sim->ferrys[ferry_idx].num_vehiculos_abordo;
            sim->ferrys[ferry_idx].vehiculos_a_bordo[num] = ve_primero->datos;
            sim->ferrys[ferry_idx].num_vehiculos_abordo++;
            sim->ferrys[ferry_idx].peso_actual_toneladas += ve_primero->datos.peso_toneladas;
            sim->ferrys[ferry_idx].pasajeros_actuales += (ve_primero->datos.num_adultos + 
                                                          ve_primero->datos.num_tercera_edad);
            sim->ferrys[ferry_idx].pasajeros_mayores_actuales += ve_primero->datos.num_tercera_edad;
            sim->ferrys[ferry_idx].total_ingresos += calcularIngresoVehiculo(ve_primero->datos, 
                                                                              sim->ferrys[ferry_idx].tipo);
            
            // Eliminar el primer elemento de la cola de espera (avanzar frente)
            sim->cola_espera.frente = (sim->cola_espera.frente + 1) % MAX_VEHICULOS_COLA;
            sim->cola_espera.cantidad--;
            
            printf("  🚨 EMERGENCIA %s cargada después de bajar %d vehículo(s)\n", 
                   ve_primero->datos.placa, num_bajados);
            
            sim->tiempo_carga_restante = TIEMPO_DE_CARGA_VEHICULO;
        } else {
            printf("  ❌ No se pudo hacer espacio para emergencia %s incluso después de bajar todos los vehículos normales\n", 
                   ve_primero->datos.placa);
            // La emergencia permanece en cola_espera
            return;
        }
        
        // Reinsertar TODOS los vehículos bajados en cola total (en orden inverso)
        for (int i = num_bajados - 1; i >= 0; i--) {
            reinsertar_vehiculo(sim, vehiculos_bajados[i], 0);
        }
        
        if (num_bajados > 0) {
            printf("  🔄 Reinsertados %d vehículos en cola total\n", num_bajados);
        }
    }
}

/**
 * @brief Reinstaura un vehículo previamente marcado como inactivo en la cola total.
 * @param sim Puntero a la simulación que contiene `cola_todos_vehiculos`.
 * @param vehiculo Estructura `Vehiculo` que se desea reinsertar.
 * @param origen Código que indica el origen de la reinserción (no usado actualmente).
 * @return 1 si se reactivó un vehículo existente, 0 si se agregó como nuevo.
 *
 * Descripción (español): Busca en `cola_todos_vehiculos` un elemento con la misma
 * placa y `activo==0`. Si se encuentra, lo reactiva; si no, lo encola como nuevo.
 */
int reinsertar_vehiculo(struct Simulacion *sim, struct Vehiculo vehiculo, int origen) {
    // Buscar en cola total (debe estar inactivo)
    for (int i = 0; i < sim->cola_todos_vehiculos.cantidad; i++) {
        int idx = (sim->cola_todos_vehiculos.frente + i) % MAX_VEHICULOS_COLA;
        struct VehiculoConEstado *v_estado = &sim->cola_todos_vehiculos.elementos[idx];
        
        if (!v_estado->activo && strcmp(v_estado->datos.placa, vehiculo.placa) == 0) {
            v_estado->activo = 1;
            v_estado->timestamp_asignacion = sim->tiempo_actual_minutos;
            printf("  🔄 Vehículo %s reinsertado en cola total\n", vehiculo.placa);
            return 1;
        }
    }
    
    // Si no se encuentra, agregarlo como nuevo
    printf("  ⚠️ Vehículo %s no encontrado, agregando como nuevo\n", vehiculo.placa);
    encolarNuevoVehiculo(&sim->cola_todos_vehiculos, vehiculo, sim->tiempo_actual_minutos);
    return 0;
}
/**
 * @brief Determina si un ferry puede zarpar según reglas de capacidad y colas.
 * @param sim Puntero a la simulación con colas y ferrys.
 * @param ferry_idx Índice del ferry a evaluar.
 * @param hora_actual Hora actual en formato militar (hhmm) para comparar llegadas.
 * @return 1 si el ferry puede zarpar, 0 en caso contrario.
 *
 * Descripción (español): Evalúa múltiples condiciones: mínimo de vehículos (30%),
 * cola de espera vacía, capacidad máxima alcanzada y presencia de vehículos aptos
 * en `cola_todos_vehiculos`. Considera emergencias como aptas para cualquier ferry.
 */
 int puedeViajar(struct Simulacion *sim, int ferry_idx, int hora_actual) {
    struct Ferry *ferry = &sim->ferrys[ferry_idx];
    
    // ===== VALIDACIÓN 1: Si no hay vehículos a bordo, no puede viajar =====
    if (ferry->num_vehiculos_abordo == 0) {
        return 0;
    }
    
    // ===== VALIDACIÓN 2: Verificar si el ferry está LLENO =====
    int ferry_lleno_vehiculos = (ferry->num_vehiculos_abordo >= ferry->capacidad_vehiculos);
    int ferry_lleno_pasajeros = (ferry->pasajeros_actuales >= (ferry->capacidad_clase1 + ferry->capacidad_clase2));
    int ferry_lleno_peso = (ferry->peso_actual_toneladas >= ferry->peso_maximo_toneladas);
    
    if (ferry_lleno_vehiculos || ferry_lleno_pasajeros || ferry_lleno_peso) {
        // Devolver vehículos de cola_espera a cola_total
        if (sim->cola_espera.cantidad > 0) {
            int indice_actual = sim->cola_espera.frente;
            for (int i = 0; i < sim->cola_espera.cantidad; i++) {
                struct VehiculoConEstado *v_estado = &sim->cola_espera.elementos[indice_actual];
                reinsertar_vehiculo(sim, v_estado->datos, 1);
                indice_actual = (indice_actual + 1) % MAX_VEHICULOS_COLA;
            }
            // Vaciar cola_espera
            inicializarCola(&sim->cola_espera);
        }
        return 1;
    }
    
    // ===== VALIDACIÓN 3: Verificar MÍNIMO de vehículos =====
    int minimo_vehiculos = (int)(0.30 * ferry->capacidad_vehiculos);
    if (ferry->num_vehiculos_abordo < minimo_vehiculos) {
        return 0;
    }
    
    // ===== VALIDACIÓN 4: Verificar vehículos en cola_espera =====
    if (sim->cola_espera.cantidad > 0) {
        // Verificar si algún vehículo en cola_espera PUEDE ser cargado
        int alguno_puede_cargarse = 0;
        int indice_actual = sim->cola_espera.frente;
        
        for (int i = 0; i < sim->cola_espera.cantidad; i++) {
            struct VehiculoConEstado *v_estado = &sim->cola_espera.elementos[indice_actual];
            if (cabeEnFerry(sim, ferry_idx, v_estado->datos)) {
                alguno_puede_cargarse = 1;
                break;
            }
            indice_actual = (indice_actual + 1) % MAX_VEHICULOS_COLA;
        }
        
        if (alguno_puede_cargarse) {
            // Intentar cargar el primer vehículo que pueda
            indice_actual = sim->cola_espera.frente;
            for (int i = 0; i < sim->cola_espera.cantidad; i++) {
                struct VehiculoConEstado *v_estado = &sim->cola_espera.elementos[indice_actual];
                if (cabeEnFerry(sim, ferry_idx, v_estado->datos)) {
                    // Cargar directamente
                    int num = ferry->num_vehiculos_abordo;
                    ferry->vehiculos_a_bordo[num] = v_estado->datos;
                    ferry->num_vehiculos_abordo++;
                    ferry->peso_actual_toneladas += v_estado->datos.peso_toneladas;
                    ferry->pasajeros_actuales += (v_estado->datos.num_adultos + v_estado->datos.num_tercera_edad);
                    ferry->pasajeros_mayores_actuales += v_estado->datos.num_tercera_edad;
                    ferry->total_ingresos += calcularIngresoVehiculo(v_estado->datos, ferry->tipo);
                    
                    // Eliminar de cola_espera (avanzar frente)
                    sim->cola_espera.frente = (sim->cola_espera.frente + 1) % MAX_VEHICULOS_COLA;
                    sim->cola_espera.cantidad--;
                    
                    sim->tiempo_carga_restante = TIEMPO_DE_CARGA_VEHICULO;
                    printf("  ✅ CARGADO DIRECTO: %s en %s\n", 
                           v_estado->datos.placa, ferry->nombre);
                    return 0;  // Seguir cargando
                }
                indice_actual = (indice_actual + 1) % MAX_VEHICULOS_COLA;
            }
            return 0;
        } else {
            // Vehículos en cola_espera NO pueden ser cargados, devolverlos
            int indice_actual = sim->cola_espera.frente;
            for (int i = 0; i < sim->cola_espera.cantidad; i++) {
                struct VehiculoConEstado *v_estado = &sim->cola_espera.elementos[indice_actual];
                reinsertar_vehiculo(sim, v_estado->datos, 1);
                indice_actual = (indice_actual + 1) % MAX_VEHICULOS_COLA;
            }
            inicializarCola(&sim->cola_espera);
            return 1;
        }
    }
    
    // ===== VALIDACIÓN 5: Verificar si hay vehículos aptos en cola_total =====
    int hay_vehiculos_aptos = 0;
    int indice_actual = sim->cola_todos_vehiculos.frente;
    
    for (int i = 0; i < sim->cola_todos_vehiculos.cantidad; i++) {
        struct VehiculoConEstado *v_estado = &sim->cola_todos_vehiculos.elementos[indice_actual];
        
        if (v_estado->activo == 1 && v_estado->datos.hora_llegada <= hora_actual) {
            int es_apto = 0;
            if (v_estado->datos.es_emergencia == 1) {
                es_apto = 1;
            } else {
                es_apto = (v_estado->datos.tipo_ferry == ferry->tipo);
            }
            
            if (es_apto && cabeEnFerry(sim, ferry_idx, v_estado->datos)) {
                hay_vehiculos_aptos = 1;
                break;
            }
        }
        indice_actual = (indice_actual + 1) % MAX_VEHICULOS_COLA;
    }
    
    if (hay_vehiculos_aptos) {
        return 0;
    }
    
    return 1;
} 
/**
 * @brief Inicia un viaje para un ferry, registra estadísticas y genera reportes.
 * @param in Archivo de salida (reporte) donde se volcará información del viaje.
 * @param sim Puntero a la estructura de simulación que contiene el estado global.
 * @param ferry_idx Índice del ferry que inicia el viaje en el arreglo `sim->ferrys`.
 * @return void
 *
 * Descripción (español): Actualiza el estado del ferry a `ESTADO_VIAJE`, establece
 * el tiempo restante de viaje, actualiza contadores y totales del día, genera los
 * reportes de consola y en archivo, y reinicia los contadores del ferry para el
 * siguiente ciclo de carga.
 */
//=============================================================================
// FUNCIÓN iniciarViaje
//=============================================================================
void iniciarViaje(FILE *in, struct Simulacion *sim, int ferry_idx) {
    
    // Acceder directamente al ferry usando el índice
    sim->ferrys[ferry_idx].estado = ESTADO_VIAJE;
    sim->ferrys[ferry_idx].tiempo_restante_viaje = sim->ferrys[ferry_idx].tiempo_viaje;
    
    // Actualizar estadísticas del ferry
    sim->ferrys[ferry_idx].total_viajes_realizados++;
    sim->ferrys[ferry_idx].total_vehiculos_transportados += sim->ferrys[ferry_idx].num_vehiculos_abordo;
    sim->ferrys[ferry_idx].total_pasajeros_transportados += sim->ferrys[ferry_idx].pasajeros_actuales;
    sim->ferrys[ferry_idx].total_pasajeros_mayores += sim->ferrys[ferry_idx].pasajeros_mayores_actuales;
    // 2. Actualizar estadísticas globales
    sim->total_vehiculos_dia += sim->ferrys[ferry_idx].num_vehiculos_abordo;
    sim->total_pasajeros_dia += sim->ferrys[ferry_idx].pasajeros_actuales;
    sim->total_ingresos_dia += sim->ferrys[ferry_idx].total_ingresos; 
    generarReporteViaje(sim, ferry_idx);
    generarReporteViajeArchivo(in, sim, ferry_idx);
    
    //Reiniciar contadores para el próximo viaje
    sim->ferrys[ferry_idx].num_vehiculos_abordo = 0;
    sim->ferrys[ferry_idx].peso_actual_toneladas = 0;
    sim->ferrys[ferry_idx].pasajeros_actuales = 0;
    sim->ferrys[ferry_idx].pasajeros_mayores_actuales = 0;
    
    // Cambiar al siguiente ferry en el orden de carga
    sim->indice_orden_actual = (sim->indice_orden_actual + 1) % MAX_FERRIES;
}

/**
 * @brief Actualiza los estados temporales de todos los ferrys (tiempo de viaje).
 * @param sim Puntero a la estructura de simulación con todos los ferrys.
 * @return void
 *
 * Descripción (español): Decrementa el contador `tiempo_restante_viaje` para cada
 * ferry que esté en `ESTADO_VIAJE`. Si el tiempo llega a cero o menos, cambia el
 * estado del ferry a `ESTADO_ESPERA` e imprime un mensaje informativo.
 */
//=============================================================================
// NUEVA FUNCIÓN: actualizarEstadosFerrys
//=============================================================================
void actualizarEstadosFerrys(struct Simulacion *sim) {
    for (int i = 0; i < MAX_FERRIES; i++) {
        if (sim->ferrys[i].estado == ESTADO_VIAJE) {
            sim->ferrys[i].tiempo_restante_viaje--;
            
            if (sim->ferrys[i].tiempo_restante_viaje <= 0) {
                sim->ferrys[i].estado = ESTADO_ESPERA;
                printf("🚢 Ferry %s llegó a puerto y está en ESPERA\n", 
                       sim->ferrys[i].nombre);
            }
        }
    }
}

/**
 * @brief Evalúa condiciones para determinar si la simulación debe terminar.
 * @param sim Puntero a la simulación con colas y ferrys.
 * @param ferry_idx Índice del ferry a evaluar.
 * @param hora_actual Hora actual en formato militar (hhmm) usada para comparaciones.
 * @return 0 si la simulación debe terminar, 1 si debe continuar.
 *
 * Descripción (español): Comprueba si no existen vehículos aptos para el ferry,
 * la cola de espera está vacía y el ferry no puede zarpar; en ese caso finaliza
 * la simulación e imprime un resumen del ferry.
 */
//=============================================================================
// FUNCIÓN terminarSimulacion (completada)
//=============================================================================
int terminarSimulacion(struct Simulacion *sim, int ferry_idx, int hora_actual) {
    struct Ferry *ferry = &sim->ferrys[ferry_idx];
    int tipo_ferry_actual = ferry->tipo;
    
    // Verificar si hay vehículos APTOS para este ferry en cola total
    int hay_vehiculos_aptos = 0;
    for (int i = 0; i < sim->cola_todos_vehiculos.cantidad; i++) {
        int idx = (sim->cola_todos_vehiculos.frente + i) % MAX_VEHICULOS_COLA;
        struct VehiculoConEstado *v_estado = &sim->cola_todos_vehiculos.elementos[idx];
        
        if (v_estado->activo == 1) {
            if (v_estado->datos.es_emergencia == 1 || 
                v_estado->datos.tipo_ferry == tipo_ferry_actual) {
                hay_vehiculos_aptos = 1;
                break;
            }
        }
    }
    
    int cola_espera_vacia = (sim->cola_espera.cantidad == 0);
    int puede_zarpar = puedeViajar(sim, ferry_idx, hora_actual);
    
    // TERMINAR si:
    // 1. No hay vehículos aptos para este ferry
    // 2. La cola de espera está vacía
    // 3. El ferry no puede zarpar (no cumple mínimo)
    if (!hay_vehiculos_aptos && cola_espera_vacia && !puede_zarpar) {
        printf("\n✅ Simulación terminada.\n");
        printf("   Ferry %s: %d/%d vehículos, mínimo: %d\n",
               ferry->nombre,
               ferry->num_vehiculos_abordo,
               ferry->capacidad_vehiculos,
               (int)(0.30 * ferry->capacidad_vehiculos));
        return 0;
    }
    
    return 1;
}
//=============================================================================
// FUNCIONES DE ESTADÍSTICAS
//=============================================================================
/**
 * @brief Genera un reporte detallado del viaje escrito en archivo.
 * @param in Puntero a archivo donde se escribirá el reporte.
 * @param sim Puntero a la simulación que contiene datos del ferry.
 * @param ferry_idx Índice del ferry a reportar.
 * @return void
 *
 * Descripción (español): Escribe en `in` un bloque con información del viaje
 * (número de vehículos, pasajeros, peso, ingresos y lista de placas transportadas).
 */
//=============================================================================
// NUEVA FUNCIÓN: generarReporteViaje
//=============================================================================
void generarReporteViajeArchivo(FILE *in, struct Simulacion *sim, int ferry_idx) {
    fprintf(in, "\n");
    fprintf(in, "══════════════════════════════════════════════════════════\n");
    fprintf(in, "🚢 VIAJE Nro. %d - Ferry: %s\n", 
           sim->ferrys[ferry_idx].total_viajes_realizados, 
           sim->ferrys[ferry_idx].nombre);
    fprintf(in, "──────────────────────────────────────────────────────────\n");
    fprintf(in, "  📊 Número de vehículos: %d\n", sim->ferrys[ferry_idx].num_vehiculos_abordo);
    fprintf(in, "  👥 Pasajeros: %d (mayores de 60: %d)\n", 
           sim->ferrys[ferry_idx].pasajeros_actuales, 
           sim->ferrys[ferry_idx].pasajeros_mayores_actuales);
    fprintf(in, "  ⚖️  Peso total: %.2f toneladas\n", sim->ferrys[ferry_idx].peso_actual_toneladas);
    fprintf(in, "  💰 Ingreso: %.2f BsF.\n", sim->ferrys[ferry_idx].total_ingresos);
    fprintf(in, "──────────────────────────────────────────────────────────\n");
    fprintf(in, "  📋 Vehículos transportados:\n");
    
    for (int i = 0; i < sim->ferrys[ferry_idx].num_vehiculos_abordo; i++) {
        char *tipo_str;
        switch(sim->ferrys[ferry_idx].vehiculos_a_bordo[i].tipo_vehiculo) {
            case 0: tipo_str = "liviano"; break;
            case 1: tipo_str = "rústico"; break;
            case 2: tipo_str = "van/microbus"; break;
            case 3: tipo_str = "carga"; break;
            case 4: tipo_str = "ambulancia"; break;
            case 5: tipo_str = "bomberos"; break;
            case 6: tipo_str = "policía"; break;
            default: tipo_str = "desconocido";
        }
        fprintf(in, "    %d. Placa: %s - Tipo: %s\n", 
               i + 1, 
               sim->ferrys[ferry_idx].vehiculos_a_bordo[i].placa, 
               tipo_str);
    }
    fprintf(in, "══════════════════════════════════════════════════════════\n\n");
}

/**
 * @brief Muestra por consola un reporte resumido del viaje del ferry.
 * @param sim Puntero a la estructura de simulación.
 * @param ferry_idx Índice del ferry cuya información se mostrará.
 * @return void
 *
 * Descripción (español): Imprime en stdout detalles del viaje similar al
 * reporte en archivo, incluyendo lista de vehículos transportados.
 */
void generarReporteViaje(struct Simulacion *sim, int ferry_idx) {
    printf("\n");
    printf("══════════════════════════════════════════════════════════\n");
    printf("🚢 VIAJE Nro. %d - Ferry: %s\n", 
           sim->ferrys[ferry_idx].total_viajes_realizados, 
           sim->ferrys[ferry_idx].nombre);
    printf("──────────────────────────────────────────────────────────\n");
    printf("  📊 Número de vehículos: %d\n", sim->ferrys[ferry_idx].num_vehiculos_abordo);
    printf("  👥 Pasajeros: %d (mayores de 60: %d)\n", 
           sim->ferrys[ferry_idx].pasajeros_actuales, 
           sim->ferrys[ferry_idx].pasajeros_mayores_actuales);
    printf("  ⚖️  Peso total: %.2f toneladas\n", sim->ferrys[ferry_idx].peso_actual_toneladas);
    printf("  💰 Ingreso: %.2f BsF.\n", sim->ferrys[ferry_idx].total_ingresos);
    printf("──────────────────────────────────────────────────────────\n");
    printf("  📋 Vehículos transportados:\n");
    
    for (int i = 0; i < sim->ferrys[ferry_idx].num_vehiculos_abordo; i++) {
        char *tipo_str;
        switch(sim->ferrys[ferry_idx].vehiculos_a_bordo[i].tipo_vehiculo) {
            case 1: tipo_str = "liviano"; break;
            case 2: tipo_str = "rústico"; break;
            case 3: tipo_str = "van/microbus"; break;
            case 4: tipo_str = "carga"; break;
            case 5: tipo_str = "ambulancia"; break;
            case 6: tipo_str = "bomberos"; break;
            case 7: tipo_str = "policía"; break;
        }
        printf("    %d. Placa: %s - Tipo: %s\n", 
               i + 1, 
               sim->ferrys[ferry_idx].vehiculos_a_bordo[i].placa, 
               tipo_str);
    }
    printf("══════════════════════════════════════════════════════════\n\n");
}

/**
 * @brief Actualiza métricas relacionadas con la cola de espera.
 * @param sim Puntero a la simulación con la cola de espera.
 * @return void
 *
 * Descripción (español): Si la cantidad actual en `cola_espera` supera el
 * máximo registrado, actualiza `max_vehiculos_espera` y la hora en que ocurrió.
 */
void actualizarEstadisticasEspera(struct Simulacion *sim) {
    int total_espera = sim->cola_espera.cantidad;
    
    if (total_espera > sim->max_vehiculos_espera) {
        sim->max_vehiculos_espera = total_espera;
        sim->hora_max_vehiculos_espera = minutosAHoraMilitar(sim->tiempo_actual_minutos);
    }
}



/**
 * @brief Escribe las estadísticas finales de la simulación en un archivo.
 * @param in Puntero al archivo donde se volcarán las estadísticas.
 * @param sim Puntero a la simulación con los datos acumulados.
 * @return void
 *
 * Descripción (español): Incluye totales de vehículos, pasajeros, ingresos,
 * vehículo más frecuente, máxima espera y resumen por ferry.
 */
void imprimirEstadisticasArchivo(FILE *in, struct Simulacion *sim) {
    fprintf(in, "\n");
    fprintf(in, "══════════════════════════════════════════════════════════\n");
    fprintf(in, "📊 ESTADÍSTICAS FINALES DEL DÍA\n");
    fprintf(in, "──────────────────────────────────────────────────────────\n");
    fprintf(in, "  🚗 Total vehículos transportados: %d\n", sim->total_vehiculos_dia);
    fprintf(in, "  👥 Total pasajeros transportados: %d\n", sim->total_pasajeros_dia);
    fprintf(in, "  💰 Total ingresos del día: %.2f BsF.\n", sim->total_ingresos_dia);
    fprintf(in, "  ⏳ Pasajeros no trasladados: %d\n", sim->total_pasajeros_no_trasladados);
    
    // Calcular el vehículo más frecuente
    char tipo_mas_frecuente[30];
    calcularVehiculoMasFrecuente(sim, tipo_mas_frecuente);
    fprintf(in, "  🚙 Vehículo más frecuente: %s\n", tipo_mas_frecuente);
    
    fprintf(in, "  📈 Máxima espera: %d vehículos a las %d\n", 
           sim->max_vehiculos_espera, sim->hora_max_vehiculos_espera);
    
    fprintf(in, "──────────────────────────────────────────────────────────\n");
    fprintf(in, "  📊 VIAJES POR FERRY:\n");
    for (int i = 0; i < MAX_FERRIES; i++) {
        fprintf(in, "    • %s: %d viajes, %d vehículos, %d pasajeros\n",
               sim->ferrys[i].nombre,
               sim->ferrys[i].total_viajes_realizados,
               sim->ferrys[i].total_vehiculos_transportados,
               sim->ferrys[i].total_pasajeros_transportados);
    }
    fprintf(in, "══════════════════════════════════════════════════════════\n\n");
}

/**
 * @brief Imprime por consola las estadísticas finales de la simulación.
 * @param sim Puntero a la estructura de simulación con los datos.
 * @return void
 *
 * Descripción (español): Muestra en stdout los mismos datos que `imprimirEstadisticasArchivo`.
 */
void imprimirEstadisticas(struct Simulacion *sim) {
    printf("\n");
    printf("══════════════════════════════════════════════════════════\n");
    printf("📊 ESTADÍSTICAS FINALES DEL DÍA\n");
    printf("──────────────────────────────────────────────────────────\n");
    printf("  🚗 Total vehículos transportados: %d\n", sim->total_vehiculos_dia);
    printf("  👥 Total pasajeros transportados: %d\n", sim->total_pasajeros_dia);
    printf("  💰 Total ingresos del día: %.2f BsF.\n", sim->total_ingresos_dia);
    printf("  ⏳ Pasajeros no trasladados: %d\n", sim->total_pasajeros_no_trasladados);
    
    // Calcular el vehículo más frecuente
    char tipo_mas_frecuente[30];
    calcularVehiculoMasFrecuente(sim, tipo_mas_frecuente);
    printf("  🚙 Vehículo más frecuente: %s\n", tipo_mas_frecuente);
    
    printf("  📈 Máxima espera: %d vehículos a las %d\n", 
           sim->max_vehiculos_espera, sim->hora_max_vehiculos_espera);
    
    printf("──────────────────────────────────────────────────────────\n");
    printf("  📊 VIAJES POR FERRY:\n");
    for (int i = 0; i < MAX_FERRIES; i++) {
        printf("    • %s: %d viajes, %d vehículos, %d pasajeros\n",
               sim->ferrys[i].nombre,
               sim->ferrys[i].total_viajes_realizados,
               sim->ferrys[i].total_vehiculos_transportados,
               sim->ferrys[i].total_pasajeros_transportados);
    }
    printf("══════════════════════════════════════════════════════════\n\n");
}

/**
 * @brief Calcula el tipo de vehículo más frecuente en toda la simulación.
 * @param sim Puntero a la simulación que contiene ferrys y colas.
 * @param tipo_str Buffer donde se copiará el nombre del tipo más frecuente.
 * @return void
 *
 * Descripción (español): Recorre ferrys, `cola_todos_vehiculos` y `cola_espera`
 * para contar apariciones por tipo y devuelve el tipo con mayor conteo.
 */
void calcularVehiculoMasFrecuente(struct Simulacion *sim, char *tipo_str) {
    // Contadores para cada tipo de vehículo (0-6)
    // 0=liviano, 1=rústico, 2=van, 3=carga, 4=ambulancia, 5=bomberos, 6=policía
    int contadores[7] = {0, 0, 0, 0, 0, 0, 0};
    
    // Recorrer todos los ferrys y sus vehículos a bordo
    for (int i = 0; i < MAX_FERRIES; i++) {
        for (int j = 0; j < sim->ferrys[i].num_vehiculos_abordo; j++) {
            int tipo = sim->ferrys[i].vehiculos_a_bordo[j].tipo_vehiculo;
            if (tipo >= 1 && tipo <= 7) {
                contadores[tipo - 1]++;
            }
        }
    }
    
    // Contar vehículos en cola_todos_vehiculos (vehículos activos no asignados)
    int indice_actual = sim->cola_todos_vehiculos.frente;
    for (int i = 0; i < sim->cola_todos_vehiculos.cantidad; i++) {
        struct VehiculoConEstado *v_estado = &sim->cola_todos_vehiculos.elementos[indice_actual];
        
        // Solo considerar vehículos activos
        if (v_estado->activo == 1) {
            int tipo = v_estado->datos.tipo_vehiculo;
            if (tipo >= 1 && tipo <= 7) {
                contadores[tipo - 1]++;
            }
        }
        
        indice_actual = (indice_actual + 1) % MAX_VEHICULOS_COLA;
    }
    
    // Contar vehículos en cola_espera
    indice_actual = sim->cola_espera.frente;
    for (int i = 0; i < sim->cola_espera.cantidad; i++) {
        struct VehiculoConEstado *v_estado = &sim->cola_espera.elementos[indice_actual];
        
        int tipo = v_estado->datos.tipo_vehiculo;
        if (tipo >= 1 && tipo <= 7) {
            contadores[tipo - 1]++;
        }
        
        indice_actual = (indice_actual + 1) % MAX_VEHICULOS_COLA;
    }
    
    // Encontrar el tipo con mayor frecuencia
    int max_count = 0;
    int max_tipo = 0;
    for (int t = 0; t < 7; t++) {
        if (contadores[t] > max_count) {
            max_count = contadores[t];
            max_tipo = t;
        }
    }
    
    // Convertir el tipo a string
    switch(max_tipo) {
        case 0: strcpy(tipo_str, "liviano"); break;
        case 1: strcpy(tipo_str, "rústico"); break;
        case 2: strcpy(tipo_str, "van/microbus"); break;
        case 3: strcpy(tipo_str, "carga"); break;
        case 4: strcpy(tipo_str, "ambulancia"); break;
        case 5: strcpy(tipo_str, "bomberos"); break;
        case 6: strcpy(tipo_str, "policía"); break;
    }
}

/**
 * @brief Calcula estadísticas finales relacionadas con pasajeros no trasladados.
 * @param sim Puntero a la simulación que contiene colas y ferrys.
 * @param ferry_idx Índice del ferry actualmente procesado (puede ser -1 si no aplica).
 * @return void
 *
 * Descripción (español): Suma los pasajeros que quedaron en `cola_todos_vehiculos`, en
 * `cola_espera` y en el ferry actual para obtener `total_pasajeros_no_trasladados`.
 */
void calcularEstadisticasFinales(struct Simulacion *sim, int ferry_idx) {
    int pasajeros_no_trasladados = 0;
    int vehiculos_no_trasladados = 0;
    
    printf("\n========== ESTADÍSTICAS FINALES ==========\n");
    
    // ===== 1. Vehículos en cola_todos_vehiculos (activos no asignados) =====
    int indice_actual = sim->cola_todos_vehiculos.frente;
    int count_cola_total = 0;
    
    for (int i = 0; i < sim->cola_todos_vehiculos.cantidad; i++) {
        struct VehiculoConEstado *v_estado = &sim->cola_todos_vehiculos.elementos[indice_actual];
        
        if (v_estado->activo == 1) {
            pasajeros_no_trasladados += v_estado->datos.total_pasajeros;
            vehiculos_no_trasladados++;
            count_cola_total++;
        }
        
        indice_actual = (indice_actual + 1) % MAX_VEHICULOS_COLA;
    }
    printf("📋 Vehículos en cola total (no asignados): %d\n", count_cola_total);
    
    // ===== 2. Vehículos en cola_espera =====
    indice_actual = sim->cola_espera.frente;
    int count_cola_espera = sim->cola_espera.cantidad;
    
    for (int i = 0; i < sim->cola_espera.cantidad; i++) {
        struct VehiculoConEstado *v_estado = &sim->cola_espera.elementos[indice_actual];
        pasajeros_no_trasladados += v_estado->datos.total_pasajeros;
        indice_actual = (indice_actual + 1) % MAX_VEHICULOS_COLA;
    }
    printf("⏳ Vehículos en cola de espera: %d\n", count_cola_espera);
    vehiculos_no_trasladados += count_cola_espera;
    
    // ===== 3. Vehículos en el ferry actual que no pudo zarpar =====
    int count_ferry = 0;
    if (ferry_idx >= 0 && ferry_idx < MAX_FERRIES) {
        struct Ferry *ferry = &sim->ferrys[ferry_idx];
        count_ferry = ferry->num_vehiculos_abordo;
        
        for (int i = 0; i < ferry->num_vehiculos_abordo; i++) {
            pasajeros_no_trasladados += ferry->vehiculos_a_bordo[i].total_pasajeros;
        }
        printf("🚢 Vehículos a bordo del ferry %s (sin zarpar): %d\n", 
               ferry->nombre, count_ferry);
        vehiculos_no_trasladados += count_ferry;
    }
    
    // ===== 4. Vehículos en otros ferrys que no están en viaje =====
    int count_otros_ferrys = 0;
    for (int i = 0; i < MAX_FERRIES; i++) {
        if (i != ferry_idx && sim->ferrys[i].num_vehiculos_abordo > 0) {
            struct Ferry *ferry = &sim->ferrys[i];
            count_otros_ferrys += ferry->num_vehiculos_abordo;
            
            for (int j = 0; j < ferry->num_vehiculos_abordo; j++) {
                pasajeros_no_trasladados += ferry->vehiculos_a_bordo[j].total_pasajeros;
            }
        }
    }
    if (count_otros_ferrys > 0) {
        printf("🚢 Vehículos en otros ferrys (sin viajar): %d\n", count_otros_ferrys);
        vehiculos_no_trasladados += count_otros_ferrys;
    }
    
    // ===== RESUMEN FINAL =====
    printf("\n📊 RESUMEN FINAL:\n");
    printf("   ✅ Total vehículos trasladados: %d\n", sim->total_vehiculos_dia);
    printf("   ❌ Total vehículos NO trasladados: %d\n", vehiculos_no_trasladados);
    printf("   📦 Total vehículos procesados: %d\n", 
           sim->total_vehiculos_dia + vehiculos_no_trasladados);
    
    printf("\n   👥 Total pasajeros trasladados: %d\n", sim->total_pasajeros_dia);
    printf("   👥 Total pasajeros NO trasladados: %d\n", pasajeros_no_trasladados);
    printf("   👥 Total pasajeros procesados: %d\n", 
           sim->total_pasajeros_dia + pasajeros_no_trasladados);
    
    // Guardar en la estructura (opcional, si quieres conservar el valor)
    sim->total_pasajeros_no_trasladados = pasajeros_no_trasladados;
}
//=============================================================================
// FUNCIÓN calcularIngresoVehiculo
//=============================================================================
/**
 * @brief Calcula el ingreso generado por un vehículo según su tipo y pasaje.
 * @param v La estructura `Vehiculo` con información de pasajeros y tipo.
 * @param tipoFerry Tipo de ferry (`TIPO_EXPRESS` o tradicional) que afecta tarifas.
 * @return float Ingreso económico generado por ese vehículo.
 *
 * Descripción (español): Aplica las tarifas establecidas para adultos, tercera edad
 * y tarifa por tipo de vehículo, diferenciando entre ferrys express y tradicionales.
 */
float calcularIngresoVehiculo(struct Vehiculo v, int tipoFerry) {
    float ingreso = 0.0;
    
    // Tarifas según tabla del enunciado
    if (v.tiene_pasajeros == 1) {
       if (tipoFerry == TIPO_EXPRESS) {
        // Ferry Express
        if (v.tipo_pasaje_adultos == 0) { // VIP
            ingreso += v.num_adultos * 1020.00;
            ingreso += v.num_tercera_edad * 520.00;
        } else { // Ejecutiva
            ingreso += v.num_adultos * 620.00;
            ingreso += v.num_tercera_edad * 320.00;
        }
    } else {
        // Ferry Tradicional
        if (v.tipo_pasaje_adultos == 0) { // Primera clase
            ingreso += v.num_adultos * 370.00;
            ingreso += v.num_tercera_edad * 190.50;
        } else { // Turista
            ingreso += v.num_adultos * 290.00;
            ingreso += v.num_tercera_edad * 150.50;
        }
    }
    }
    
    // Tarifa del vehículo según tipo
    switch(v.tipo_vehiculo) {
        case 1: // liviano
            ingreso += (tipoFerry == TIPO_EXPRESS) ? 1090.00 : 590.00;
            break;
        case 2: // rústico
            ingreso += (tipoFerry == TIPO_EXPRESS) ? 1310.00 : 710.00;
            break;
        case 3: // van/microbus
            ingreso += (tipoFerry == TIPO_EXPRESS) ? 1600.00 : 850.00;
            break;
        case 4: // carga
            ingreso += (tipoFerry == TIPO_EXPRESS) ? 2400.00 : 1200.00;
            break;
        case 5: case 6: case 7: // ambulancias, bomberos, policía
            // Asumimos que emergencias no pagan o pagan tarifa especial
            ingreso += 0; 
            break;
    }
    
    return ingreso;
}

//=============================================================================
// FUNCIÓN actualizarEstadisticasBajada
//=============================================================================
/**
 * @brief Actualiza contadores del ferry tras la bajada de un vehículo.
 * @param ferry Puntero al `Ferry` cuyos contadores se actualizarán.
 * @param bajado Puntero al `Vehiculo` que fue bajado del ferry.
 * @return void
 *
 * Descripción (español): Resta el peso y pasajeros del vehículo bajado de los
 * contadores actuales del ferry. La línea para restar ingresos está comentada
 * porque la política de cálculo puede variar según la implementación.
 */
void actualizarEstadisticasBajada(struct Ferry *ferry, struct Vehiculo *bajado) {
    ferry->peso_actual_toneladas -= bajado->peso_toneladas;
    ferry->pasajeros_actuales -= bajado->total_pasajeros;
    ferry->pasajeros_mayores_actuales -= bajado->num_tercera_edad;
    //ferry->total_ingresos -= calcularIngresoVehiculo(bajado, ferry->tipo);
}
