#ifndef CPU_H
#define CPU_H

#include "memory.h"
#include "latch.h"
#include "opcode.h"
#include "executionStats.h"

#define REGISTER_SIZE 32
#define PC_END 0xFFFFFFFF

extern int pc;
extern int reg[REGISTER_SIZE];

void init_register();
void init_memory();
int is_pipeline_active(Latch* latch);
void update_pc();
void fetch(Latch* latch);
void decode(Latch* latch);
Instruction decode_instruction(int inst_byte, int pc);
void execute(Latch* latch);
void access_memory(Latch* latch);
void write_back(Latch* latch);

#endif