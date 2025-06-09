#!/usr/bin/python3
import random
import string
import sys

def test_prob(prob = 1.0):
    return random.random() > prob

def sample(list):
    return random.sample(list, 1)[0]

Characters = list(string.ascii_lowercase + string.ascii_uppercase) + list(str(i) for i in range(10))

def generate_symbol(symbol_table):
    if len(symbol_table) > 0 and not test_prob(SemanticErrorProb):
        return sample(list(symbol_table))
    else:
        first_letter = sample(string.ascii_lowercase)
        symbol_length = random.randint(0, 15) if test_prob(SyntaxErrorProb) else random.randint(0, 50)
        return first_letter + ''.join(sample(Characters) for _ in range(symbol_length))

Types = ['M', 'A', 'R', 'I', 'E']

def generate_instr_type():
    return sample(Types) if test_prob(SyntaxErrorProb) else sample(string.ascii_uppercase)

def generate_instr_code(instr_type, module_size, use_list):
    op_code = str(random.randint(0, 9)) if test_prob(SyntaxErrorProb) else ''
    if instr_type == 'R':
        if module_size > 0 and test_prob(SemanticErrorProb):
            relative_addr = random.randint(0, module_size - 1)
        else:
            relative_addr = random.randint(0, 999)
        return op_code + '{0:03}'.format(relative_addr)
    elif instr_type == 'E':
        if len(use_list) > 0 and test_prob(SemanticErrorProb):
            use_index = random.randint(0, len(use_list) - 1)
        else:
            use_index = random.randint(0, 30)
        return op_code + '{0:03}'.format(use_index)
    elif instr_type == 'M':
        if test_prob(SemanticErrorProb):
            mod_index = random.randint(0,NumberOfModules)
        else:
            mod_index = random.randint(0,100)
        return op_code + '{0:03}'.format(mod_index)
    else:
        return op_code + str(random.randint(0, 512) if test_prob(SemanticErrorProb) else random.randint(0, 10000))

Whitespaces = [' ', '\t', '\n']

def write_token(file, token, max_number_of_whitespaces = 10):
    file.write(str(token))
    file.write(' ')
    """
    for _ in range(random.randint(1, max_number_of_whitespaces + 1)):
        file.write(sample(Whitespaces))
    """

def generate_module(file, symbol_table, instr_count):
    number_of_defs = random.randint(0, 16) if test_prob(SyntaxErrorProb) else random.randint(0, 30)
    number_of_uses = random.randint(0, 16) if test_prob(SyntaxErrorProb) else random.randint(0, 30)
    if test_prob(SyntaxErrorProb):
        number_of_instrs = 0 if instr_count >= 512 else random.randint(0, 512 - instr_count)
    else:
        number_of_instrs = random.randint(0, 512)

    write_token(file, number_of_defs)
    for _ in range(number_of_defs):
        token = generate_symbol(symbol_table)
        if number_of_instrs > 0 and test_prob(SyntaxErrorProb):
            offset = random.randint(0, number_of_instrs - 1)
        else:
            offset = random.randint(0, 512)
        if token not in symbol_table:
            symbol_table[token] = offset
        write_token(file, token)
        write_token(file, offset)
    file.write('\n')

    write_token(file, number_of_uses)
    use_list = []
    for _ in range(number_of_uses):
#        token = sample(list(symbol_table)) if test_prob(SemanticErrorProb) else generate_symbol(symbol_table)
        token = generate_symbol(symbol_table)
        use_list.append(token)
        write_token(file, token)
    file.write('\n')

    write_token(file, number_of_instrs)
    for _ in range(number_of_instrs):
        instr_type = generate_instr_type()
        instr_code = generate_instr_code(instr_type, number_of_instrs, use_list)
        write_token(file, instr_type)
        write_token(file, instr_code)
    file.write('\n')

    return number_of_instrs

def generate_input(path):
#    fout = open(path, 'w') 
    fout = sys.stdout
    symbol_table = { }
    number_of_modules = random.randint(1, MaxNumberOfModules)
    NumberOfModules = number_of_modules
    instr_count = 0;

    for i in range(number_of_modules):
        instr_count += generate_module(fout, symbol_table, instr_count)

    fout.close()

SyntaxErrorProb = 0.00
SemanticErrorProb = 0.05
MaxNumberOfModules = 20
NumberOfModules = 20

if __name__ == '__main__':
    generate_input('input')


