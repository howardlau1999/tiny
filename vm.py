import sys

stack = []
context = []
eip = None
funcs = {}
labels = {}
program = []

def do_add(): stack[-2] += stack[-1]; stack.pop();
def do_sub(): stack[-2] -= stack[-1]; stack.pop();
def do_mul(): stack[-2] *= stack[-1]; stack.pop();
def do_div(): stack[-2] /= stack[-1]; stack.pop();

def do_push(arg):
    if arg in context[-1]:
        val = context[-1][arg]
    else:
        val = int(arg)
    stack.append(val)

def do_pop(arg):
    context[-1][arg] = stack.pop()

def do_call(func_name: str):
    global eip
    context.append({})
    func = funcs[func_name]
    for param in reversed(func["params"]):
        context[-1][param["name"]] = stack.pop()
    if eip is not None:
        stack.append(eip)
    eip = func["addr"]

def do_ret():
    global eip
    eip = stack.pop(-2)
    context.pop()

def do_read(fn):
    with open(fn.replace('\"', ''), "r") as f:
        stack.append(int(f.readline()))

def do_write(fn):
    with open(fn.replace('\"', ''), "w") as f:
        print(stack.pop(), file=f)

def do_cmpeq():
    stack[-2] = int(stack[-2] == stack[-1])
    stack.pop()

def do_cmpne():
    stack[-2] = int(stack[-2] != stack[-1])
    stack.pop()

def do_jz(label):
    global eip
    if stack.pop() == 0:
        eip = labels[label]

def do_jmp(label):
    global eip
    eip = labels[label]

no_arg_table = {
    "ADD": do_add, "SUB": do_sub, "MUL": do_mul, "DIV": do_div, "RET": do_ret, "CMPEQ": do_cmpeq, "CMPNE": do_cmpne
}

one_arg_table = {
    "POP": do_pop, "PUSH": do_push, "READ": do_read, "CALL": do_call, "WRITE": do_write, "JMP": do_jmp, "JZ": do_jz
}

def execute():
    global eip
    while eip < len(program):
        line = program[eip]
        vals = line.split()
        eip += 1
        if vals[0] in no_arg_table:
            no_arg_table[vals[0]]()
        elif vals[0] in one_arg_table:
            one_arg_table[vals[0]](vals[1])
        elif vals[0][-1] == ":":
            pass
        elif "VAR" == vals[1]:
            context[-1][vals[-1]] = 0

def main():
    global program
    with open(sys.argv[1], "r") as asm_f:
        program = [line.strip() for line in asm_f.readlines()]
    
    entry_func = None
    cur = 0
    while cur < len(program):
        vals = program[cur].split()
        if vals[0][-1] == ":":
            labels[vals[0][:-1]] = cur

        if "FUNC" in vals:
            func_name = vals[-1]
            func_addr = cur + 1
            func_params = []
            if "ENTRY" in vals:
                entry_func = func_name
            while True:
                cur += 1
                vals = program[cur].split()
                if vals[0][-1] == ":":
                    labels[vals[0][:-1]] = cur
                if "END" in vals:
                    funcs[func_name] = {"addr": func_addr, "params": func_params}
                    break
                if "PARAM" in vals:
                    func_params.append({"name": vals[-1], "type": vals[0]})
                    func_addr = cur + 1
        cur += 1

    do_call(entry_func)
    execute()

if __name__ == '__main__':
    main()
    