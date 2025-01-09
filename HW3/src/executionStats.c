#include "../header/executionStats.h"
#include "../header/opcode.h"
#include <stdlib.h>

void update_instruction_stats(Instruction decoded_inst, ExecutionStats *stats){
    Opcode opcode = get_opcode(decoded_inst.opcode, decoded_inst.func);
    // stats->total_instructions++;

    if((opcode == BEQ) || (opcode == BNE)){
        stats->branch_count++;
    }else{
        stats->not_taken_branch_count++;
        if((opcode == J) || (opcode == JAL)){
            stats->jump_count++;
        } else if((opcode == LW) || (opcode == SW)){
            stats->memory_access_count++;
            if(opcode == LW){
                stats->register_operation_count++;
            }
        } else{
            stats->register_operation_count++;
        }
    }
}