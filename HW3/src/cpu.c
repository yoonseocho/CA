#include "../header/cpu.h"
#include <stdio.h>
#include <stdlib.h>

int pc = 0;
int reg[REGISTER_SIZE] = {0, };

void clearControlSignals(Latch* latch);
void set_control_signals(Opcode opcode, Latch* latch);
void update_instruction_stats(Instruction decoded_inst, ExecutionStats *stats);
void check_for_stalls(Latch* latch);
void data_forwarding(Latch* latch);
int ALU(int opcode, int rs, int rt, int s_imm, int shamt, int zero_ext_imm, int control_ALUOp, int pc);

ExecutionStats stats = {0};

void init_register(){
    reg[29] = 0x1000000;
    reg[31] = 0xffffffff;
}

void init_memory(){
    memory = (int *)malloc(sizeof(int) * MEMORY_SIZE);
}

int is_pipeline_active(Latch* latch){
    return latch->IF_ID[0].valid || latch->ID_EX[0].valid || latch->EX_MEM[0].valid || latch->MEM_WB[0].valid || pc < PC_END;
}
void update_pc(){
    pc += 4;
}

void fetch(Latch* latch){
    if(latch->IF_ID[1].stall){
        update_pc();
        return;
    }
    if(latch->EX_MEM[1].invalidate){
        update_pc();
        latch->IF_ID[1].inst_byte = inst_memory[pc/4];
        latch->IF_ID[1].pc = pc;
        latch->IF_ID[1].valid = 1;
        update_pc();
    }
    else{
        if(pc != PC_END){
            latch->IF_ID[1].inst_byte = inst_memory[pc/4];
            latch->IF_ID[1].valid = 1;
            latch->IF_ID[1].pc = pc;
            update_pc();
        }
        else{
            latch->IF_ID[1].valid = 0;
        }
    }
}

void decode(Latch* latch){
    if(!latch->IF_ID[0].valid){
        latch->ID_EX[1].valid = 0;
        return;
    }
    if(latch->ID_EX[0].stall){
        return;
    }

    if(latch->EX_MEM[1].invalidate){
        Instruction decoded_inst = decode_instruction(latch->IF_ID[1].inst_byte, latch->IF_ID[1].pc);
        Opcode opcode = get_opcode(decoded_inst.opcode, decoded_inst.func);

        latch->ID_EX[1].inst = decoded_inst;
        latch->ID_EX[1].pc = latch->EX_MEM[1].target_pc;
        pc = latch->ID_EX[1].pc;
        latch->ID_EX[1].valid = 1;

        clearControlSignals(latch);
        set_control_signals(opcode, latch);
        update_instruction_stats(latch->ID_EX[1].inst, &stats);
    }else{
        Instruction decoded_inst = decode_instruction(latch->IF_ID[0].inst_byte, latch->IF_ID[0].pc);
        Opcode opcode = get_opcode(decoded_inst.opcode, decoded_inst.func);

        latch->ID_EX[1].inst = decoded_inst;
        latch->ID_EX[1].pc =latch->IF_ID[0].pc;
        latch->ID_EX[1].valid = 1;

        clearControlSignals(latch);
        set_control_signals(opcode, latch);
        update_instruction_stats(latch->ID_EX[1].inst, &stats);
    }
}

Instruction decode_instruction(int inst_byte, int pc){
    Instruction decoded_inst;
    decoded_inst.opcode = (inst_byte >> 26) & 0x3F;
    decoded_inst.rs = (inst_byte >> 21) & 0x1F;
    decoded_inst.rt = (inst_byte >> 16) & 0x1F;
    decoded_inst.rd = (inst_byte >> 11) & 0x1F;

    decoded_inst.shamt = (inst_byte >> 6) & 0x1F;
    decoded_inst.func = inst_byte & 0x3F;
    decoded_inst.imm = inst_byte & 0xFFFF;
    decoded_inst.s_imm =  (decoded_inst.imm & 0x8000) ? (decoded_inst.imm | 0xFFFF0000) : decoded_inst.imm;
    decoded_inst.zero_ext_imm = decoded_inst.imm & 0xFFFF;
    decoded_inst.branch_addr = (decoded_inst.imm & 0x8000) ? (0xFFFC0000 | (decoded_inst.imm << 2)) : (decoded_inst.imm << 2);
    decoded_inst.jump_addr = (pc & 0xF0000000) | ((inst_byte & 0x3FFFFFF)<<2);
    return decoded_inst;
}

void execute(Latch* latch){
    if(!latch->ID_EX[0].valid){
        latch->EX_MEM[1].valid = 0;
        return;
    }
    if(latch->ID_EX[0].stall){
        return;
    }
    latch->EX_MEM[1].pc = latch->ID_EX[0].pc;
    latch->EX_MEM[1].control = latch->ID_EX[0].control;
    // latch->EX_MEM[1].inst = latch->ID_EX[0].inst;

    int input1 = reg[latch->ID_EX[0].inst.rs];
    int input2 = latch->ID_EX[0].control.ALUSrc ? latch->ID_EX[0].inst.s_imm : reg[latch->ID_EX[0].inst.rt];
    // int input1, input2;
    // printf("input1: %d, input2: %d\n", input1, input2);

    Opcode opcode = get_opcode(latch->ID_EX[0].inst.opcode, latch->ID_EX[0].inst.func);
    
    
    // printf("latch->EX_MEM[0].w_index?? %d\n",latch->EX_MEM[0].w_index);
    // printf("latch->ID_EX[0].inst.rs?? %d\n",latch->ID_EX[0].inst.rs);
    // printf("latch->ID_EX[0].inst.rt?? %d\n",latch->ID_EX[0].inst.rt);
   
   // 데이터 포워딩: EX/MEM -> ID/EX
   if(latch->EX_MEM[0].control.RegWrite){
        if(latch->EX_MEM[0].w_index == latch->ID_EX[0].inst.rs){
            input1 = latch->EX_MEM[0].ALU_result; // rs 포워딩
            // printf("[EX/MEM -> ID/EX] rs 포워딩 latch->EX_MEM[1].ALU_result?? %d\n",latch->EX_MEM[1].ALU_result);   
            // latch->IF_ID[1].stall = 1; // fetch 중단
            // latch->IF_ID[0].stall = 1; // decode 중단
            // latch->ID_EX[0].stall = 1; // execute 중단
        }
        if(latch->ID_EX[0].control.ALUSrc){
            if(latch->EX_MEM[0].w_index == latch->ID_EX[0].inst.rt){
                input2 = latch->EX_MEM[0].ALU_result; // rt 포워딩
                // printf("[EX/MEM -> ID/EX] rt 포워딩 latch->EX_MEM[1].ALU_result?? %d\n",latch->EX_MEM[1].ALU_result);
                // latch->IF_ID[1].stall = 1; // fetch 중단
                // latch->IF_ID[0].stall = 1; // decode 중단
                // latch->ID_EX[0].stall = 1; // execute 중단
            }
        }
   }

   // 데이터 포워딩: MEM/WB -> ID/EX
   if(latch->MEM_WB[0].control.RegWrite){
        if(latch->MEM_WB[0].w_index == latch->ID_EX[0].inst.rs){
            input1 = latch->MEM_WB[0].w_value;
            // printf("[MEM/WB -> ID/EX] latch->MEM_WB[1].w_value?? %d\n",input1);
            // latch->IF_ID[1].stall = 1; // fetch 중단
            // latch->IF_ID[0].stall = 1; // decode 중단
            // latch->ID_EX[0].stall = 1; // execute 중단
        }
        if(latch->MEM_WB[0].w_index == latch->ID_EX[0].inst.rt){
            input2 = latch->MEM_WB[0].w_value;
            // printf("[MEM/WB -> ID/EX] latch->MEM_WB[1].w_value?? %d\n",input2);
            // latch->IF_ID[1].stall = 1; // fetch 중단
            // latch->IF_ID[0].stall = 1; // decode 중단
            // latch->ID_EX[0].stall = 1; // execute 중단
        }
   }

    // ALU 연산
    // printf("input1: %d, input2: %d\n", input1, input2);
    latch->EX_MEM[1].w_index = latch->ID_EX[0].control.RegDest_ra ? 31 : latch->ID_EX[0].control.RegDest ? latch->ID_EX[0].inst.rd : latch->ID_EX[0].inst.rt;
    latch->EX_MEM[1].ALU_result = ALU(latch->ID_EX[0].inst.opcode, input1, input2, latch->ID_EX[0].inst.s_imm, latch->ID_EX[0].inst.shamt, latch->ID_EX[0].inst.zero_ext_imm, latch->ID_EX[0].control.ALUOp, latch->ID_EX[0].pc);
    
    if(latch->ID_EX[0].control.MemRead || latch->ID_EX[0].control.MemWrite){
        int effective_address = input1 + latch->ID_EX[0].inst.s_imm; // 유효 주소 계산
        latch->EX_MEM[1].mem_index = effective_address / 4; // 메모리 인덱스 계산
        if(latch->EX_MEM[1].mem_index < 0 || latch->EX_MEM[1].mem_index >= MEMORY_SIZE){
            fprintf(stderr, "Memory access error: Invalid memory index %d\n", latch->EX_MEM[1].mem_index);
            exit(EXIT_FAILURE);
        }
    }
    
    latch->EX_MEM[1].opcode = OpcodeNames[opcode];
    latch->EX_MEM[1].valid = 1;

    // Branch, jump 처리
    if(latch->ID_EX[0].control.Branch && ((opcode == BEQ && input1 == input2) || (opcode == BNE && input1 != input2))){
        pc += latch->ID_EX[0].inst.branch_addr;
        latch->EX_MEM[1].target_pc = pc;
        invalidate_pipeline_stages(latch);
    } else if(latch->ID_EX[0].control.Jump || latch->ID_EX[0].control.JAL){
        pc = latch->ID_EX[0].inst.jump_addr;
        latch->EX_MEM[1].target_pc = pc;
    } else if (latch->ID_EX[0].control.JR){
        pc = input1;
        latch->EX_MEM[1].target_pc = pc;
    }
    latch->EX_MEM[1].pc = latch->ID_EX[0].pc;
    // latch->EX_MEM[1].stall = latch->IF_ID[1].stall;
}

void access_memory(Latch* latch){
    if(!latch->EX_MEM[0].valid){
        latch->MEM_WB[1].valid = 0;
        return;
    }

    latch->MEM_WB[1].pc = latch->EX_MEM[0].pc;
    latch->MEM_WB[1].w_index = latch->EX_MEM[0].w_index;

    // int mem_index = latch->EX_MEM[0].ALU_result / 4;
    if(latch->EX_MEM[0].control.MemRead){ //lw
        latch->MEM_WB[1].mem_value = memory[latch->EX_MEM[0].mem_index];
    }
    if(latch->EX_MEM[0].control.MemWrite){ //sw
        memory[latch->EX_MEM[0].mem_index] = reg[latch->EX_MEM[0].w_index];
    }

    // latch->MEM_WB[1].ALU_result = latch->EX_MEM[0].ALU_result;
    latch->MEM_WB[1].valid = 1;
    latch->MEM_WB[1].opcode = latch->EX_MEM[0].opcode;
    // latch->MEM_WB[1].control = latch->EX_MEM[0].control;
    // latch->MEM_WB[1].mem_index = latch->EX_MEM[0].mem_index;
    // printf("latch->MEM_WB[1].mem_index??? %d\n",latch->MEM_WB[1].mem_index);
}

void write_back(Latch* latch){
    if(!latch->MEM_WB[0].valid) return;

    if(latch->MEM_WB[0].control.RegWrite){
        latch->MEM_WB[0].w_value = latch->MEM_WB->control.MemtoReg ? latch->MEM_WB[0].mem_value : latch->MEM_WB[0].ALU_result;
        if(latch->MEM_WB[0].control.RegDest){
            reg[latch->MEM_WB[0].w_index] = latch->MEM_WB[0].w_value;
        }
        else if(latch->MEM_WB[0].control.RegDest_ra){
            reg[latch->MEM_WB[0].w_index] = latch->MEM_WB[0].pc + 4;
        }
    }
}