#ifndef EXECUTIONSTATS_H
#define EXECUTIONSTATS_H

#include "memory.h"

typedef struct {
    int total_instructions;
    int memory_access_count;
    int register_operation_count;
    int branch_count;
    int not_taken_branch_count;
    int jump_count;
} ExecutionStats;

extern ExecutionStats stats;

void update_instruction_stats(Instruction decoded_inst, ExecutionStats *stats);

#endif