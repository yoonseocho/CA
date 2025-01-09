#include "../header/latch.h"
#include <stdio.h>

void clearControlSignals(Latch* latch) {
    memset(&latch->ID_EX[1].control, 0, sizeof(ControlSignals));
}

void set_control_signals(Opcode opcode, Latch* latch){
    switch(opcode) {
        case ADD:
            latch->ID_EX[1].control.RegDest = 1;
            latch->ID_EX[1].control.ALUOp = 1;
            latch->ID_EX[1].control.RegWrite = 1;
            break;
        case ADDU:
            latch->ID_EX[1].control.RegDest = 1;
            latch->ID_EX[1].control.ALUOp = 2;
            latch->ID_EX[1].control.RegWrite = 1;
            break;
        case AND:
            latch->ID_EX[1].control.RegDest = 1;
            latch->ID_EX[1].control.ALUOp = 3;
            latch->ID_EX[1].control.RegWrite = 1;
            break;
        case NOR:
            latch->ID_EX[1].control.RegDest = 1;
            latch->ID_EX[1].control.ALUOp = 4;
            latch->ID_EX[1].control.RegWrite = 1;
            break;
        case OR:
            latch->ID_EX[1].control.RegDest = 1;
            latch->ID_EX[1].control.ALUOp = 5;
            latch->ID_EX[1].control.RegWrite = 1;
            break;
        case SLT:
            latch->ID_EX[1].control.RegDest = 1;
            latch->ID_EX[1].control.ALUOp = 6;
            latch->ID_EX[1].control.RegWrite = 1;
            break;
        case SLTU:
            latch->ID_EX[1].control.RegDest = 1;
            latch->ID_EX[1].control.ALUOp = 7;
            latch->ID_EX[1].control.RegWrite = 1;
            break;
        case SLL:
            latch->ID_EX[1].control.RegDest = 1;
            latch->ID_EX[1].control.ALUOp = 8;
            latch->ID_EX[1].control.RegWrite = 1;
            break;
        case SRL:
            latch->ID_EX[1].control.RegDest = 1;
            latch->ID_EX[1].control.ALUOp = 9;
            latch->ID_EX[1].control.RegWrite = 1;
            break;
        case SUB:
            latch->ID_EX[1].control.RegDest = 1;
            latch->ID_EX[1].control.ALUOp = 10;
            latch->ID_EX[1].control.RegWrite = 1;
            break;
        case SUBU:
            latch->ID_EX[1].control.RegDest = 1;
            latch->ID_EX[1].control.ALUOp = 11;
            latch->ID_EX[1].control.RegWrite = 1;
            break;
        case JR:
            latch->ID_EX[1].control.ALUOp = 12;
            latch->ID_EX[1].control.JR = 1;
            break;
        case JALR:
            latch->ID_EX[1].control.ALUOp = 13;
            latch->ID_EX[1].control.RegWrite = 1;
            latch->ID_EX[1].control.RegDest_ra = 1;
            latch->ID_EX[1].control.JR = 1;
            break;
        case ADDI:
        case ADDIU:
        case ANDI:
        case LUI:
        case ORI:
        case SLTI:
        case SLTIU:
            latch->ID_EX[1].control.RegWrite = 1;
            latch->ID_EX[1].control.ALUSrc = 1;
            break;
        case LW:
            latch->ID_EX[1].control.RegWrite = 1;
            latch->ID_EX[1].control.ALUSrc = 1;
            latch->ID_EX[1].control.MemtoReg = 1;
            latch->ID_EX[1].control.MemRead = 1;
            break;
        case SW:
            latch->ID_EX[1].control.RegWrite = 1;
            latch->ID_EX[1].control.ALUSrc = 1;
            latch->ID_EX[1].control.MemWrite = 1;
            break;
        case BEQ:
        case BNE:
            latch->ID_EX[1].control.ALUSrc = 1;
            latch->ID_EX[1].control.Branch = 1;
            break;
        case J:
            latch->ID_EX[1].control.Jump = 1;
            break;
        case JAL:
            latch->ID_EX[1].control.RegWrite = 1;
            latch->ID_EX[1].control.RegDest_ra = 1;
            latch->ID_EX[1].control.JAL = 1;
            break;
        default:
            break;
    }
}