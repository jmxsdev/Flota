# Cómo ejecutar tests (Proyecto Flota)

Compilación:

    gcc -std=c11 -O2 -Wall -Wextra -o flota main.c

Ejecución con un archivo de prueba (método simple): copiar `proy1.in` a la raíz y ejecutar:

    ./flota

Alternativa (redirección):

    ./flota < test/emergencias/escenario5_demostrativo_50/proy1.in

Script automático `run_test`:

- Hacer ejecutable y ejecutar:

    chmod +x run_test
    ./run_test

- `run_test` compila, recorre `test/*/`, ejecuta la simulación por cada `proy1.in` encontrado y guarda resultados en `test/results/`.

Generar entradas con `test/generate_test.py`:

- Ejemplos:

    python3 test/generate_test.py --outputs test/archivo1.in=1000
    python3 test/generate_test.py --outputs test/archivo2.in=8600 --one-per-minute --start-hour=0 --end-hour=23

- Opciones útiles: `--one-per-minute`, `--start-hour`, `--end-hour`, `--max-per-minute`, `--max-consecutive`, `--no-stats`.

Generar escenarios de emergencia con `test/emergencias_test.py`:

- Generar todos los escenarios:

    python3 test/emergencias_test.py --scenario 0

- Generar un escenario concreto (p.ej. 5):

    python3 test/emergencias_test.py --scenario 5

- Cambiar directorio de salida:

    python3 test/emergencias_test.py --output-dir mis_pruebas --scenario 5

Salida y resultados:

- El programa genera `proy_1.out` en la raíz; `run_test` moverá esa salida a `test/results/<escenario>_proy_1.out` y guardará tiempos en `test/results/<escenario>_time.txt`.

Notas:

- Asegúrate de tener `python3` instalado para los scripts de generación.
- Concede permisos de ejecución a `run_test` si vas a usarlo.

