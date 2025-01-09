#ifndef LATCH_H
#define LATCH_H

#include "memory.h"
#include "controlSignals.h"
#include "opcode.h"
#include <string.h>

typedef struct {
    int pc;
    int inst_byte;
    int valid;
    int stall;
} IF_ID_Latch;

typedef struct {
    int pc;
    Instruction inst;
    int w_index;
    int valid;
    int stall;
    int input1;
    int input2;
    ControlSignals control;
} ID_EX_Latch;

typedef struct{
    int pc;
    int target_pc;
    const char *opcode;
    Instruction inst;
    int ALU_result;
    int w_index;
    int valid;
    int invalidate;
    int stall;
    int input1;
    int input2;
    int mem_index;
    ControlSignals control;
} EX_MEM_Latch;

typedef struct {
    int pc;
    const char *opcode;
    int ALU_result;
    int mem_index;
    int mem_value;
    int w_index;
    int w_value;
    int valid;
    ControlSignals control;
} MEM_WB_Latch;

typedef struct {
    IF_ID_Latch IF_ID[2];
    ID_EX_Latch ID_EX[2];
    EX_MEM_Latch EX_MEM[2];
    MEM_WB_Latch MEM_WB[2];

    int data_hazard;
} Latch;

typedef enum{
    IF,
    ID,
    EX,
    MEM,
    WB
} PipelineStage;

void init_latch_invalid(Latch* latch);
void init_latch(Latch* latch);
void invalidate_pipeline_stages(Latch* latch);
void update_latch(Latch* latch);
void clearControlSignals(Latch* latch);
void set_control_signals(Opcode opcode, Latch* latch);

#endif