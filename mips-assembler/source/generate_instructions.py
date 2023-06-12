import json
import re

R_TYPE_INSTRUCTIONS = [
    ("add",     0x20, "rd, rs, rt"),
    ("addu",    0x21, "rd, rs, rt"),
    ("and",     0x24, "rd, rs, rt"),
    ("div",     0x1A, "rs, rt"),
    ("divu",    0x1B, " rs, rt"),
    ("jalr",    0x09, "rd, rs"),
    ("jr",      0x08, "rs"),
    ("mfhi",    0x10, "rd"),
    ("mflo",    0x12, "rd"),
    ("mthi",    0x11, "rs"),
    ("mtlo",    0x13, "rs"),
    ("mult",    0x18, " rs, rt"),
    ("multu",   0x19, "rs, rt"),
    ("nor",     0x27, "rd, rs, rt"),
    ("or",      0x25, "rd, rs, rt"),
    ("sll",     0x00, "rd, rt, sa"),
    ("sllv",    0x04, "rd, rt, rs"),
    ("slt",     0x2A, "rd, rs, rt"),
    ("sltu",    0x2B, "rd, rs, rt"),
    ("sra",     0x03, "rd, rt, sa"),
    ("srav",    0x07, "rd, rt, rs"),
    ("srl",     0x02, " rd, rt, sa"),
    ("srlv",    0x06, "rd, rt, rs"),
    ("sub",     0x22, "rd, rs, rt"),
    ("subu",    0x23, " rd, rs, rt"),
    ("syscall", 0x0C, ""),
    ("xor",     0x26, "rd, rs, rt"),
]

I_TYPE_INSTRUCTIONS = [
    ("addi",  0x08, "rt, rs, immediate"),
    ("addiu", 0x09, "rt, rs, immediate"),
    ("andi",  0x0C, "rt, rs, immediate"),
    ("beq",   0x04, "rs, rt, label"),
    ("bgez",  0x01, "rs, label"),
    ("bgtz",  0x07, "rs, label"),
    ("blez",  0x06, "rs, label"),
    ("bltz",  0x01, "rs, label"),
    ("bne",   0x05, "rs, rt, label"),
    ("lb",    0x20, "rt, immediate(rs)"),
    ("lbu",   0x24, "rt, immediate(rs)"),
    ("lh",    0x21, "rt, immediate(rs)"),
    ("lhu",   0x25, "rt, immediate(rs)"),
    ("lui",   0x0F, "rt, immediate"),
    ("lw",    0x23, "rt, immediate(rs)"),
    ("ori",   0x0D, "rt, rs, immediate"),
    ("sb",    0x28, " rt, immediate(rs)"),
    ("slti",  0x0A, "rt, rs, immediate"),
    ("sltiu", 0x0B, "rt, rs, immediate"),
    ("sh",    0x29, " rt, immediate(rs)"),
    ("sw",    0x2B, " rt, immediate(rs)"),
    ("xori",  0x0E, "rt, rs, immediate"),
    ("lwl",   0x22, "rt, immediate(rs)"),
    ("lwr",   0x26, "rt, immediate(rs)"),
    ("swl",   0x2A, "rt, immediate(rs)"),
    ("swr",   0x2E, "rt, immediate(rs)"),
]

J_TYPE_INSTRUCTIONS = [
    ("j",   0x02, "label"),
    ("jal", 0x03, "label"),
]
     

def key2regex(key: str):
    if key in ['rt', 'rs', 'rd', 'sa']:
        return r'\$\w+'
    if key == 'immediate':
        return r'-?\d+'
    if key == 'label':
        return r'\w+'

def parse_arg(arg: str):
    arg_ = arg.replace('(', ',').replace(')', '')
    args = arg_.strip().split(',')
    args = [s.strip() for s in args]
    arg = arg.strip().replace('(', '\(').replace(')', '\)')
    for key in ['rt', 'rs', 'rd', 'sa', 'immediate', 'label']:
        arg = arg.replace(key, f'({key2regex(key)})')
    return arg, args

def generate_instructions_to_json(save_path):
    data = {}
    for (inst, funct, arg) in R_TYPE_INSTRUCTIONS:
        regex, fields = parse_arg(arg)
        data[inst] = {
            'funct': funct,
            'type': 'R',
            'opcode': 0,
            'regex': regex,
            'fields': fields,
        }
    for (inst, opcode, arg) in I_TYPE_INSTRUCTIONS:
        regex, fields = parse_arg(arg)
        data[inst] = {
            'regex': regex,
            'fields': fields,
            'type': 'I',
            'opcode': opcode
        }
    for (inst, opcode, arg) in J_TYPE_INSTRUCTIONS:
        regex, fields = parse_arg(arg)
        data[inst] = {
            'regex': regex,
            'fields': fields,
            'type': 'J',
            'opcode': opcode
        }
    with open(save_path, 'w') as f:
        json.dump(data, f, indent=4)


if __name__ == '__main__':
    generate_instructions_to_json('instructions.json')