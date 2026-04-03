void actualizarColaEsperaDesdeOrigenes(struct Simulacion *sim, int ferry_idx) {
    // Calcular capacidad restante del ferry
    int capacidad_restante = sim->ferrys[ferry_idx].capacidad_vehiculos - sim->ferrys[ferry_idx].num_vehiculos_abordo;
    int espacio_disponible = capacidad_restante - sim->cola_espera.cantidad;
    
    /*if (espacio_disponible <= 0) {
        return;
    }*/
    
    int hora_actual = minutosAHoraMilitar(sim->tiempo_actual_minutos);
    
    // ==================== RECOLECTAR VEHÍCULOS ELEGIBLES ====================
    // Crear arrays temporales para almacenar vehículos que pueden ser considerados
    #define MAX_VEHICULOS 1000  // Ajustar según necesidades
    
    struct Vehiculo candidatos[MAX_VEHICULOS];
    int num_candidatos = 0;
    
    // 1. Recolectar vehículos de prioridad que ya han llegado
    struct Vehiculo temp_prioridad = verFrente(sim->cola_prioridad);
    int llego_prioridad = (temp_prioridad)
    // 2. Recolectar vehículos normales según tipo de ferry
    struct Vehiculo temp_normal;

    if (sim->ferrys[ferry_idx].tipo == TIPO_EXPRESS) {
        temp_normal = verFrente(&sim->cola_express);

    } else {
        temp_normal = verFrente(&sim->cola_tradicional);

    }
    
    
    
    // ==================== ORDENAR POR HORA DE LLEGADA ====================
    // Ordenar los candidatos por hora de llegada (menor a mayor)
    for (int i = 0; i < num_candidatos - 1; i++) {
        for (int j = i + 1; j < num_candidatos; j++) {
            if (candidatos[i].hora_llegada > candidatos[j].hora_llegada) {
                struct Vehiculo temp = candidatos[i];
                candidatos[i] = candidatos[j];
                candidatos[j] = temp;
            }
        }
    }
    
    // ==================== INSERTAR EN COLA DE ESPERA RESPETANDO ORDEN ====================
    int movidos = 0;
    int *vehiculos_a_mover = malloc(num_candidatos * sizeof(int));
    int num_a_mover = 0;
    
    // Determinar cuántos vehículos podemos mover sin exceder capacidad
    for (int i = 0; i < num_candidatos && movidos < espacio_disponible; i++) {
        vehiculos_a_mover[num_a_mover++] = i;
        movidos++;
    }
    
    // ==================== INSERTAR EN COLA DE ESPERA ====================
    // IMPORTANTE: Insertar manteniendo el orden cronológico en la cola de espera
    for (int i = 0; i < num_a_mover; i++) {
        int idx = vehiculos_a_mover[i];
        struct Vehiculo v = candidatos[idx];
        
        // Determinar tipo de origen para insertar en cola de espera
        int tipo_origen;
        if (v.es_emergencia == 1) {
            tipo_origen = 2;  // Prioridad
        } else if (sim->ferrys[ferry_idx].tipo == TIPO_EXPRESS) {
            tipo_origen = 0;  // Express
        } else {
            tipo_origen = 1;  // Tradicional
        }
        
        // Insertar en cola de espera (esta función debe mantener orden por hora_llegada)
        if (!insertarEnColaEsperaOrdenado(&sim->cola_espera, v, tipo_origen, sim, ferry_idx)) {
            // Si falla la inserción, liberar y retornar
            free(vehiculos_a_mover);
            return;
        }
    }
    
    // ==================== ELIMINAR VEHÍCULOS DE LAS COLAS ORIGINALES ====================
    // Ahora sí, eliminar de las colas originales los vehículos que movimos
    
    // Para prioridad
    int prioridad_eliminados = 0;
    for (int i = 0; i < num_a_mover; i++) {
        struct Vehiculo v = candidatos[vehiculos_a_mover[i]];
        if (v.es_emergencia == 1) {
            // Eliminar de cola de prioridad (asumiendo que el primero es el que tiene menor hora_llegada)
            if (!colaVacia(&sim->cola_prioridad)) {
                struct Vehiculo frente = verFrente(&sim->cola_prioridad);
                if (frente.id == v.id) {
                    desencolar(&sim->cola_prioridad);
                    prioridad_eliminados++;
                }
            }
        }
    }
    
    // Para normales
    int normales_eliminados = 0;
    for (int i = 0; i < num_a_mover; i++) {
        struct Vehiculo v = candidatos[vehiculos_a_mover[i]];
        if (v.es_emergencia == 0) {
            if (sim->ferrys[ferry_idx].tipo == TIPO_EXPRESS) {
                if (!colaVacia(&sim->cola_express)) {
                    struct Vehiculo frente = verFrente(&sim->cola_express);
                    if (frente.id == v.id) {
                        desencolar(&sim->cola_express);
                        normales_eliminados++;
                    }
                }
            } else {
                if (!colaVacia(&sim->cola_tradicional)) {
                    struct Vehiculo frente = verFrente(&sim->cola_tradicional);
                    if (frente.id == v.id) {
                        desencolar(&sim->cola_tradicional);
                        normales_eliminados++;
                    }
                }
            }
        }
    }
    
    // ==================== SACRIFICAR VEHÍCULOS NORMALES SI EXCEDE CAPACIDAD ====================
    // Verificar si después de mover, la cola de espera excede la capacidad del ferry
    int capacidad_total = sim->ferrys[ferry_idx].capacidad_vehiculos;
    int abordo_actual = sim->ferrys[ferry_idx].num_vehiculos_abordo;
    int en_espera = sim->cola_espera.cantidad;
    
    if (abordo_actual + en_espera > capacidad_total) {
        // Hay que sacrificar vehículos normales de la cola de espera
        int exceso = (abordo_actual + en_espera) - capacidad_total;
        
        // Sacrificar los últimos vehículos normales insertados (los más recientes)
        sacrificarVehiculosNormales(&sim->cola_espera, exceso, sim, ferry_idx);
    }
    
    if (movidos > 0) {
        printf("  🔄 Movidos %d vehículos a cola de espera (Prioridad: %d, Normales: %d)\n", 
               movidos, prioridad_eliminados, normales_eliminados);
    }
    
    free(vehiculos_a_mover);
}

// Función auxiliar para insertar en cola de espera manteniendo orden
bool insertarEnColaEsperaOrdenado(struct ColaCircular *cola, struct Vehiculo v, int tipo_origen, 
                                   struct Simulacion *sim, int ferry_idx) {
    // Esta función debe insertar el vehículo en la posición correcta
    // según su hora_llegada para mantener la cola ordenada
    
    if (colaVacia(cola)) {
        return encolar(cola, v, tipo_origen);
    }
    
    // Crear una cola temporal para reordenar
    struct ColaCircular temp = *cola;
    struct ColaCircular nueva_cola;
    inicializarCola(&nueva_cola);
    
    bool insertado = false;
    
    // Recorrer la cola original e insertar en nueva_cola en orden
    while (!colaVacia(&temp)) {
        struct Vehiculo actual = verFrente(&temp);
        int tipo_actual = obtenerTipoOrigen(&temp); // Necesitas esta función
        
        if (!insertado && v.hora_llegada <= actual.hora_llegada) {
            encolar(&nueva_cola, v, tipo_origen);
            insertado = true;
        }
        
        encolar(&nueva_cola, actual, tipo_actual);
        desencolar(&temp);
    }
    
    if (!insertado) {
        encolar(&nueva_cola, v, tipo_origen);
    }
    
    // Reemplazar la cola original
    *cola = nueva_cola;
    return true;
}

// Función para sacrificar vehículos normales
void sacrificarVehiculosNormales(struct ColaCircular *cola_espera, int cantidad, 
                                  struct Simulacion *sim, int ferry_idx) {
    struct ColaCircular temp = *cola_espera;
    struct ColaCircular nueva_cola;
    inicializarCola(&nueva_cola);
    
    int sacrificados = 0;
    int num_vehiculos = cola_espera->cantidad;
    
    // Recorrer la cola y sacrificar vehículos normales desde el final
    // Para simplificar, creamos un array temporal
    struct Vehiculo vehiculos[MAX_VEHICULOS];
    int tipos[MAX_VEHICULOS];
    int count = 0;
    
    // Extraer todos los vehículos
    while (!colaVacia(&temp)) {
        vehiculos[count] = verFrente(&temp);
        tipos[count] = obtenerTipoOrigen(&temp);
        count++;
        desencolar(&temp);
    }
    
    // Sacrificar desde el final hacia el principio
    for (int i = count - 1; i >= 0 && sacrificados < cantidad; i--) {
        if (tipos[i] != 2) {  // No sacrificar prioritarios
            // Devolver a su cola original
            devolverAColaOriginal(sim, vehiculos[i], tipos[i], ferry_idx);
            sacrificados++;
            // Marcar como sacrificado (no se insertará en nueva_cola)
            tipos[i] = -1;
        }
    }
    
    // Reconstruir cola de espera con los vehículos no sacrificados
    for (int i = 0; i < count; i++) {
        if (tipos[i] != -1) {
            encolar(&nueva_cola, vehiculos[i], tipos[i]);
        }
    }
    
    *cola_espera = nueva_cola;
    
    if (sacrificados > 0) {
        printf("  ⚠️ Sacrificados %d vehículos normales por exceso de capacidad\n", sacrificados);
    }
}

void devolverAColaOriginal(struct Simulacion *sim, struct Vehiculo v, int tipo_origen, int ferry_idx) {
    switch(tipo_origen) {
        case 0:  // Express
            encolar(&sim->cola_express, v, 0);
            break;
        case 1:  // Tradicional
            encolar(&sim->cola_tradicional, v, 1);
            break;
        case 2:  // Prioridad (no debería sacrificarse, pero por si acaso)
            encolar(&sim->cola_prioridad, v, 2);
            break;
    }
}
