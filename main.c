#include <stdio.h>
#include <string.h>
#include <ctype.h>

// Constantes para límites (memoria estática)
#define MAX_FERRIES 3
#define MAX_VEHICULOS_POR_FERRY 20  // Capacidad máxima de vehículos en un ferry
#define TIEMPO_DE_CARGA_VEHICULO 3                                    // 
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
int esEmergencia(struct Ferry *ferry, struct Simulacion *sim);
int cabeVehiculo(struct Ferry *ferry, struct Simulacion *sim);
void cargarVehiculo(struct Ferry *ferry, struct Simulacion *sim);
void descargarVehiculo(struct Ferry *ferry, struct Simulacion *sim);
int hayVehiculosEnCola(struct Ferry *ferry, struct Simulacion *sim, int hora_carga);
int estaCargando(struct Ferry *ferry, int *hora_carga);
int puedeViajar(struct Ferry *ferry, struct Simulacion *sim, int hora_carga);
void iniciarViaje(FILE *in, struct Ferry *ferry, struct Simulacion *sim);
int terminarSimulacion(struct Ferry *ferry, struct Simulacion *sim, int hora_carga);
void actualizarEstadosFerrys(struct Simulacion *sim);
void cargarVehiculoEmergencia(struct Ferry *ferry, struct Simulacion *sim, 
                              struct Vehiculo v, int *hora_carga);
void bajarVehiculosParaEmergencia(struct Ferry *ferry, struct Simulacion *sim, 
                                  struct Vehiculo v, int *hora_carga);
void cargarVehiculoNormal(struct Ferry *ferry, struct Simulacion *sim, 
                          struct Vehiculo v, int *hora_carga);


// --- Funciones de Estadisticas ---
void generarReporteViaje(struct Ferry *ferry, struct Simulacion *sim);
void generarReporteViajeArchivo(FILE *in, struct Ferry *ferry, struct Simulacion *sim);
void actualizarEstadisticasEspera(struct Simulacion *sim);
void calcularEstadisticasFinales(struct Simulacion *sim);
void imprimirEstadisticasArchivo(FILE *in, struct Simulacion *sim);
void imprimirEstadisticas(struct Simulacion *sim);
float calcularIngresoVehiculo(struct Vehiculo *v, int tipoFerry);
void actualizarEstadisticasBajada(struct Ferry *ferry, struct Vehiculo *bajado);



// =============================================================================
//                               FUNCIÓN PRINCIPAL
// =============================================================================
int main() {
    
    // Inicializar simulación
    FILE *in = fopen("proy_1.out", "a");
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

    iniciarSimulacion(in,&simulacion);
    
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

// Función para procesar un vehículo leído del archivo
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
    if (v->tipo_vehiculo != 3) {  // No es carga
        v->peso_toneladas = (float)v->peso / 1000.0;  // kg a toneladas
    }

     // El conductor cuenta como adulto si no se especifica lo contrario
    v->total_pasajeros = v->num_adultos + v->num_tercera_edad + 1;
    
}

// -----------------------------------------------------------------------------
//                      Funciones Utilitarias de Tiempo
// -----------------------------------------------------------------------------

// Función para convertir hora militar (ej: 830) a minutos desde medianoche
int horaMilitarAMinutos(int hora_militar) {
    int horas = hora_militar / 100;
    int minutos = hora_militar % 100;
    return horas * 60 + minutos;
}

// Función para convertir minutos a hora militar
int minutosAHoraMilitar(int minutos) {
    int horas = minutos / 60;
    int mins = minutos % 60;
    return horas * 100 + mins;
}

// -----------------------------------------------------------------------------
//                      Funciones de Validación
// -----------------------------------------------------------------------------

// 2.2 FUNCIONES DE VALIDACIÓN COMPLETAS

//Validar rangos de COD
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

//=============================================================================
// FUNCIÓN PRINCIPAL DE SIMULACIÓN REFACTORIZADA
//=============================================================================
void iniciarSimulacion(FILE *in, struct Simulacion *sim) {
    int band = 1;
    
    while (band) {
        int hora_carga = minutosAHoraMilitar(sim->tiempo_actual_minutos);
        
        // Obtener el ferry que debe estar cargando según el orden
        struct Ferry *ferry_actual = &sim->ferrys[sim->indice_orden_actual];
        //Seleccionar la cola dependiendo del tipo de ferry
        struct ColaVehiculos *cola = (ferry_actual->tipo == TIPO_EXPRESS) ? 
                                  &sim->cola_express : &sim->cola_tradicional;
        //Seleccionar el ultimo vehiculo en cola
        struct Vehiculo v = verFrente(cola);
        
        // CASO 1: El ferry está en estado de CARGA
        if (ferry_actual->estado == ESTADO_CARGA) {
            //printf("Hay ferry cargando...\n");
            // Verificar si puede zarpar
            if (puedeViajar(ferry_actual, sim, hora_carga)) {
                iniciarViaje(in,ferry_actual, sim);
                printf("Intentando zarpar...\n");
                printf("🚢 Ferry %s ZARPÓ a las %d\n", ferry_actual->nombre, hora_carga);
            } 
            // Si no puede zarpar, intentar cargar vehículos
            else {
               // printf("Actualmente hay %d vehiculos en la cola tradicional", cola->cantidad);
              //  printf("Actualmente es %d horas y el vehiculo llego a las %d horas\n", hora_carga, v.hora_llegada);
                //Verificar si ha llegado un vehiculo a la cola en ese momento
                if (v.hora_llegada <= hora_carga) {
                   /* printf("El vehiculo deveria entrar\n");
                    printf("cantidad de Vehiculos a bordo: %d, cantidad de pasajeros %d, \
                        peso actual %f ton, capacidad de vehiculos: %d, capacidad peso %f, \
                        capacidad pasajeros %d", ferry_actual->num_vehiculos_abordo, 
                        ferry_actual->pasajeros_actuales, 
                        ferry_actual->peso_actual_toneladas, 
                        ferry_actual->capacidad_vehiculos, 
                        ferry_actual->peso_maximo_toneladas, 
                        ferry_actual->capacidad_clase1 + ferry_actual->capacidad_clase2);
                    */// Intentar carga normal primero
                    if (cabeVehiculo(ferry_actual, sim) && ferry_actual->tiempo_restante_carga == 0) {
                        cargarVehiculo(ferry_actual, sim);
                        printf("  ✅ Cargado vehículo en %s a las %d\n", 
                              ferry_actual->nombre, hora_carga);
                    }
                }

                 
            }
        } 
        // CASO 2: El ferry NO está en carga (VIAJE o ESPERA)
        else {
            // Si está en espera y es su turno, ponerlo en carga
            if (ferry_actual->estado == ESTADO_ESPERA) {
                ferry_actual->estado = ESTADO_CARGA;
                printf("🔵 Ferry %s iniciando CARGA a las %d\n", 
                       ferry_actual->nombre, hora_carga);
            }
            // Si está en viaje, no hacer nada (solo esperar)
        }
        
        // Avanzar el tiempo de simulación (3 minutos por operación)
        sim->tiempo_actual_minutos += 1;
        
        // Actualizar estados de ferrys en viaje
        actualizarEstadosFerrys(sim);
        
        // Actualizar estadísticas de espera
        actualizarEstadisticasEspera(sim);
        
        // Verificar si termina la simulación
        band = terminarSimulacion(ferry_actual, sim, hora_carga);
        //system("sleep 1");
        //printf("Esperando...\n");
    }
    imprimirEstadisticas(sim);
    // Al finalizar, calcular e imprimir estadísticas finales
    calcularEstadisticasFinales(sim);
    imprimirEstadisticasArchivo(in, sim);
}

int cabeVehiculo(struct Ferry *ferry, struct Simulacion *sim) {
    // Obtener la cola correcta
    struct ColaVehiculos *cola = (ferry->tipo == TIPO_EXPRESS) ? 
                                  &sim->cola_express : &sim->cola_tradicional;
    
    // Si no hay vehículos, no cabe nada
    if (colaVacia(cola)) return 0;
    
    struct Vehiculo v = verFrente(cola);
    
    if(v.es_emergencia) return 1;

    // Verificaciones de capacidad
    if (ferry->num_vehiculos_abordo >= ferry->capacidad_vehiculos) return 0;
    
    int capPasajeros = ferry->capacidad_clase1 + ferry->capacidad_clase2;
    if (ferry->pasajeros_actuales + v.total_pasajeros > capPasajeros) return 0;
    
    // Vehículos normales: verificar peso
    return (ferry->peso_actual_toneladas + v.peso_toneladas <= ferry->peso_maximo_toneladas);
}


//=============================================================================
// FUNCIÓN puedeViajar
//=============================================================================
int puedeViajar(struct Ferry *ferry, struct Simulacion *sim, int hora_carga) {
    int hayVehiculos = hayVehiculosEnCola(ferry, sim, hora_carga);
    int minimoVehiculos = (int)(0.30 * ferry->capacidad_vehiculos);
    //printf("minimo de vehiculos %d",minimoVehiculos );
    
    // Caso 1: Capacidad mínima alcanzada Y no hay vehículos en cola
    if (!hayVehiculos && ferry->num_vehiculos_abordo >= minimoVehiculos) {
        return 1;
    }
    
    // Caso 2: Capacidad máxima de vehículos alcanzada
    if (ferry->num_vehiculos_abordo >= ferry->capacidad_vehiculos) {
        return 1;
    }
    
    // Caso 3: Capacidad máxima de pasajeros alcanzada
    if (ferry->pasajeros_actuales >= (ferry->capacidad_clase1 + ferry->capacidad_clase2)) {
        return 1;
    }
    
    // Caso 4: Hay vehículos en cola pero no se pueden cargar por peso
    if (hayVehiculos) {
        struct Vehiculo v = (ferry->tipo == TIPO_EXPRESS) ? 
                            verFrente(&sim->cola_express) : 
                            verFrente(&sim->cola_tradicional);
        
        // Si el próximo vehículo no es emergencia Y excede el peso máximo
        if (!v.es_emergencia && 
            (ferry->peso_actual_toneladas + v.peso_toneladas > ferry->peso_maximo_toneladas)) {
            return 1;
        }
    }
    
    return 0;
}

//=============================================================================
// FUNCIÓN iniciarViaje
//=============================================================================
void iniciarViaje(FILE *in, struct Ferry *ferry, struct Simulacion *sim) {
    // 1. Generar reporte del viaje
    generarReporteViaje(ferry, sim);
    generarReporteViajeArchivo(in, ferry, sim);
    
    // 2. Actualizar estadísticas globales
    sim->total_vehiculos_dia += ferry->num_vehiculos_abordo;
    sim->total_pasajeros_dia += ferry->pasajeros_actuales;
    sim->total_ingresos_dia += ferry->total_ingresos;
    
    // 3. Actualizar estadísticas del ferry
    ferry->total_viajes_realizados++;
    ferry->total_vehiculos_transportados += ferry->num_vehiculos_abordo;
    ferry->total_pasajeros_transportados += ferry->pasajeros_actuales;
    ferry->total_pasajeros_mayores += ferry->pasajeros_mayores_actuales;
    
    // 4. Cambiar estado del ferry a VIAJE
    ferry->estado = ESTADO_VIAJE;
    ferry->tiempo_restante_viaje = ferry->tiempo_viaje;
    
    // 5. Avanzar al siguiente ferry en el orden de carga
    int indice_anterior = sim->indice_orden_actual;
    sim->indice_orden_actual = (sim->indice_orden_actual + 1) % MAX_FERRIES;
    
    // 6. Buscar el siguiente ferry disponible para ponerlo en ESPERA (no en carga aún)
    int id_siguiente = sim->orden_carga[sim->indice_orden_actual];
    for (int i = 0; i < MAX_FERRIES; i++) {
        if (sim->ferrys[i].id == id_siguiente) {
            // Solo si no está en viaje, ponerlo en espera
            if (sim->ferrys[i].estado != ESTADO_VIAJE) {
                sim->ferrys[i].estado = ESTADO_ESPERA;
            }
            break;
        }
    }
    
    // 7. Limpiar datos del viaje actual
    ferry->num_vehiculos_abordo = 0;
    ferry->peso_actual_toneladas = 0.0;
    ferry->pasajeros_actuales = 0;
    ferry->pasajeros_mayores_actuales = 0;
    ferry->total_ingresos = 0.0;
}

//=============================================================================
// NUEVA FUNCIÓN: actualizarEstadosFerrys
//=============================================================================
void actualizarEstadosFerrys(struct Simulacion *sim) {
    for (int i = 0; i < MAX_FERRIES; i++) {
        if (sim->ferrys[i].estado == ESTADO_VIAJE) {
            sim->ferrys[i].tiempo_restante_viaje -= 3; // Restamos 3 minutos por ciclo
            
            if (sim->ferrys[i].tiempo_restante_viaje <= 0) {
                sim->ferrys[i].estado = ESTADO_ESPERA;
                sim->ferrys[i].tiempo_restante_viaje = 0;
                printf("🟢 Ferry %s ha REGRESADO de viaje a las %d\n", 
                       sim->ferrys[i].nombre, 
                       minutosAHoraMilitar(sim->tiempo_actual_minutos));
            }
        }else if (sim->ferrys[i].estado == ESTADO_CARGA) {
            if (sim->ferrys[i].tiempo_restante_carga <= TIEMPO_DE_CARGA_VEHICULO && sim->ferrys[i].tiempo_restante_carga > 0) {
                sim->ferrys[i].tiempo_restante_carga--;    
            }
        }
    }
}



//=============================================================================
// NUEVA FUNCIÓN: hayVehiculosEnCola
//=============================================================================
int hayVehiculosEnCola(struct Ferry *ferry, struct Simulacion *sim, int hora_carga) {
    
    struct ColaVehiculos *cola = (ferry->tipo == TIPO_EXPRESS) ? 
                                  &sim->cola_express : &sim->cola_tradicional;


    return verFrente(cola).hora_llegada <= hora_carga;
}
//=============================================================================
// FUNCIÓN subirVehiculoABordo
//=============================================================================
void subirVehiculoABordo(struct Ferry *ferry, struct Vehiculo *v) {
    // Copiar el vehículo al ferry en la siguiente posición disponible
    ferry->vehiculos_a_bordo[ferry->num_vehiculos_abordo] = *v;
    ferry->num_vehiculos_abordo++;
    
    // Actualizar peso y pasajeros
    ferry->peso_actual_toneladas += v->peso_toneladas;
    ferry->pasajeros_actuales += v->total_pasajeros;
    ferry->pasajeros_mayores_actuales += v->num_tercera_edad;
    
    // Calcular y acumular ingreso
    ferry->total_ingresos += calcularIngresoVehiculo(v, ferry->tipo);
    
    // Opcional: imprimir confirmación
    // printf("    ✅ Vehículo %s subido a bordo. Total a bordo: %d\n", 
    //        v->placa, ferry->num_vehiculos_abordo);
}
//=============================================================================
// FUNCIÓN cargarVehiculo (ajustada para usar hora_carga)
//=============================================================================
void cargarVehiculo(struct Ferry *ferry, struct Simulacion *sim) {
    struct ColaVehiculos *cola = (ferry->tipo == TIPO_EXPRESS) ? 
                                  &sim->cola_express : &sim->cola_tradicional;
    
    if (colaVacia(cola)) return;
    
    struct Vehiculo v = verFrente(cola);
    int hora_carga = minutosAHoraMilitar(sim->tiempo_actual_minutos);
    
    if (v.es_emergencia) {
        cargarVehiculoEmergencia(ferry, sim, v, &hora_carga);
    } else {
        cargarVehiculoNormal(ferry, sim, v, &hora_carga);
    }

    ferry->tiempo_restante_carga = TIEMPO_DE_CARGA_VEHICULO;
}

//=============================================================================
// FUNCIÓN cargarVehiculoEmergencia REFACTORIZADA
//=============================================================================
void cargarVehiculoEmergencia(struct Ferry *ferry, struct Simulacion *sim, 
                              struct Vehiculo v, int *hora_carga) {
    struct ColaVehiculos *cola = (ferry->tipo == TIPO_EXPRESS) ? 
                                  &sim->cola_express : &sim->cola_tradicional;
    
    printf("*** VEHÍCULO DE EMERGENCIA PRIORITARIO: %s a las %d ***\n", 
           v.placa, *hora_carga);
    
    if (ferry->num_vehiculos_abordo < ferry->capacidad_vehiculos) {
        // Hay espacio directo
        subirVehiculoABordo(ferry, &v);
        desencolar(cola);  // Usando la función desencolar()
        printf("  Emergencia subido directamente. Vehículos a bordo: %d\n", 
               ferry->num_vehiculos_abordo);
    } else {
        printf("  ¡Ferry lleno! Buscando vehículo no-emergencia para bajar\n");
        bajarVehiculosParaEmergencia(ferry, sim, v, hora_carga);
    }
}

//=============================================================================
// FUNCIÓN bajarVehiculosParaEmergencia REFACTORIZADA
//=============================================================================
void bajarVehiculosParaEmergencia(struct Ferry *ferry, struct Simulacion *sim, 
                                  struct Vehiculo v, int *hora_carga) {
    struct ColaVehiculos *cola = (ferry->tipo == TIPO_EXPRESS) ? 
                                  &sim->cola_express : &sim->cola_tradicional;
    
    // Buscar vehículo no-emergencia para bajar (desde el final)
    for (int i = ferry->num_vehiculos_abordo - 1; i >= 0; i--) {
        if (!ferry->vehiculos_a_bordo[i].es_emergencia) {
            // Bajar este vehículo
            struct Vehiculo bajado = ferry->vehiculos_a_bordo[i];
            
            // Remover del ferry
            for (int j = i; j < ferry->num_vehiculos_abordo - 1; j++) {
                ferry->vehiculos_a_bordo[j] = ferry->vehiculos_a_bordo[j + 1];
            }
            ferry->num_vehiculos_abordo--;
            
            // Actualizar estadísticas
            actualizarEstadisticasBajada(ferry, &bajado);
            
            // Insertar al frente de la cola usando un enfoque que funciona con las funciones disponibles
            // Como no tenemos insertarAlFrente, necesitamos una solución alternativa
            
            // Crear una cola temporal para reordenar
            struct Vehiculo temp[MAX_VEHICULOS_COLA];
            int tam_temp = 0;
            
            // Guardar el vehículo bajado primero (irá al frente)
            temp[tam_temp++] = bajado;
            
            // Guardar todos los vehículos actuales de la cola (en orden)
            int idx = cola->frente;
            for (int k = 0; k < cola->cantidad; k++) {
                temp[tam_temp++] = cola->elementos[idx];
                idx = (idx + 1) % MAX_VEHICULOS_COLA;
            }
            
            // Vaciar la cola original (reiniciarla)
            cola->frente = 0;
            cola->final = -1;
            cola->cantidad = 0;
            
            // Volver a encolar todos en el nuevo orden (primero el bajado)
            for (int k = 0; k < tam_temp; k++) {
                encolar(cola, temp[k]);
            }
            
            printf("  Vehículo %s bajado y puesto al frente de la cola a las %d\n", 
                   bajado.placa, *hora_carga);
            
            // Subir el vehículo de emergencia
            subirVehiculoABordo(ferry, &v);
            desencolar(cola);  // Usando la función desencolar()
            printf("  Emergencia %s subido después de hacer espacio a las %d\n", 
                   v.placa, *hora_carga);
            return;
        }
    }
    
    printf("  ⚠️  Todos los vehículos a bordo son de emergencia. %s debe esperar\n", 
           v.placa);
}

//=============================================================================
// FUNCIÓN cargarVehiculoNormal REFACTORIZADA
//=============================================================================
void cargarVehiculoNormal(struct Ferry *ferry, struct Simulacion *sim, 
                          struct Vehiculo v, int *hora_carga) {
    struct ColaVehiculos *cola = (ferry->tipo == TIPO_EXPRESS) ? 
                                  &sim->cola_express : &sim->cola_tradicional;
    
    // Verificar si cabe (esta verificación ya debería haberse hecho en cabeVehiculo)
    float nuevoPeso = ferry->peso_actual_toneladas + v.peso_toneladas;
    int nuevosPasajeros = ferry->pasajeros_actuales + v.total_pasajeros;
    int capPasajeros = ferry->capacidad_clase1 + ferry->capacidad_clase2;
    
    if (ferry->num_vehiculos_abordo < ferry->capacidad_vehiculos &&
        nuevosPasajeros <= capPasajeros &&
        nuevoPeso <= ferry->peso_maximo_toneladas) {
        
        subirVehiculoABordo(ferry, &v);
        desencolar(cola);  // Usando la función desencolar()
        printf("  Vehículo normal %s cargado a las %d\n", v.placa, *hora_carga);
    } else {
        printf("  Vehículo normal %s no cabe (espera) a las %d\n", 
               v.placa, *hora_carga);
        // No se desencola, el vehículo permanece en la cola
    }
}

//=============================================================================
// FUNCIÓN terminarSimulacion (completada)
//=============================================================================
int terminarSimulacion(struct Ferry *ferry, struct Simulacion *sim, int hora_carga) {
    // Verificar condiciones de terminación:
    struct ColaVehiculos *cola = (ferry->tipo == TIPO_EXPRESS) ? 
                                  &sim->cola_express : &sim->cola_tradicional;
    // 1. Colas de acceso vacías
    int colasVacia = colaVacia(cola);
    
    // 2. No hay suficientes vehículos en ningún ferry para iniciar viaje

    int puedeZarpar = puedeViajar(ferry, sim, hora_carga);

    if (colasVacia && !puedeZarpar) {
        printf("\n✅ Simulación terminada: Colas vacías y sin ferrys activos\n");
        return 0; // Terminar simulación
    }
    
    return 1; // Continuar simulación
}

//=============================================================================
// FUNCIONES DE ESTADÍSTICAS
//=============================================================================
//=============================================================================
// NUEVA FUNCIÓN: generarReporteViaje
//=============================================================================
void generarReporteViajeArchivo(FILE *in, struct Ferry *ferry, struct Simulacion *sim) {
    fprintf(in,"\n");
    fprintf(in,"══════════════════════════════════════════════════════════\n");
    fprintf(in,"🚢 VIAJE Nro. %d - Ferry: %s\n", 
           ferry->total_viajes_realizados + 1, ferry->nombre);
    fprintf(in,"──────────────────────────────────────────────────────────\n");
    fprintf(in,"  📊 Número de vehículos: %d\n", ferry->num_vehiculos_abordo);
    fprintf(in,"  👥 Pasajeros: %d (mayores de 60: %d)\n", 
           ferry->pasajeros_actuales, ferry->pasajeros_mayores_actuales);
    fprintf(in,"  ⚖️  Peso total: %.2f toneladas\n", ferry->peso_actual_toneladas);
    fprintf(in,"  💰 Ingreso: %.2f BsF.\n", ferry->total_ingresos);
    fprintf(in,"──────────────────────────────────────────────────────────\n");
    fprintf(in,"  📋 Vehículos transportados:\n");
    
    for (int i = 0; i < ferry->num_vehiculos_abordo; i++) {
        char *tipo_str;
        switch(ferry->vehiculos_a_bordo[i].tipo_vehiculo) {
            case 0: tipo_str = "liviano"; break;
            case 1: tipo_str = "rústico"; break;
            case 2: tipo_str = "van/microbus"; break;
            case 3: tipo_str = "carga"; break;
            case 4: tipo_str = "ambulancia"; break;
            case 5: tipo_str = "bomberos"; break;
            case 6: tipo_str = "policía"; break;
            default: tipo_str = "desconocido";
        }
        fprintf(in,"    %d. ID: %s - Tipo: %s\n", 
               i + 1, ferry->vehiculos_a_bordo[i].placa, tipo_str);
    }
    fprintf(in,"══════════════════════════════════════════════════════════\n\n");
}
void generarReporteViaje(struct Ferry *ferry, struct Simulacion *sim) {
    printf("\n");
    printf("══════════════════════════════════════════════════════════\n");
    printf("🚢 VIAJE Nro. %d - Ferry: %s\n", 
           ferry->total_viajes_realizados + 1, ferry->nombre);
    printf("──────────────────────────────────────────────────────────\n");
    printf("  📊 Número de vehículos: %d\n", ferry->num_vehiculos_abordo);
    printf("  👥 Pasajeros: %d (mayores de 60: %d)\n", 
           ferry->pasajeros_actuales, ferry->pasajeros_mayores_actuales);
    printf("  ⚖️  Peso total: %.2f toneladas\n", ferry->peso_actual_toneladas);
    printf("  💰 Ingreso: %.2f BsF.\n", ferry->total_ingresos);
    printf("──────────────────────────────────────────────────────────\n");
    printf("  📋 Vehículos transportados:\n");
    
    for (int i = 0; i < ferry->num_vehiculos_abordo; i++) {
        char *tipo_str;
        switch(ferry->vehiculos_a_bordo[i].tipo_vehiculo) {
            case 0: tipo_str = "liviano"; break;
            case 1: tipo_str = "rústico"; break;
            case 2: tipo_str = "van/microbus"; break;
            case 3: tipo_str = "carga"; break;
            case 4: tipo_str = "ambulancia"; break;
            case 5: tipo_str = "bomberos"; break;
            case 6: tipo_str = "policía"; break;
            default: tipo_str = "desconocido";
        }
        printf("    %d. ID: %s - Tipo: %s\n", 
               i + 1, ferry->vehiculos_a_bordo[i].placa, tipo_str);
    }
    printf("══════════════════════════════════════════════════════════\n\n");
}

void actualizarEstadisticasEspera(struct Simulacion *sim) {
    int total_espera = sim->cola_express.cantidad + sim->cola_tradicional.cantidad;
    
    if (total_espera > sim->max_vehiculos_espera) {
        sim->max_vehiculos_espera = total_espera;
        sim->hora_max_vehiculos_espera = minutosAHoraMilitar(sim->tiempo_actual_minutos);
    }
}

void calcularEstadisticasFinales(struct Simulacion *sim) {
    // Calcular pasajeros no trasladados (los que quedaron en cola)
    int total_en_colas = sim->cola_express.cantidad + sim->cola_tradicional.cantidad;
    
    // Estimar pasajeros en cola (promedio simple)
    int pasajeros_en_cola = 0;
    
    // Contar pasajeros en cola Express
    for (int i = 0; i < sim->cola_express.cantidad; i++) {
        int idx = (sim->cola_express.frente + i) % MAX_VEHICULOS_COLA;
        pasajeros_en_cola += sim->cola_express.elementos[idx].total_pasajeros;
    }
    
    // Contar pasajeros en cola Tradicional
    for (int i = 0; i < sim->cola_tradicional.cantidad; i++) {
        int idx = (sim->cola_tradicional.frente + i) % MAX_VEHICULOS_COLA;
        pasajeros_en_cola += sim->cola_tradicional.elementos[idx].total_pasajeros;
    }
    
    sim->total_pasajeros_no_trasladados = pasajeros_en_cola;
}

void imprimirEstadisticasArchivo(FILE *in, struct Simulacion *sim) {
    fprintf(in,"\n");
    fprintf(in,"══════════════════════════════════════════════════════════\n");
    fprintf(in,"📊 ESTADÍSTICAS FINALES DEL DÍA\n");
    fprintf(in,"──────────────────────────────────────────────────────────\n");
    fprintf(in,"  🚗 Total vehículos transportados: %d\n", sim->total_vehiculos_dia);
    fprintf(in,"  👥 Total pasajeros transportados: %d\n", sim->total_pasajeros_dia);
    fprintf(in,"  💰 Total ingresos del día: %.2f BsF.\n", sim->total_ingresos_dia);
    fprintf(in,"  ⏳ Pasajeros no trasladados: %d\n", sim->total_pasajeros_no_trasladados);
    
    // Aquí deberías calcular el vehículo más frecuente
    fprintf(in,"  🚙 Vehículo más frecuente: %s\n", "liviano (por implementar)");
    
    fprintf(in,"  📈 Máxima espera: %d vehículos a las %d\n", 
           sim->max_vehiculos_espera, sim->hora_max_vehiculos_espera);
    
    fprintf(in,"──────────────────────────────────────────────────────────\n");
    fprintf(in,"  📊 VIAJES POR FERRY:\n");
    for (int i = 0; i < MAX_FERRIES; i++) {
        fprintf(in,"    • %s: %d viajes, %d vehículos, %d pasajeros\n",
               sim->ferrys[i].nombre,
               sim->ferrys[i].total_viajes_realizados,
               sim->ferrys[i].total_vehiculos_transportados,
               sim->ferrys[i].total_pasajeros_transportados);
    }
    fprintf(in,"══════════════════════════════════════════════════════════\n\n");
}
void imprimirEstadisticas(struct Simulacion *sim) {
    printf("\n");
    printf("══════════════════════════════════════════════════════════\n");
    printf("📊 ESTADÍSTICAS FINALES DEL DÍA\n");
    printf("──────────────────────────────────────────────────────────\n");
    printf("  🚗 Total vehículos transportados: %d\n", sim->total_vehiculos_dia);
    printf("  👥 Total pasajeros transportados: %d\n", sim->total_pasajeros_dia);
    printf("  💰 Total ingresos del día: %.2f BsF.\n", sim->total_ingresos_dia);
    printf("  ⏳ Pasajeros no trasladados: %d\n", sim->total_pasajeros_no_trasladados);
    
    // Aquí deberías calcular el vehículo más frecuente
    printf("  🚙 Vehículo más frecuente: %s\n", "liviano (por implementar)");
    
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

//=============================================================================
// FUNCIÓN calcularIngresoVehiculo (si no la tienes)
//=============================================================================
float calcularIngresoVehiculo(struct Vehiculo *v, int tipoFerry) {
    float ingreso = 0.0;
    
    // Tarifas según tabla del enunciado
    if (tipoFerry == TIPO_EXPRESS) {
        // Ferry Express
        if (v->tipo_pasaje_adultos == 0) { // VIP
            ingreso += v->num_adultos * 1020.00;
            ingreso += v->num_tercera_edad * 520.00;
        } else { // Ejecutiva
            ingreso += v->num_adultos * 620.00;
            ingreso += v->num_tercera_edad * 320.00;
        }
    } else {
        // Ferry Tradicional
        if (v->tipo_pasaje_adultos == 0) { // Primera clase
            ingreso += v->num_adultos * 370.00;
            ingreso += v->num_tercera_edad * 190.50;
        } else { // Turista
            ingreso += v->num_adultos * 290.00;
            ingreso += v->num_tercera_edad * 150.50;
        }
    }
    
    // Tarifa del vehículo según tipo
    switch(v->tipo_vehiculo) {
        case 0: // liviano
            ingreso += (tipoFerry == TIPO_EXPRESS) ? 1090.00 : 590.00;
            break;
        case 1: // rústico
            ingreso += (tipoFerry == TIPO_EXPRESS) ? 1310.00 : 710.00;
            break;
        case 2: // van/microbus
            ingreso += (tipoFerry == TIPO_EXPRESS) ? 1600.00 : 850.00;
            break;
        case 3: // carga
            ingreso += (tipoFerry == TIPO_EXPRESS) ? 2400.00 : 1200.00;
            break;
        case 4: case 5: case 6: // ambulancias, bomberos, policía
            // Asumimos que emergencias no pagan o pagan tarifa especial
            ingreso += 0; 
            break;
    }
    
    return ingreso;
}

//=============================================================================
// FUNCIÓN actualizarEstadisticasBajada (si no la tienes)
//=============================================================================
void actualizarEstadisticasBajada(struct Ferry *ferry, struct Vehiculo *bajado) {
    ferry->peso_actual_toneladas -= bajado->peso_toneladas;
    ferry->pasajeros_actuales -= bajado->total_pasajeros;
    ferry->pasajeros_mayores_actuales -= bajado->num_tercera_edad;
    ferry->total_ingresos -= calcularIngresoVehiculo(bajado, ferry->tipo);
}
