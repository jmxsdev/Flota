#!/usr/bin/env python3
"""
Generador de pruebas específicas para vehículos de emergencia.
Este script genera archivos de prueba para demostrar el comportamiento
de prioridad de ambulancias, policías y bomberos.
"""

import os
import random
import string
import argparse
from datetime import datetime
from typing import List, Tuple, Optional, Dict

random.seed(42)  # Para reproducibilidad

def random_plate():
    """Genera una placa aleatoria."""
    chars = string.ascii_uppercase + string.digits
    return ''.join(random.choice(chars) for _ in range(6))

def gen_codigo(tipo_vehiculo):
    """Genera código según tipo de vehículo."""
    c1 = tipo_vehiculo
    c2 = random.randint(0, 2)
    c3 = random.randint(0, 1)
    return c1 * 100 + c2 * 10 + c3

def gen_vehicle_data(tipo_vehiculo: int) -> Tuple[int, int, int, int, int, int, str]:
    """Genera datos específicos para un tipo de vehículo."""
    codigo = gen_codigo(tipo_vehiculo)
    
    if tipo_vehiculo == 4:  # Carga
        num_adultos = 0
        num_tercera = 0
        tipo_pasaje_adultos = 0
        tipo_pasaje_tercera = 0
        peso = random.randint(1, 12)
    else:
        num_adultos = random.randint(0, 4)
        num_tercera = random.randint(0, 2)
        tipo_pasaje_adultos = random.randint(0, 1)
        tipo_pasaje_tercera = random.randint(0, 1)
        peso = random.randint(500, 4000)
    
    placa = random_plate()
    
    return (codigo, num_adultos, num_tercera, tipo_pasaje_adultos, 
            tipo_pasaje_tercera, peso, placa)

def generate_emergency_burst_file(path: str, n_ambulancias: int, n_policias: int, 
                                  n_bomberos: int, n_otros: int,
                                  burst_size: int = 5,
                                  start_hour: int = 8, end_hour: int = 20) -> None:
    """
    Genera un archivo con ráfagas concentradas de vehículos de emergencia.
    
    Args:
        path: Ruta del archivo a generar
        n_ambulancias: Número de ambulancias
        n_policias: Número de policías
        n_bomberos: Número de bomberos
        n_otros: Número de otros vehículos
        burst_size: Tamaño de cada ráfaga (cuántos vehículos iguales seguidos)
        start_hour: Hora de inicio
        end_hour: Hora de fin
    """
    os.makedirs(os.path.dirname(path), exist_ok=True)
    
    total = n_ambulancias + n_policias + n_bomberos + n_otros
    
    # Crear lista de vehículos en orden específico para demostrar prioridad
    vehicle_list = []
    
    # Ráfagas de ambulancias (prioridad más alta)
    for i in range(0, n_ambulancias, burst_size):
        burst = min(burst_size, n_ambulancias - i)
        vehicle_list.extend([5] * burst)  # 5 = ambulancia
    
    # Ráfagas de bomberos (prioridad media-alta)
    for i in range(0, n_bomberos, burst_size):
        burst = min(burst_size, n_bomberos - i)
        vehicle_list.extend([6] * burst)  # 6 = bomberos
    
    # Ráfagas de policías (prioridad media)
    for i in range(0, n_policias, burst_size):
        burst = min(burst_size, n_policias - i)
        vehicle_list.extend([7] * burst)  # 7 = policía
    
    # Otros vehículos (prioridad baja)
    otros_tipos = [1, 2, 3, 4]  # liviano, rústico, van, carga
    for _ in range(n_otros):
        vehicle_list.append(random.choice(otros_tipos))
    
    # Mezclar para que no estén todos los emergencias al principio
    random.shuffle(vehicle_list)
    
    # Generar horas ordenadas
    start_min = start_hour * 60
    end_min = end_hour * 60
    total_minutes = end_min - start_min
    
    # Distribuir los vehículos en el tiempo
    times = []
    current_minute = start_min
    vehicles_per_minute = []
    
    remaining = total
    while remaining > 0:
        if remaining > 10:
            size = random.randint(1, 8)
        else:
            size = remaining
        vehicles_per_minute.append(size)
        remaining -= size
    
    # Generar horas para cada minuto
    for idx, count in enumerate(vehicles_per_minute):
        minute = start_min + (idx % total_minutes)
        if minute > end_min:
            minute = end_min
        hour_format = (minute // 60) * 100 + (minute % 60)
        for _ in range(count):
            times.append(hour_format)
    
    # Orden aleatorio de ferries
    order = random.sample([1, 2, 3], 3)
    
    # Escribir archivo
    with open(path, 'w') as f:
        f.write(f"{order[0]} {order[1]} {order[2]}\n")
        
        for i, tipo in enumerate(vehicle_list):
            tipo_ferry = random.choice([0, 1])
            codigo, adultos, tercera, tipo_adultos, tipo_tercera, peso, placa = gen_vehicle_data(tipo)
            hora = times[i] if i < len(times) else times[-1]
            f.write(f"{codigo} {adultos} {tercera} {tipo_adultos} {tipo_tercera} {peso} {hora} {placa} {tipo_ferry}\n")
    
    # Generar estadísticas
    stats_path = os.path.splitext(path)[0] + '.stats.txt'
    with open(stats_path, 'w') as sf:
        sf.write(f"{'='*60}\n")
        sf.write(f"ESTADÍSTICAS DE GENERACIÓN - PRUEBA DE EMERGENCIA\n")
        sf.write(f"{'='*60}\n")
        sf.write(f"Archivo: {path}\n")
        sf.write(f"Fecha: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}\n")
        sf.write(f"Total vehículos: {total}\n")
        sf.write(f"\nComposición:\n")
        sf.write(f"  Ambulancias: {n_ambulancias}\n")
        sf.write(f"  Policías: {n_policias}\n")
        sf.write(f"  Bomberos: {n_bomberos}\n")
        sf.write(f"  Otros: {n_otros}\n")
        sf.write(f"  Tamaño de ráfaga: {burst_size}\n")
        sf.write(f"  Horario: {start_hour}:00-{end_hour}:00\n")
        
        # Contar ráfagas consecutivas
        consecutive_bursts = {}
        current_type = vehicle_list[0]
        current_count = 1
        
        for tipo in vehicle_list[1:]:
            if tipo == current_type:
                current_count += 1
            else:
                if current_count > 1:
                    if current_type not in consecutive_bursts:
                        consecutive_bursts[current_type] = []
                    consecutive_bursts[current_type].append(current_count)
                current_type = tipo
                current_count = 1
        
        if current_count > 1:
            if current_type not in consecutive_bursts:
                consecutive_bursts[current_type] = []
            consecutive_bursts[current_type].append(current_count)
        
        sf.write(f"\nRÁFAGAS CONSECUTIVAS DETECTADAS:\n")
        tipo_nombre = {5: "Ambulancia", 6: "Bomberos", 7: "Policía"}
        for tipo in sorted(consecutive_bursts.keys()):
            if tipo in tipo_nombre:
                max_burst = max(consecutive_bursts[tipo])
                sf.write(f"  {tipo_nombre[tipo]}: máximo {max_burst} vehículos seguidos\n")
        
        sf.write(f"\n{'='*60}\n")
    
    # Mostrar resumen
    print(f"\n✓ Generado: {path}")
    print(f"  - {n_ambulancias} ambulancias")
    print(f"  - {n_policias} policías")
    print(f"  - {n_bomberos} bomberos")
    print(f"  - {n_otros} otros vehículos")
    print(f"✓ Estadísticas: {stats_path}")

def generate_test_scenario_1(base_dir: str = "test/emergencias") -> None:
    """Escenario 1: Muchas ambulancias seguidas (prueba de prioridad máxima)"""
    path = os.path.join(base_dir, "escenario1_muchas_ambulancias", "proy1.in")
    generate_emergency_burst_file(
        path=path,
        n_ambulancias=30,
        n_policias=5,
        n_bomberos=5,
        n_otros=10,
        burst_size=10,  # Ráfagas de 10 ambulancias seguidas
        start_hour=8,
        end_hour=12
    )

def generate_test_scenario_2(base_dir: str = "test/emergencias") -> None:
    """Escenario 2: Ráfagas mixtas de emergencias (prueba de prioridad relativa)"""
    path = os.path.join(base_dir, "escenario2_emergencias_mixtas", "proy1.in")
    generate_emergency_burst_file(
        path=path,
        n_ambulancias=15,
        n_policias=15,
        n_bomberos=15,
        n_otros=5,
        burst_size=5,
        start_hour=10,
        end_hour=16
    )

def generate_test_scenario_3(base_dir: str = "test/emergencias") -> None:
    """Escenario 3: Ráfaga masiva de ambulancias (estrés)"""
    path = os.path.join(base_dir, "escenario3_ambulancias_masivas", "proy1.in")
    generate_emergency_burst_file(
        path=path,
        n_ambulancias=40,
        n_policias=5,
        n_bomberos=5,
        n_otros=0,
        burst_size=20,  # Ráfagas de 20 ambulancias
        start_hour=9,
        end_hour=11
    )

def generate_test_scenario_4(base_dir: str = "test/emergencias") -> None:
    """Escenario 4: Prioridad intercalada (emergencias mezcladas con normales)"""
    path = os.path.join(base_dir, "escenario4_mezcla_prioridades", "proy1.in")
    
    # Crear lista personalizada para este escenario
    os.makedirs(os.path.dirname(path), exist_ok=True)
    
    # Secuencia específica: Ambulancias, luego normales, luego policías, luego bomberos
    vehicle_list = (
        [5] * 10 +  # 10 ambulancias
        [1, 2, 3, 4] * 5 +  # 20 vehículos normales
        [7] * 8 +   # 8 policías
        [6] * 7 +   # 7 bomberos
        [5] * 5     # 5 ambulancias más
    )
    
    random.shuffle(vehicle_list)
    
    # Generar horas
    start_min = 8 * 60
    end_min = 18 * 60
    total = len(vehicle_list)
    
    times = []
    current_minute = start_min
    while len(times) < total:
        count = min(random.randint(1, 5), total - len(times))
        for _ in range(count):
            times.append((current_minute // 60) * 100 + (current_minute % 60))
        current_minute += random.randint(1, 3)
        if current_minute > end_min:
            current_minute = start_min
    
    order = random.sample([1, 2, 3], 3)
    
    with open(path, 'w') as f:
        f.write(f"{order[0]} {order[1]} {order[2]}\n")
        for i, tipo in enumerate(vehicle_list):
            tipo_ferry = random.choice([0, 1])
            codigo, adultos, tercera, tipo_adultos, tipo_tercera, peso, placa = gen_vehicle_data(tipo)
            hora = times[i]
            f.write(f"{codigo} {adultos} {tercera} {tipo_adultos} {tipo_tercera} {peso} {hora} {placa} {tipo_ferry}\n")
    
    print(f"\n✓ Generado: {path}")
    print(f"  Secuencia personalizada con prioridades intercaladas")

def generate_test_scenario_5(base_dir: str = "test/emergencias") -> None:
    """Escenario 5: 50 vehículos demostrativos (todos los tipos)"""
    path = os.path.join(base_dir, "escenario5_demostrativo_50", "proy1.in")
    generate_emergency_burst_file(
        path=path,
        n_ambulancias=12,
        n_policias=10,
        n_bomberos=8,
        n_otros=20,
        burst_size=4,
        start_hour=8,
        end_hour=18
    )

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Generar pruebas demostrativas para vehículos de emergencia')
    parser.add_argument('--scenario', type=int, choices=[1, 2, 3, 4, 5, 0],
                       help='Escenario a generar (1-5) o 0 para todos')
    parser.add_argument('--output-dir', type=str, default='test/emergencias',
                       help='Directorio base para las pruebas (default: test/emergencias)')
    
    args = parser.parse_args()
    
    print(f"\n{'='*60}")
    print("GENERADOR DE PRUEBAS PARA VEHÍCULOS DE EMERGENCIA")
    print(f"{'='*60}")
    print(f"Directorio de salida: {args.output_dir}")
    
    if args.scenario == 0 or args.scenario is None:
        print("\nGenerando todos los escenarios...")
        generate_test_scenario_1(args.output_dir)
        generate_test_scenario_2(args.output_dir)
        generate_test_scenario_3(args.output_dir)
        generate_test_scenario_4(args.output_dir)
        generate_test_scenario_5(args.output_dir)
        print(f"\n✓ Todos los escenarios generados en {args.output_dir}/")
    elif args.scenario == 1:
        generate_test_scenario_1(args.output_dir)
    elif args.scenario == 2:
        generate_test_scenario_2(args.output_dir)
    elif args.scenario == 3:
        generate_test_scenario_3(args.output_dir)
    elif args.scenario == 4:
        generate_test_scenario_4(args.output_dir)
    elif args.scenario == 5:
        generate_test_scenario_5(args.output_dir)
    
    print(f"\n{'='*60}")
    print("Para ejecutar las pruebas, use:")
    print("  python main.py < archivo_de_prueba.in")
    print(f"{'='*60}\n")
