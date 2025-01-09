#ifndef CPU_H
#define CPU_H

#include "memory.h"
#include "control_signal.h"
#include "executionStats.h"

extern int pc;
extern int reg[];

ExecutionStats stats;

void init_register();
int fetch();
Instruction decode(int inst_byte);
void execute(Instruction decoded_inst, int* ALU_result, int* mem_index, ControlSignals *control);
void access_memory(Instruction decoded_inst, int mem_index, int* mem_value, ControlSignals *control, ExecutionStats *stats);
void write_back(Instruction decoded_inst, int ALU_result, int mem_value, ControlSignals *control);
void branchAddr_ALU(int branch_Addr);
void update_pc();

#endif