#!/usr/bin/env python3
import os
import random
import string
import argparse
import sys
from typing import List, Tuple, Optional, Dict
from datetime import datetime
from io import StringIO
from collections import Counter

# Generador para archivos proy1.in
random.seed(42)

def random_plate():
    chars = string.ascii_uppercase + string.digits
    return ''.join(random.choice(chars) for _ in range(6))

def gen_codigo(tipo_vehiculo):
    c1 = tipo_vehiculo
    c2 = random.randint(0, 2)
    c3 = random.randint(0, 1)
    return c1 * 100 + c2 * 10 + c3

def generate_vehicle_types_with_consecutive_bursts(n: int, 
                                                   max_consecutive: Optional[Dict[int, int]] = None) -> List[int]:
    """
    Genera una lista de tipos de vehículos respetando ráfagas consecutivas.
    Mantiene la distribución de pesos original.
    """
    types_list = []
    
    # Pesos para distribución normal (probabilidades originales)
    all_types = [1, 2, 3, 4, 5, 6, 7]
    weights = [0.15, 0.15, 0.15, 0.24, 0.10, 0.08, 0.13]  # Corresponden a tipos 1-7
    
    # Si no hay restricciones de consecutivos, usar distribución normal
    if not max_consecutive:
        for _ in range(n):
            tipo = random.choices(all_types, weights=weights, k=1)[0]
            types_list.append(tipo)
        return types_list
    
    # Con restricciones de ráfagas consecutivas
    consecutive_count = 0
    current_type = None
    
    for _ in range(n):
        # Determinar si podemos continuar con el tipo actual
        can_continue = False
        if current_type is not None:
            max_allowed = max_consecutive.get(current_type, 1)  # Por defecto 1 si no está especificado
            if consecutive_count < max_allowed:
                can_continue = True
        
        # Probabilidad de continuar la ráfaga (más alta si el tipo tiene mayor peso)
        continue_probability = 0.5  # 50% base de continuar
        
        if can_continue and random.random() < continue_probability:
            # Continuar con el mismo tipo
            new_type = current_type
            consecutive_count += 1
        else:
            # Elegir un nuevo tipo según los pesos originales
            # Asegurar que no violamos restricciones al cambiar
            available_types = []
            available_weights = []
            
            for tipo, weight in zip(all_types, weights):
                # Siempre podemos cambiar a cualquier tipo (el contador se reseteará)
                available_types.append(tipo)
                available_weights.append(weight)
            
            # Elegir nuevo tipo basado en pesos
            new_type = random.choices(available_types, weights=available_weights, k=1)[0]
            
            # Resetear contador
            if new_type != current_type:
                consecutive_count = 1
            else:
                consecutive_count += 1
            current_type = new_type
        
        types_list.append(current_type)
    
    return types_list

def generate_sorted_times_with_bursts(n: int, 
                                     one_per_minute: bool = False,
                                     max_vehicles_per_minute: Optional[Dict[int, int]] = None,
                                     max_consecutive_same_type: Optional[Dict[int, int]] = None,
                                     start_min: int = 6*60, 
                                     end_min: int = 22*60) -> Tuple[List[int], List[int]]:
    """
    Genera una lista de tiempos ordenados de menor a mayor.
    """
    if one_per_minute:
        total_minutes = end_min - start_min + 1
        if n > total_minutes:
            raise ValueError(f"No se pueden generar {n} vehículos con un vehículo por minuto")
        
        selected_minutes = sorted(random.sample(range(start_min, end_min + 1), n))
        times = [hh * 100 + mm for hh, mm in [(m // 60, m % 60) for m in selected_minutes]]
        return times, [None] * n
    
    # Generar los tipos de vehículos respetando ráfagas consecutivas
    types_list = generate_vehicle_types_with_consecutive_bursts(n, max_consecutive_same_type)
    
    # Generar distribución de vehículos por minuto (entre 1 y 10 por minuto)
    minutes_alloc = []
    remaining = n
    
    while remaining > 0:
        if remaining > 10:
            size = random.randint(1, 10)
        else:
            size = remaining
        minutes_alloc.append(size)
        remaining -= size
    
    # Mezclar para evitar que todas las ráfagas estén al principio
    random.shuffle(minutes_alloc)
    
    # Asignar tiempos (distribución uniforme en el horario)
    available_minutes = list(range(start_min, end_min + 1))
    
    # Si necesitamos más minutos de los disponibles, repetir algunos
    if len(minutes_alloc) > len(available_minutes):
        # Distribuir equitativamente
        result_minutes = []
        for i in range(len(minutes_alloc)):
            idx = i % len(available_minutes)
            result_minutes.append(available_minutes[idx])
        available_minutes = result_minutes
    else:
        # Seleccionar minutos aleatorios sin repetición
        available_minutes = random.sample(available_minutes, len(minutes_alloc))
        available_minutes.sort()  # Ordenar para que las horas sean cronológicas
    
    times = []
    vehicle_types = []
    type_index = 0
    
    for idx, count_in_minute in enumerate(minutes_alloc):
        minute = available_minutes[idx]
        hour_minute_format = (minute // 60) * 100 + (minute % 60)
        
        for _ in range(count_in_minute):
            if type_index < len(types_list):
                times.append(hour_minute_format)
                vehicle_types.append(types_list[type_index])
                type_index += 1
    
    return times, vehicle_types

def gen_vehicle_data(tipo_vehiculo: Optional[int] = None) -> Tuple[int, int, int, int, int, int, str]:
    """Genera los datos de un vehículo."""
    if tipo_vehiculo is None:
        # Distribución original de tipos
        r = random.random()
        if r < 0.10:
            tipo = 5  # ambulancia
        elif r < 0.18:
            tipo = 6  # bomberos
        elif r < 0.26:
            tipo = 7  # policia
        elif r < 0.50:
            tipo = 4  # carga
        elif r < 0.70:
            tipo = 3  # van
        elif r < 0.85:
            tipo = 2  # rustico
        else:
            tipo = 1  # liviano
    else:
        tipo = tipo_vehiculo
    
    codigo = gen_codigo(tipo)
    
    if tipo == 4:
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

def parse_burst_config(config_str: str) -> Dict[int, int]:
    """Parsea configuración de ráfagas."""
    if not config_str:
        return {}
    
    result = {}
    for part in config_str.split(','):
        if ':' not in part:
            raise ValueError(f"Formato inválido: {part}. Use 'tipo:max'")
        tipo_str, max_str = part.split(':', 1)
        try:
            tipo = int(tipo_str)
            max_val = int(max_str)
            if tipo not in range(1, 8):
                raise ValueError(f"Tipo inválido: {tipo}")
            result[tipo] = max_val
        except ValueError as e:
            raise ValueError(f"Error parseando '{part}': {e}")
    
    return result

def generate_file(path: str, n: int, order: Optional[List[int]] = None, 
                 one_per_minute: bool = False,
                 start_hour: int = 6, end_hour: int = 22,
                 max_per_minute_config: Optional[str] = None,
                 max_consecutive_config: Optional[str] = None,
                 save_stats: bool = True) -> None:
    """Genera un archivo de entrada."""
    os.makedirs(os.path.dirname(path), exist_ok=True)
    
    if order is None:
        order = random.sample([1, 2, 3], 3)
    
    max_per_minute = parse_burst_config(max_per_minute_config) if max_per_minute_config else {}
    max_consecutive = parse_burst_config(max_consecutive_config) if max_consecutive_config else {}
    
    start_min = start_hour * 60
    end_min = end_hour * 60
    
    times, assigned_types = generate_sorted_times_with_bursts(
        n, one_per_minute, max_per_minute, max_consecutive, start_min, end_min
    )
    
    stats_buffer = StringIO()
    original_stdout = sys.stdout
    
    try:
        sys.stdout = stats_buffer
        
        print(f"\n{'='*60}")
        print(f"ESTADÍSTICAS DE GENERACIÓN")
        print(f"{'='*60}")
        print(f"Archivo: {path}")
        print(f"Fecha: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")
        print(f"Total vehículos: {n}")
        print(f"Configuración: one_per_minute={one_per_minute}, horario={start_hour}:00-{end_hour}:00")
        if max_per_minute_config:
            print(f"Máximo por minuto: {max_per_minute_config}")
        if max_consecutive_config:
            print(f"Máximo consecutivo: {max_consecutive_config}")
        
        with open(path, 'w') as f:
            f.write(f"{order[0]} {order[1]} {order[2]}\n")
            
            stats = {1: 0, 2: 0, 3: 0, 4: 0, 5: 0, 6: 0, 7: 0}
            minute_groups = {}
            
            for i in range(n):
                tipo = assigned_types[i] if assigned_types[i] is not None else None
                tipo_ferry = random.choice([0, 1])
                codigo, adultos, tercera, tipo_adultos, tipo_tercera, peso, placa = gen_vehicle_data(tipo)
                hora = times[i]
                
                tipo_real = codigo // 100
                stats[tipo_real] = stats.get(tipo_real, 0) + 1
                
                if hora not in minute_groups:
                    minute_groups[hora] = []
                minute_groups[hora].append(tipo_real)
                
                f.write(f"{codigo} {adultos} {tercera} {tipo_adultos} {tipo_tercera} {peso} {hora} {placa} {tipo_ferry}\n")
            
            print(f"\nDistribución por tipo:")
            print(f"{'Tipo':<15} {'Cantidad':<10} {'Porcentaje':<10} {'Esperado':<10}")
            print(f"{'-'*50}")
            
            expected = {1: 15, 2: 15, 3: 15, 4: 24, 5: 10, 6: 8, 7: 13}
            for tipo in sorted(stats.keys()):
                tipo_nombre = {1: "Liviano", 2: "Rústico", 3: "Van/Micro", 
                              4: "Carga", 5: "Ambulancia", 6: "Bomberos", 
                              7: "Policía"}[tipo]
                if stats[tipo] > 0:
                    porcentaje = stats[tipo]/n*100
                    print(f"{tipo_nombre:<15} {stats[tipo]:<10} {porcentaje:.1f}%      {expected[tipo]}%")
            
            # Detectar ráfagas consecutivas
            print(f"\nRÁFAGAS CONSECUTIVAS DETECTADAS (más de 1 vehículo seguido):")
            print(f"{'Tipo':<15} {'Tamaño máx':<12} {'Veces':<10}")
            print(f"{'-'*40}")
            
            consecutive_bursts = {}
            if assigned_types and assigned_types[0] is not None:
                current_burst_type = assigned_types[0]
                current_burst_count = 1
                
                for i in range(1, len(assigned_types)):
                    current_type = assigned_types[i]
                    if current_type == current_burst_type:
                        current_burst_count += 1
                    else:
                        if current_burst_count > 1:
                            if current_burst_type not in consecutive_bursts:
                                consecutive_bursts[current_burst_type] = []
                            consecutive_bursts[current_burst_type].append(current_burst_count)
                        current_burst_type = current_type
                        current_burst_count = 1
                
                # Última ráfaga
                if current_burst_count > 1:
                    if current_burst_type not in consecutive_bursts:
                        consecutive_bursts[current_burst_type] = []
                    consecutive_bursts[current_burst_type].append(current_burst_count)
            
            if consecutive_bursts:
                for tipo in sorted(consecutive_bursts.keys()):
                    tipo_nombre = {1: "Liviano", 2: "Rústico", 3: "Van/Micro", 
                                  4: "Carga", 5: "Ambulancia", 6: "Bomberos", 
                                  7: "Policía"}[tipo]
                    max_burst = max(consecutive_bursts[tipo])
                    veces = len(consecutive_bursts[tipo])
                    print(f"{tipo_nombre:<15} {max_burst:<12} {veces:<10}")
            else:
                print("No se detectaron ráfagas consecutivas")
            
            # Mostrar ejemplo de las primeras ráfagas
            if assigned_types and len(assigned_types) > 20:
                print(f"\nPrimeros 20 vehículos (para verificar ráfagas):")
                tipos_nombres = {1: "L", 2: "R", 3: "V", 4: "C", 5: "A", 6: "B", 7: "P"}
                primeros = [tipos_nombres[t] for t in assigned_types[:20]]
                print(" ".join(primeros))
            
            print(f"{'='*60}\n")
        
        if save_stats:
            stats_path = os.path.splitext(path)[0] + '.stats.txt'
            stats_content = stats_buffer.getvalue()
            
            with open(stats_path, 'w') as sf:
                sf.write(stats_content)
            
            sys.stdout = original_stdout
            print(f"✓ Estadísticas guardadas en: {stats_path}")
            print(stats_buffer.getvalue())
        else:
            sys.stdout = original_stdout
            print(stats_buffer.getvalue())
        
    finally:
        sys.stdout = original_stdout
        stats_buffer.close()

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Generar archivos proy1.in para pruebas')
    parser.add_argument('--outputs', nargs='+', required=True)
    parser.add_argument('--one-per-minute', action='store_true')
    parser.add_argument('--start-hour', type=int, default=6)
    parser.add_argument('--end-hour', type=int, default=22)
    parser.add_argument('--max-per-minute', type=str, default=None)
    parser.add_argument('--max-consecutive', type=str, default=None)
    parser.add_argument('--no-stats', action='store_true')
    
    args = parser.parse_args()
    
    for pair in args.outputs:
        if '=' not in pair:
            print('Invalid pair:', pair)
            continue
        path, count = pair.split('=', 1)
        count = int(count)
        
        print(f'\nGenerando {path} con {count} vehículos...')
        
        try:
            generate_file(path, count, 
                         one_per_minute=args.one_per_minute,
                         start_hour=args.start_hour, 
                         end_hour=args.end_hour,
                         max_per_minute_config=args.max_per_minute,
                         max_consecutive_config=args.max_consecutive,
                         save_stats=not args.no_stats)
        except ValueError as e:
            print(f"Error generando {path}: {e}")
