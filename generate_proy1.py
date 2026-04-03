#!/usr/bin/env python3
import os
import random
import string
import argparse

# Generador para archivos proy1.in
# Formato de cada línea:
# codigo num_adultos num_tercera_edad tipo_pasaje_adultos tipo_pasaje_tercera_edad peso hora_llegada placa tipo_ferry

random.seed(42)

def random_plate():
    chars = string.ascii_uppercase + string.digits
    return ''.join(random.choice(chars) for _ in range(6))

def gen_codigo(tipo_vehiculo):
    # tipo_vehiculo: 1..7 mapping where 4==carga
    c1 = tipo_vehiculo
    c2 = random.randint(0,2)  # region
    c3 = random.randint(0,1)  # tiene pasajeros flag
    return c1*100 + c2*10 + c3

def gen_hora(start_min=6*60, end_min=22*60):
    m = random.randint(start_min, end_min)
    hh = m // 60
    mm = m % 60
    return hh*100 + mm

def gen_line(tipo_ferry_override=None):
    # Decide tipo de vehiculo con probabilidades razonables
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
        tipo = 3  # van/microbus
    elif r < 0.85:
        tipo = 2  # rustico
    else:
        tipo = 1  # liviano

    codigo = gen_codigo(tipo)

    # pasajeros
    if tipo == 4:
        # carga, normalmente no pasajeros
        num_adultos = 0
        num_tercera = 0
        tipo_pasaje_adultos = 0
        tipo_pasaje_tercera = 0
        peso = random.randint(1, 20)  # toneladas (integer)
    else:
        # pasajeros present
        # tiene_pasajeros flag encoded in codigo low digit but also we set passenger counts
        num_adultos = random.randint(0, 4)
        num_tercera = random.randint(0, 2)
        tipo_pasaje_adultos = random.randint(0,1)
        tipo_pasaje_tercera = random.randint(0,1)
        peso = random.randint(500, 4000)  # kg

    hora = gen_hora()
    placa = random_plate()
    if tipo_ferry_override is None:
        tipo_ferry = random.choice([0,1])
    else:
        tipo_ferry = tipo_ferry_override

    return f"{codigo} {num_adultos} {num_tercera} {tipo_pasaje_adultos} {tipo_pasaje_tercera} {peso} {hora} {placa} {tipo_ferry}\n"


def generate_file(path, n, order=None):
    os.makedirs(os.path.dirname(path), exist_ok=True)
    if order is None:
        # random permutation of 1 2 3
        order = random.sample([1,2,3], 3)
    with open(path, 'w') as f:
        f.write(f"{order[0]} {order[1]} {order[2]}\n")
        for i in range(n):
            f.write(gen_line())


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Generar archivos proy1.in para pruebas')
    parser.add_argument('--outputs', nargs='+', help='pairs path=count e.g. test/ejemplo1/proy1.in=1200', required=True)
    args = parser.parse_args()

    for pair in args.outputs:
        if '=' not in pair:
            print('Invalid pair:', pair)
            continue
        path, count = pair.split('=',1)
        count = int(count)
        print('Generating', path, 'with', count, 'entries')
        generate_file(path, count)
