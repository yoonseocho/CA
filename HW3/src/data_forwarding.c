#include "../header/latch.h"
#include "../header/cpu.h"
#include <stdio.h>

void data_forwarding(Latch* latch){
    if(latch->EX_MEM[1].control.RegWrite){// execution이랑 writeBack data dependancy 확인
        if(latch->EX_MEM[1].w_index == latch->EX_MEM[1].inst.rs && latch->EX_MEM[1].w_index != 0){ // rs에 대한 포워딩
            reg[latch->EX_MEM[1].w_index] = latch->EX_MEM[1].control.RegDest_ra ? latch->EX_MEM[1].pc : latch->EX_MEM[1].ALU_result;
            printf("[rs에 대한 포워딩 처리 중] R[%d] = 0x%x ", latch->EX_MEM[1].w_index, reg[latch->EX_MEM[1].w_index]);
        }
        else if(latch->EX_MEM[1].w_index == latch->EX_MEM[1].inst.rt && latch->EX_MEM[1].w_index != 0){ // rt에 대한 포워딩
            reg[latch->EX_MEM[1].w_index] = latch->EX_MEM[1].ALU_result;
            printf("[rt에 대한 포워딩 처리 중] R[%d] = 0x%x ", latch->EX_MEM[1].w_index, reg[latch->EX_MEM[1].w_index]);
        }
    }

    if(latch->EX_MEM[0].control.MemRead){// execution이랑 memoryAccess랑 data dependancy 확인
        if(latch->EX_MEM[0].control.RegWrite){
            if(latch->MEM_WB[1].w_index == latch->ID_EX[0].inst.rs && latch->MEM_WB[1].w_index != 0){ // rs에 대한 포워딩
                fprintf(stderr, "[MEM hazrd1 처리 중]\n");
                
            }
            else if(latch->MEM_WB[1].w_index == latch->ID_EX[0].inst.rt && latch->MEM_WB[1].w_index != 0){// rt에 대한 포워딩
                fprintf(stderr, "[MEM hazrd2 처리 중]\n");
            }

            // 앞에 나오는 isnt가 레지스터에 쓰려고 해. 그리고 뒤에 따라오는 inst가 레지스터를 읽으려고 해. 
            // execution이랑 memoryAccess랑, execution이랑 writeBack data dependancy 확인
            // 레지스터에 쓸려고 하는 index와 읽어오려고하는 index가 같으면 data dependency가 있는 것이다.. EX hazard             rt에 쓸때, rd에 쓸때, 31번에 쓸때, 구분해서 쓰기만 하면 된다. 그리고 memory, writeBack stage에서 data dependancy 확인만 하면된다.
            // 메모리에 쓰려고 하는 index와 읽어오려고 하는 index가 같으면 data dependency가 있는 것이다.. MEM hazard
        }
    }
}

void check_for_stalls(Latch* latch){
    if(latch->ID_EX[0].control.MemRead){
        latch->IF_ID[0].valid = 0;
        latch->IF_ID[1].valid = 0;
        latch->ID_EX[0].valid = 0;
        fprintf(stderr, "[STALL]\n");
    }else{
        data_forwarding(latch);
    }
}