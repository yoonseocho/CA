#ifndef MEMORY_H
#define MEMORY_H

#define MEMORY_SIZE 0x400000

typedef struct {
    int opcode;
    int rs;
    int rt;
    int rd;

    int shamt;
    int func;
    int imm;
    int s_imm;
    int zero_ext_imm;
    int branch_addr;
    int jump_addr;
} Instruction;

extern int inst_memory[MEMORY_SIZE];
extern int memory[MEMORY_SIZE];

int load_instructions(const char *filename);

#endif