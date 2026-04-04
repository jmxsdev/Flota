# Proyecto Flota - Documentación

Resumen
-------
Simulación de operación de ferrys y gestión de colas de vehículos para embarque.

**Estructuras principales**

- `struct Vehiculo`:
  - `int codigo` : Código compuesto (centenas-decenas-unidades) que define tipo, región y si lleva pasajeros.
  - `int region` : Región extraída del código.
  - `int tiene_pasajeros` : 1 si el pasajero es el chofer, 0 si no.
  - `int num_adultos` : Número de pasajeros adultos.
  - `int num_tercera_edad` : Número de pasajeros tercera edad.
  - `int tipo_pasaje_adultos` : 0=Primera/VIP, 1=Turista/Ejecutiva.
  - `int tipo_pasaje_tercera_edad` : Igual que anterior para tercera edad.
  - `int peso` : Peso (kg o toneladas según tipo).
  - `int hora_llegada` : Hora militar (ej. 830 para 08:30).
  - `char placa[MAX_LARGO_PLACA]` : Placa del vehículo.
  - `int tipo_ferry` : 0=Express, 1=Tradicional (preferencia de cola).
  - Campos calculados:
    - `int tipo_vehiculo` : 0..6 (liviano, rústico, van, carga, ambulancia, bomberos, policía).
    - `int es_emergencia` : 1 si es vehículo de emergencia.
    - `float peso_toneladas` : Peso en toneladas (convertido desde kg si aplica).
    - `int total_pasajeros` : Suma de pasajeros + conductor si aplica.

- `struct Ferry`:
  - `int id` : Identificador (1..3).
  - `char nombre[NOMBRE_FERRY_MAX]` : Nombre.
  - `int tipo` : `TIPO_EXPRESS` o `TIPO_TRADICIONAL`.
  - Capacidades: `capacidad_vehiculos`, `peso_maximo_toneladas`, `capacidad_clase1`, `capacidad_clase2`.
  - Tiempos: `tiempo_viaje` (minutos).
  - Estado y tiempos restantes: `estado`, `tiempo_restante_viaje`, `tiempo_restante_carga`.
  - Contenedores: `vehiculos_a_bordo[]`, `num_vehiculos_abordo`.
  - Estadísticas: `peso_actual_toneladas`, `pasajeros_actuales`, `total_viajes_realizados`, `total_ingresos`, etc.

- `struct ColaVehiculos`:
  - Arreglo circular `elementos[]`, índices `frente`, `final`, y `cantidad`.

- `struct VehiculoEspera` y `struct ColaEspera`:
  - `VehiculoEspera` contiene un `Vehiculo` + `tipo_cola_original`.
  - `ColaEspera` es una cola con orden especial (prioritarios primero, luego por hora).

- `struct Simulacion`:
  - `ferrys[]` : Array de `Ferry`.
  - Colas: `cola_express`, `cola_tradicional`, `cola_prioridad`, `cola_espera`.
  - `orden_carga[]` y `indice_orden_actual` para turno de carga.
  - Tiempos y estadísticas globales (`tiempo_actual_minutos`, `hora_inicio`, totals, etc.).

Funciones principales (descripción, parámetros y retornos)
-----------------------------------------------------

Nota: todos los prototipos están en `main.c` y la implementación en el mismo archivo.

- Inicialización
  - `void inicializarSimulacion(struct Simulacion *sim)`
    - Inicializa ferrys, colas y estadísticas.
    - Parámetros: `sim` (puntero a la simulación). Retorno: ninguno.

  - `void inicializarTodosFerrys(struct Ferry ferrys[MAX_FERRIES])`
    - Llama a `inicializarFerry` para cada ferry.

  - `void inicializarFerry(struct Ferry *ferry, int id)`
    - Configura parámetros fijos según `id` (nombre, tipo, capacidades, tiempos).
    - Parámetros: `ferry` (puntero), `id` (1..3).

  - `void inicializarCola(struct ColaVehiculos *cola)`
    - Inicializa índices y cantidad. Ningún retorno.

- Colas de vehículos
  - `int colaVacia(struct ColaVehiculos *cola)` / `int colaLlena(struct ColaVehiculos *cola)`
    - Devuelven 1 si vacía/llena, 0 si no.

  - `int encolar(struct ColaVehiculos *cola, struct Vehiculo v)`
    - Encola `v`. Retorna 1 si éxito, 0 si falla (cola llena).

  - `struct Vehiculo desencolar(struct ColaVehiculos *cola)`
    - Extrae y retorna el vehículo del frente; retorna vehículo con `codigo=0` si error.

  - `struct Vehiculo verFrente(struct ColaVehiculos *cola)`
    - Devuelve copia del frente sin extraer.

  - `int insertarEnColaEspera(struct ColaEspera *cola, struct Vehiculo v, int tipo_cola_original, struct Simulacion *sim, int ferry_idx)`
    - Inserta ordenadamente en `cola_espera` (prioritarios primero, luego por hora).
    - Retorna 1 si éxito, 0 si no (límite alcanzado o ferry sin capacidad).

- Procesamiento de archivos y datos
  - `int procesarArchivoCompleto(const char *nombre_archivo, struct ColaVehiculos *cola_express, struct ColaVehiculos *cola_tradicional, struct ColaVehiculos *cola_prioridad, int orden_carga[MAX_FERRIES], int *hora_inicio)`
    - Lee `nombre_archivo` (ej. `proy1.in`) y carga las colas.
    - Parámetros: nombre de archivo, punteros a las colas, `orden_carga` y puntero a `hora_inicio`.
    - Retorno: número de vehículos procesados o 0 en error.

  - `void procesarVehiculo(struct Vehiculo *v)`
    - Calcula campos derivados del vehículo (tipo, emergencia, peso en toneladas, total de pasajeros).

- Utilitarias de tiempo
  - `int horaMilitarAMinutos(int hora_militar)` / `int minutosAHoraMilitar(int minutos)`
    - Conversión entre formatos de tiempo. Retornan enteros.

- Validación
  - `int validarCod(int codigo)`
  - `int validarHoraMilitar(int hora)`
  - `int validarPasajeros(int num_adultos, int num_tercera_edad)`
  - `int validarTipoPasaje(int tpa, int tpt)`
  - `int validarPeso(int codigo, int peso)`
  - `int validarTipoFerry(int tipo_ferry)`
  - `int validarPlaca(const char *placa)`
  - `int validarVehiculo(struct Vehiculo *v)`
    - Cada función valida rangos y formatos específicos. Todas retornan 1 si válido, 0 si inválido.

- Simulación
  - `void iniciarSimulacion(FILE *in, struct Simulacion *sim)`
    - Bucle principal que gestiona carga, zarpes, actualiza tiempos y termina la simulación.
    - Parámetros: `in` (archivo salida), `sim` (estado). Sin retorno.

  - `int puedeViajar(struct Simulacion *sim, int ferry_idx, int hora_actual)`
    - Determina si un ferry puede zarpar según reglas (capacidad mínima, vehículos en colas, prioridad, peso).
    - Retorna 1 = puede zarpar, 0 = no.

  - `void iniciarViaje(FILE *in, struct Simulacion *sim, int ferry_idx)`
    - Cambia estado a viaje, actualiza estadísticas y genera reporte.

  - Carga y emergencia:
    - `void cargarVehiculoDesdeEspera(struct Simulacion *sim, int ferry_idx)`
    - `void cargarVehiculo(struct Ferry *ferry, struct Simulacion *sim)`
    - `void cargarVehiculoEmergencia(...)`, `void bajarVehiculosParaEmergencia(...)`, `void cargarVehiculoNormal(...)`
    - Manejan la prioridad de emergencias, búsqueda de vehículos para bajar si el ferry está lleno, y la lógica de carga.

- Estadísticas y reportes
  - `float calcularIngresoVehiculo(struct Vehiculo v, int tipoFerry)`
    - Calcula ingreso según tabla de tarifas y tipo de ferry.

  - `void generarReporteViajeArchivo(FILE *in, struct Simulacion *sim, int ferry_idx)`
  - `void generarReporteViaje(struct Simulacion *sim, int ferry_idx)`
  - `void imprimirEstadisticasArchivo(FILE *in, struct Simulacion *sim)`
  - `void imprimirEstadisticas(struct Simulacion *sim)`
  - `void calcularEstadisticasFinales(struct Simulacion *sim)`

Archivos relevantes
------------------
- `main.c` : Implementación completa de la simulación (entrada, colas, validaciones, simulación y reportes).
- `proy1.in` : Archivo de entrada de ejemplo con orden de carga + vehículos (en el raíz del proyecto y en `test/`).
- `checking.txt` : Registro de problemas y soluciones parciales durante el desarrollo.
- `test/*/proy1.in` : Archivos de prueba grandes instalados en cada subcarpeta `test/ejemplo1..3` para validación automatizada.

Formato de entrada y reglas de validación (compatibles con `main.c`)
-----------------------------------------------------------------

El archivo de entrada tiene este formato general:

1) Primera línea: tres enteros separados por espacio que indican el orden de carga de los ferrys, por ejemplo:

  2 3 1

  Deben ser los valores 1, 2 y 3 en alguna permutación sin repetidos.

2) Líneas siguientes: una por vehículo con los campos separados por espacio:

  codigo num_adultos num_tercera_edad tipo_pasaje_adultos tipo_pasaje_tercera_edad peso hora_llegada placa tipo_ferry

  - `codigo`: entero de 3 dígitos (centenas-decenas-unidades). Restricciones: centenas (c1) entre 1 y 6 (evitar 7 para compatibilidad con validaciones), decenas (c2) entre 0 y 2, unidades (c3) 0 o 1.
  - `num_adultos`: entero en [1..20].
  - `num_tercera_edad`: entero en [0..20] y `num_adultos + num_tercera_edad <= 20`.
  - `tipo_pasaje_adultos` y `tipo_pasaje_tercera_edad`: 0 = Primera/VIP, 1 = Turista/Ejecutiva.
  - `peso`: para vehículos livianos/rústicos/van -> kg en [500..5000]; para carga -> toneladas en [1..30]; para emergencias (ambulancia/bomberos/policía) -> kg en [1500..8000].
  - `hora_llegada`: hora militar (por ejemplo `830` para 08:30). Horas 0-23 y minutos 0-59.
  - `placa`: 6-7 caracteres alfanuméricos.
  - `tipo_ferry`: 0 = Express, 1 = Tradicional.

Validaciones implementadas en `main.c` (resumen):

- `validarCod(codigo)` : comprueba que centenas (c1) in [0..7] (se recomienda evitar 7), decenas en [0..2], unidades en {0,1}.
- `validarHoraMilitar(hora)` : horas 0..23, minutos 0..59.
- `validarPasajeros(num_adultos,num_tercera_edad)` : rangos y suma <= 20.
- `validarTipoPasaje(tpa,tpt)` : valores 0 o 1.
- `validarPeso(codigo,peso)` : rango dependen del tipo derivado del `codigo/100`.
- `validarTipoFerry(tipo_ferry)` : 0 o 1.
- `validarPlaca(placa)` : 6-7 caracteres alfanuméricos.

Archivos de prueba instalados
----------------------------

He generado y colocado archivos de prueba grandes en `test/` para facilitar pruebas de regresión y validación de todas las funciones:

- `test/ejemplo1/proy1.in`  (1200 vehículos)  — generado como `proy1_large.in` y copiado a `proy1.in`.
- `test/ejemplo2/proy1.in`  (1500 vehículos)  — generado como `proy1_large.in` y copiado a `proy1.in`.
- `test/ejemplo3/proy1.in`  (2000 vehículos)  — generado como `proy1_large.in` y copiado a `proy1.in`.

Estos archivos fueron creados para respetar las validaciones arriba descritas (códigos con centenas en 1..6, pesos dentro de rangos, horas progresivas, placas alfanuméricas). Si quieres variar la distribución (más emergencias, más camiones de carga, etc.), puedo regenerarlos con parámetros personalizados.

Ejecutar la simulación con un archivo de prueba
-----------------------------------------------

Compilar (si no está compilado):

```bash
gcc -o flota main.c
```

Ejecutar con el archivo `proy1.in` en la raíz:

```bash
./flota
```

O ejecutar apuntando a los archivos de `test/` (mover/copy `proy1.in` o cambiar `main.c` para usar ruta):

```bash
cp test/ejemplo1/proy1.in proy1.in
./flota
```


Problemas al desarrolla
-----------------------
Esta sección reúne entradas registradas en `checking.txt` y cambios relevantes en git que documentan la evolución y las trabas encontradas.

Entradas extraídas de `checking.txt`:

1) El orden de carga estaba predeterminado en 1,2,3 — se validó y ahora se lee desde `proy1.in`.

2) El tiempo máximo de espera y la hora de inicio: inicialmente asumidos por defecto; se ajustó para usar la hora del primer vehículo leído.

3) Faltaba emitir reportes en archivo de salida — se añadieron funciones `generarReporteViajeArchivo` e `imprimirEstadisticasArchivo`.

4) Ajuste de cola para emergencias — se diseñó y probó la lógica de bajar vehículos no-emergencia cuando llega una emergencia.

5) Pruebas de carga con muchos vehículos — pendiente generar `proy1_largo.in` (mencionado en commits).

6) Se redujo la verbosidad de mensajes de salida para evitar sobreinformación.

7) Idea: diseñar interfaz amigable — sugerencia para siguiente iteración (UI o CLI mejorada).

8) Documentación: considerar Doxygen para el código.

9) Duda sobre `fflush` y persistencia del archivo: se optó por abrir y escribir con `fclose` al final; `fflush` se dejó comentado con explicación en `checking.txt`.

10) Problema de colas y prioridad: originalmente las colas estaban separadas por tipo de pasaje; solución: unificar colas y agregar una `cola_espera` que mantiene orden y prioridad (implementado, ver commit bd4c5bb).

Cambios relevantes del historial de git (resumen):

- bd4c5bb (2026-04-03): Se modificó la estructura del manejo de colas a una cola total y una cola de esperas; refactorización de funciones para compatibilidad.
- 406f964 (2026-03-31): Refactorización para manejo de prioridad (emergencias) y funciones menos dependientes de apuntadores.
- 36681aa (2026-03-30): Identificación de problemas en la lógica de carga prioritaria y diseño de solución.
- 93852f6 (2026-03-27): Se agregaron funciones para imprimir reportes de viaje y estadísticas finales.
- b16c021 (2026-03-24): Creación inicial de `checking.txt` y estructura de pruebas en `test/`.

Cómo revisar el código fuente rápidamente
--------------------------------------
- El punto de entrada es `main.c` (ver [main.c](main.c)).
- Archivo de configuración de entrada de ejemplo: [proy1.in](proy1.in).
- Registro de desarrollo: [checking.txt](checking.txt).

Próximos pasos sugeridos
-----------------------
- Añadir más casos de prueba (archivo grande con >1000 vehículos) en `test/`.
- Añadir documentación Doxygen y comentarios en `main.c` para generar la documentación automática.
- Crear scripts de prueba y CI que ejecuten la simulación con entradas de `test/`.

Contacto y licencia
-------------------
Proyecto desarrollado por el autor del repositorio. Añadir información de licencia si se desea compartir.

---
Generado automáticamente: resumen y documentación inicial del código disponible en `main.c` y `checking.txt`.
