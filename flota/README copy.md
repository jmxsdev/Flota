# Proyecto Flota

## Portada (4.1)
- **Título del proyecto:** Simulación de operación de ferrys y gestión de colas de vehículos
- **Nombres y apellidos de los integrantes:** [Nombre Apellido] / [Nombre Apellido]
- **Cédulas de identidad:** [V-XXXXXXXX / V-YYYYYYYY]
- **Sección:** [Sección o grupo]
- **Fecha de entrega:** [DD/MM/AAAA]

---

## 4.2 Descripción del problema
- **Explicación general y objetivos:**
  Este proyecto implementa una simulación que modela la llegada y embarque de vehículos en una terminal de ferrys. El objetivo es gestionar colas, priorizar emergencias, respetar capacidades de vehículos y peso, y generar reportes y estadísticas de operación.
- **Contexto y propósito del software:**
  Sirve para evaluar políticas de carga, priorización y tiempos en un sistema de transporte por ferry. Es una herramienta académica para comprobar reglas de negocio y estudiar comportamiento bajo distintas cargas.

---

## 4.3 Análisis de la solución
- **Cómo se organizan y almacenan los datos:**
  - Se usan estructuras C (`struct`) para representar vehículos, ferrys y el estado global de la simulación.
  - Las colas se implementan con arreglos circulares estáticos (`ColaVehiculos`) conteniendo elementos `VehiculoConEstado` que tienen un flag `activo` y un `timestamp`.
  - La entrada se procesa desde un archivo `proy1.in` que contiene el orden de carga y las líneas de vehículos.
- **Justificación de decisiones:**
  - Arreglos estáticos y estructuras compactas facilitan el manejo sin dependencias externas y son fáciles de compilar en entornos limitados (práctica de curso).
  - Prioridad a emergencias implementada para garantizar tiempo de respuesta; políticas sencillas (bajar vehículos no-emergencia) reducen complejidad lógica.
  - Elección de cálculos en minutos (internos) y hora militar (formato HHMM en I/O) para facilitar comparaciones y orden temporal.

---

## 4.4 Estructuras de datos
- **Principales `structs` utilizados:**
  - `struct Vehiculo`:
    - Campos: `codigo`, `region`, `tiene_pasajeros`, `num_adultos`, `num_tercera_edad`, `tipo_pasaje_adultos`, `tipo_pasaje_tercera_edad`, `peso`, `hora_llegada`, `placa`, `tipo_ferry`.
    - Campos calculados: `tipo_vehiculo`, `es_emergencia`, `peso_toneladas`, `total_pasajeros`.
  - `struct Ferry`:
    - Campos: `id`, `nombre`, `tipo`, `capacidad_vehiculos`, `peso_maximo_toneladas`, `capacidad_clase1`, `capacidad_clase2`, `tiempo_viaje`, estado y estadísticas (vehículos a bordo, peso actual, ingresos, etc.).
  - `struct VehiculoConEstado`:
    - Envuelve `Vehiculo` con `activo` y `timestamp_asignacion`.
  - `struct ColaVehiculos`:
    - Arreglo circular de `VehiculoConEstado`, con `frente`, `final` y `cantidad`.
  - `struct Simulacion`:
    - Contiene `ferrys[]`, `cola_todos_vehiculos`, `cola_espera`, `orden_carga[]`, tiempos y estadísticas globales.
- **Arreglos u otras estructuras aplicadas:**
  - Arreglos estáticos para ferrys y buffers circulares para colas.
  - Tablas de contadores para estadísticas (p. ej. frecuencia de tipos).
- **Diagrama/representación (opcional):**
  - `Archivo proy1.in` -> parser -> `cola_todos_vehiculos` (arreglo circular)
  - `sim` mantiene `ferrys[]` y desde `cola_todos_vehiculos` se mueve a `cola_espera` según turno y capacidad -> `ferrys[].vehiculos_a_bordo` -> `viaje` -> reportes

---

## 4.5 Algoritmos implementados
- **Lectura y carga de entradas:**
  - Leer primera línea: orden de carga (tres enteros).
  - Por cada línea: parsear campos, validar con funciones (`validarCod`, `validarHoraMilitar`, `validarPeso`, `validarPlaca`, etc.), calcular campos derivados (`procesarVehiculo`) y encolar en `cola_todos_vehiculos`.

- **Bucle principal de simulación (resumen en pseudocódigo):**

  Inicio:
    inicializar simulación
    leer archivo -> llenar `cola_todos_vehiculos`
    tiempo_actual = hora del primer vehículo

  Mientras simulación no termine:
    ferry_actual = siguiente en `orden_carga`
    if ferry_actual.estado == CARGA:
      actualizarColaEsperaDesdeOrigenes(ferry_actual)
      if tiempo_carga_restante > 0: decrementar
      else: cargarVehiculoDesdeEspera(ferry_actual)
      if puedeViajar(ferry_actual): iniciarViaje(ferry_actual)
    else if ferry_actual.estado == ESPERA:
      poner en CARGA
    avanzar tiempo (1 minuto)
    actualizarEstadosFerrys()
    band = terminarSimulacion(...)

- **Carga y prioridad de emergencias:**
  - `actualizarColaEsperaDesdeOrigenes` mueve vehículos ya llegados y aptos hacia la `cola_espera` hasta llenar espacio previsto.
  - Vehículos con `es_emergencia` tienen prioridad; si no caben, `manejarEmergenciaPrioritaria` intenta liberar espacio (baja vehículos no-emergencia) y reinsertarlos luego.

- **Condiciones de zarpe (`puedeViajar`):**
  - Revisa mínimo de vehículos (30% de capacidad), límites de peso y pasajeros, y si la cola de espera puede proveer vehículos que todavía quepan; también vacía/reinserta la `cola_espera` según condiciones.

---

## 4.6 Pruebas realizadas
- **Scripts y cómo ejecutarlos:**
  - `run_test`: compila y ejecuta la simulación sobre cada `test/*/proy1.in`, guarda salidas en `test/results/`.

    ```bash
    chmod +x run_test
    ./run_test
    ```

  - `test/generate_test.py`: genera archivos `proy1.in` con parámetros (`--outputs`, `--one-per-minute`, `--start-hour`, `--end-hour`, `--max-consecutive`, `--no-stats`).

    Ejemplos:
    ```bash
    python3 test/generate_test.py --outputs test/archivo1.in=1000
    python3 test/generate_test.py --outputs test/archivo2.in=8600 --one-per-minute
    ```

  - `test/emergencias_test.py`: genera escenarios específicos para emergencias (escenarios 1..5).

- **Casos de prueba descritos:**
  - Escenarios de emergencias (ráfagas de ambulancias, mezclas, masivas).
  - Archivos grandes (varios miles de vehículos) para estrés y verificación de límites.
  - Casos de validación: entradas con códigos inválidos, horas o placas erróneas para verificar que el parser detecte errores.

- **Resumen del comportamiento observado:**
  - La lógica de prioridad responde correctamente: emergencias se cargan antes y, si es necesario, se bajan vehículos normales para hacer espacio.
  - Con entradas grandes el enfoque con buffers estáticos funciona, pero muestra penalización en tiempo por operaciones lineales repetidas (reinserciones y búsquedas).

---

## 4.7 Conclusiones
- **Reflexión sobre la experiencia de desarrollo:**
  - Implementar la simulación en C obliga a diseñar estructuras y manejo manual de colas; aporta aprendizaje sólido sobre memoria y estructuras de datos.
- **Dificultades encontradas y soluciones:**
  - Manejo de colas y reinserciones para emergencias exigió diseño cuidadoso para no perder el orden de llegada; se resolvió con `VehiculoConEstado.activo` y reinserciones controladas.
  - Se detectó un posible bug/`fall-through` en `procesarVehiculo` (`case 6` sin `break`) — recomendable corregir y añadir tests unitarios.
- **Posibles mejoras:**
  - Modularizar código, usar estructuras dinámicas, mejorar política para elegir qué vehículos bajar ante emergencias (minimizar impacto), añadir logging y pruebas automatizadas/CI.

---

Para instrucciones de ejecución y generación de tests vea `TESTS.md`.

