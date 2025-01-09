#include "../header/latch.h"
#include <stdio.h>

void init_latch_invalid(Latch* latch){
    for(int i=0; i<2; i++){
        latch->IF_ID[i].valid = 0;
        latch->ID_EX[i].valid = 0;
        latch->EX_MEM[i].valid = 0;
        latch->MEM_WB[i].valid = 0;

        latch->IF_ID[i].stall = 0;
        latch->ID_EX[i].stall = 0;
        latch->EX_MEM[i].stall = 0;
        latch->EX_MEM[i].invalidate = 0;
    }
}

void init_latch(Latch* latch){
    memset(&(latch->IF_ID[1]), 0, sizeof(latch->IF_ID[1]));
    memset(&(latch->ID_EX[1]), 0, sizeof(latch->ID_EX[1]));
    memset(&(latch->EX_MEM[1]), 0, sizeof(latch->EX_MEM[1]));
    memset(&(latch->MEM_WB[1]), 0, sizeof(latch->MEM_WB[1]));
}

void invalidate_pipeline_stages(Latch* latch){
    // IF 단계랑, ID 단계의 결과 값을 Invalidate하게 만들기
    latch->EX_MEM[1].invalidate = 1;

    latch->IF_ID[1].valid = 0;
    memset(&latch->IF_ID[1].inst_byte, 0, sizeof(latch->IF_ID[1].inst_byte));
    latch->IF_ID[1].pc = 0;
  
    latch->ID_EX[1].valid = 0;
    memset(&latch->ID_EX[0].inst, 0, sizeof(Instruction));
    latch->ID_EX[0].pc = 0;
    
}

void update_latch(Latch* latch){
    latch->IF_ID[0] = latch->IF_ID[1];
    latch->ID_EX[0] = latch->ID_EX[1];
    latch->EX_MEM[0] = latch->EX_MEM[1];
    latch->MEM_WB[0] = latch->MEM_WB[1];
}