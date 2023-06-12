import json


def generate_register_map():
    registers = [
        'zero',
        'at',
        'v0', 'v1',
        'a0', 'a1', 'a2', 'a3',
        't0', 't1', 't2', 't3', 't4', 't5', 't6', 't7', 
        's0', 's1', 's2', 's3', 's4', 's5', 's6', 's7',
        't8', 't9',
        'k0', 'k1',
        'gp',
        'sp',
        'fp',
        'ra',
        # special registers
        'pc', 'HI', "LO"
    ]

    assert len(registers) == 35
    for i, reg in enumerate(registers):
        print(f'{{{i}, "${reg}"}}')


def generate_opcode_map():
    with open('instructions.json', 'r') as f:
        data = json.load(f)
    for name, detail in data.items():
        if detail['type'] != 'R':
            print(f'{{{detail["opcode"]}, "{name}"}}')


def generate_funct_map():
    with open('instructions.json', 'r') as f:
        data = json.load(f)
    for name, detail in data.items():
        if detail['type'] == 'R':
            print(f'{{{detail["funct"]}, "{name}"}}')


if __name__ == '__main__':
    # generate_register_map()
    # generate_opcode_map()
    generate_funct_map()