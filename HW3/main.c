#include "header/cpu.h"
#include "header/executionStats.h"
#include <stdio.h>
#include <stdlib.h>

void print_cycle();
void print_fetch(Latch* latch);
void print_decode(Latch* latch);
void print_execute(Latch* latch);
void print_memoryAccess(Latch* latch);
void print_writeBack(Latch* latch);
void print_result(ExecutionStats *stats);
void print_register();

int clock_cycle = 0;
int total_instructions = 0;

int main(int argc, char *argv[]){
    if(argc < 2){
        fprintf(stderr, "Usage: %s <file_path>\n", argv[0]);
        return 1;
    }

    int num_instructions = load_instructions(argv[1], inst_memory);
    if(num_instructions < 0){
        fprintf(stderr, "Failed to load instructions from %s\n", argv[1]);
        return 1;
    }
    total_instructions = num_instructions;

    Latch latch;

    init_register();
    init_memory();
    init_latch_invalid(&latch);
    // int n=0;
    while(is_pipeline_active(&latch)){//n<80
        write_back(&latch);
        print_writeBack(&latch);
        
        access_memory(&latch);
        print_memoryAccess(&latch);

        execute(&latch);
        print_execute(&latch);

        decode(&latch);
        print_decode(&latch);

        fetch(&latch);
        print_fetch(&latch);

        update_latch(&latch);
        init_latch(&latch);
        // print_register();
        print_cycle();
        // n++;
    }

    print_result(&stats);
    free(memory);
    return 0;
}

void print_cycle(){
    printf("cycle: %d\n", ++clock_cycle);
    printf("-----------------------\n");
}

void print_fetch(Latch* latch){
    printf("[IF]");
    if(latch->IF_ID[1].stall){
        printf(" STALL\n");
        return;
    }
    if(!latch->IF_ID[1].valid){
        printf("\n");
        return;
    }
    printf(" @0x%x ", latch->IF_ID[1].pc);
    printf("instruction: 0x%x\n", latch->IF_ID[1].inst_byte);
}

void print_decode(Latch* latch){
    printf("[ID]");
    if(latch->IF_ID[0].stall){
        printf(" STALL\n");
        return;
    }
    if(!latch->IF_ID[0].valid){
        printf("\n");
        return;
    }
    Opcode opcode = get_opcode(latch->ID_EX[1].inst.opcode, latch->ID_EX[1].inst.func);
    printf(" @0x%x (%s) ", latch->ID_EX[1].pc, OpcodeNames[opcode]);

    if(latch->ID_EX[1].inst.opcode==0x0){
        printf("rs: %d, ", latch->ID_EX[1].inst.rs);
        printf("rt: %d, ", latch->ID_EX[1].inst.rt);
        printf("rd: %d, ", latch->ID_EX[1].inst.rd);
        printf("shamt: %d, ", latch->ID_EX[1].inst.shamt);
        printf("func: 0x%x", latch->ID_EX[1].inst.func);
    }
    else if(opcode == J || opcode == JAL){
        printf("jump_addr: 0x%x", latch->ID_EX[1].inst.jump_addr);
    }
    else{
        printf("rs: %d, ", latch->ID_EX[1].inst.rs);
        printf("rt: %d, ", latch->ID_EX[1].inst.rt);
        printf("s_imm: %d", latch->ID_EX[1].inst.s_imm);
    }
    printf("\n");
}

void print_execute(Latch* latch){
    printf("[EX]");
    if(!latch->ID_EX[0].valid){
        printf("\n");
        return;
    }
    if(latch->ID_EX[0].stall){
        printf(" STALL\n");
        return;
    }
    printf(" @0x%x (%s) ", latch->EX_MEM[1].pc, latch->EX_MEM[1].opcode);
    
    if(latch->ID_EX[0].control.Branch || latch->ID_EX[0].control.Jump || latch->ID_EX[0].control.JAL || latch->ID_EX[0].control.JR){
        printf("pc: 0x%x\n", latch->EX_MEM[1].target_pc);
    }
    else if(strcmp(latch->EX_MEM[1].opcode, "lw")==0 || strcmp(latch->EX_MEM[1].opcode, "sw")==0){
        printf("\n");
    }
    else{
        printf("ALU_result: %d\n", latch->EX_MEM[1].ALU_result);
    }
    
}

void print_memoryAccess(Latch* latch){
    printf("[MEM]");
    if(!latch->EX_MEM[0].valid){
        printf("\n");
        return;
    }
    printf(" @0x%x (%s) ", latch->MEM_WB[1].pc, latch->EX_MEM[0].opcode);

    if(latch->EX_MEM[0].control.MemWrite){
        printf("%d is stored in M[%d]\n", reg[latch->MEM_WB[1].w_index], latch->MEM_WB[1].mem_index);
        return;
    }

    if(latch->EX_MEM[0].control.MemRead){
        printf("M[%d]: %d\n", latch->EX_MEM[0].mem_index, latch->MEM_WB[1].mem_value);
    }else{
        printf("no memory access!\n");
    }
    
}

void print_writeBack(Latch* latch){
    printf("[WB]");
    if(!latch->MEM_WB[0].valid){
        printf("\n");
        return;
    }
    printf(" @0x%x (%s) ", latch->MEM_WB[0].pc, latch->MEM_WB[0].opcode);
    if(latch->MEM_WB[0].control.RegWrite){
        printf("R[%d] = %d\n", latch->MEM_WB[0].w_index, latch->MEM_WB[0].w_value);
    }else{
        printf("nothing to write back!\n");
    }
}


void print_result(ExecutionStats *stats){
    printf("\n===========================PROGRAM RESULT=============================\n");
	printf("Return value R[2] : %d\n", reg[2]);
	printf("Total Cycles : %d\n", clock_cycle);
    printf("Total # of Instructions: %d\n", total_instructions); //stats->total_instructions
    printf("Total # of Memory Operation Instructions: %d\n", stats->memory_access_count);
    printf("Total # of Register Operation Instructions: %d\n", stats->register_operation_count);
    printf("Total # of Branch Instructions: %d\n", stats->branch_count);
    printf("Total # of Not-Taken Branches: %d\n", stats->not_taken_branch_count);
    printf("Total # of Jump Instructions: %d\n", stats->jump_count);
    printf("======================================================================\n");
    return;
}

void print_register(){
    for(int i=0; i<31; i++){
        printf("reg[%d] = %d\n", i, reg[i]);
    }
}