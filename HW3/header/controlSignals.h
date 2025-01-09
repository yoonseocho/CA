#ifndef CONTROLSIGNALS_H
#define CONTROLSIGNALS_H

typedef struct {
    int RegDest;
    int RegDest_ra;
    int ALUSrc;
    int ALUOp;
    int MemtoReg;
    int MemRead;
    int RegWrite;
    int MemWrite;
    int Branch;
    int Jump;
    int JR;
    int JAL;
} ControlSignals;

#endif