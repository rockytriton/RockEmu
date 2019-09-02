//
//  op_codes.c
//  Emu6502
//
//  Created by Rocky Pulley on 8/25/19.
//  Copyright © 2019 Rocky Pulley. All rights reserved.
//

#include "op_codes.h"


struct OpCode opCodeLookup[] = {
    //0
    {"BRK", 1, IMP, 7},
    {"ORA", 2, INDX, 6},
    {"XXX", 0, IMP, 0},
    {"XXX", 0, IMP, 0},
    {"XXX", 0, IMP, 0},
    {"ORA", 2, ZP, 3},
    {"ASL", 2, ZP, 1},
    {"XXX", 0, IMP, 0},
    {"PHP", 1, IMP, 3},
    {"ORA", 2, IMM, 2},
    {"ASL", 1, ACC, 2},
    {"XXX", 0, IMP, 0},
    {"XXX", 0, IMP, 0},
    {"ORA", 3, ABS, 4},
    {"ASL", 3, ABS, 6},
    {"XXX", 0, IMP, 0},
    
    //1
    {"BPL", 2, REL, 2},
    {"ORA", 2, INDY, 5},
    {"XXX", 0, IMP, 0},
    {"XXX", 0, IMP, 0},
    {"XXX", 0, IMP, 0},
    {"ORA", 2, ZPX, 4},
    {"ASL", 2, ZPX, 6},
    {"XXX", 0, IMP, 0},
    {"CLC", 1, IMP, 2},
    {"ORA", 3, ABSY, 4},
    {"XXX", 0, IMP, 0},
    {"XXX", 0, IMP, 0},
    {"XXX", 0, IMP, 0},
    {"ORA", 3, ABSX, 4},
    {"ASL", 3, ABSX, 7},
    {"XXX", 0, IMP, 0},
    
    //2
    {"JSR", 3, ABS, 6},
    {"AND", 2, INDX, 6},
    {"XXX", 0, IMP, 0},
    {"XXX", 0, IMP, 0},
    {"BIT", 2, ZP, 3},
    {"AND", 2, ZP, 3},
    {"ROL", 2, ZP, 5},
    {"XXX", 0, IMP, 0},
    {"PLP", 1, IMP, 4},
    {"AND", 2, IMM, 2},
    {"ROL", 1, ACC, 2},
    {"XXX", 0, IMP, 0},
    {"BIT", 3, ABS, 4},
    {"AND", 3, ABS, 4},
    {"ROL", 3, ABS, 5},
    {"XXX", 0, IMP, 0},
    
    //3
    {"BMI", 2, REL, 2},
    {"AND", 2, INDY, 5},
    {"XXX", 0, IMP, 0},
    {"XXX", 0, IMP, 0},
    {"XXX", 0, IMP, 0},
    {"AND", 2, ZPX, 4},
    {"ROL", 2, ZPX, 6},
    {"XXX", 0, IMP, 0},
    {"SEC", 1, IMP, 2},
    {"AND", 3, ABSY, 4},
    {"XXX", 0, IMP, 0},
    {"XXX", 0, IMP, 0},
    {"XXX", 0, IMP, 0},
    {"AND", 3, ABSX, 4},
    {"ROL", 3, ABSX, 7},
    {"XXX", 0, IMP, 0},
    
    //4
    {"RTI", 1, IMP, 6},
    {"EOR", 2, INDX, 6},
    {"XXX", 0, IMP, 0},
    {"XXX", 0, IMP, 0},
    {"XXX", 0, IMP, 0},
    {"EOR", 2, ZP, 3},
    {"LSR", 2, ZP, 5},
    {"XXX", 0, IMP, 0},
    {"PHA", 1, IMP, 3},
    {"EOR", 2, IMM, 2},
    {"LSR", 1, ACC, 2},
    {"XXX", 0, IMP, 0},
    {"JMP", 3, ABS, 3},
    {"EOR", 3, ABS, 4},
    {"LSR", 3, ABS, 6},
    {"XXX", 0, IMP, 0},
    
    //5
    {"BVC", 2, REL, 2},
    {"EOR", 2, INDY, 5},
    {"XXX", 0, IMP, 0},
    {"XXX", 0, IMP, 0},
    {"XXX", 0, IMP, 0},
    {"EOR", 2, ZPX, 4},
    {"LSR", 2, ZPX, 6},
    {"XXX", 0, IMP, 0},
    {"CLI", 1, IMP, 2},
    {"EOR", 3, ABSY, 4},
    {"XXX", 0, IMP, 0},
    {"XXX", 0, IMP, 0},
    {"XXX", 0, IMP, 0},
    {"EOR", 3, ABSX, 4},
    {"LSR", 3, ABSX, 7},
    {"XXX", 0, IMP, 0},
    
    //6
    {"RTS", 1, IMP, 6},
    {"ADC", 2, INDX, 6},
    {"XXX", 0, IMP, 0},
    {"XXX", 0, IMP, 0},
    {"XXX", 0, IMP, 0},
    {"ADC", 2, ZP, 3},
    {"ROR", 2, ZP, 5},
    {"XXX", 0, IMP, 0},
    {"PLA", 1, IMP, 4},
    {"ADC", 2, IMM, 2},
    {"ROR", 1, ACC, 2},
    {"XXX", 0, IMP, 0},
    {"JMP", 3, IND, 5},
    {"ADC", 3, ABS, 4},
    {"ROR", 3, ABS, 6},
    {"XXX", 0, IMP, 0},
    
    //7
    {"BVS", 2, REL, 2},
    {"ADC", 2, INDY, 5},
    {"XXX", 0, IMP, 0},
    {"XXX", 0, IMP, 0},
    {"XXX", 0, IMP, 0},
    {"ADC", 2, ZPX, 4},
    {"ROR", 2, ZPX, 6},
    {"XXX", 0, IMP, 0},
    {"SEI", 1, IMP, 2},
    {"ADC", 3, ABSY, 4},
    {"XXX", 0, IMP, 0},
    {"XXX", 0, IMP, 0},
    {"XXX", 0, IMP, 0},
    {"ADC", 3, ABSX, 4},
    {"ROR", 3, ABSX, 7},
    {"XXX", 0, IMP, 0},
    
    //8
    {"XXX", 0, IMP, 0},
    {"STA", 2, INDX, 6},
    {"XXX", 0, IMP, 0},
    {"XXX", 0, IMP, 0},
    {"STY", 2, ZP, 3},
    {"STA", 2, ZP, 3},
    {"STX", 2, ZP, 3},
    {"XXX", 0, IMP, 0},
    {"DEY", 1, IMP, 2},
    {"XXX", 0, IMP, 0},
    {"TXA", 1, IMP, 2},
    {"XXX", 0, IMP, 0},
    {"STY", 3, ABS, 4},
    {"STA", 3, ABS, 4},
    {"STX", 3, ABS, 4},
    {"XXX", 0, IMP, 0},
    
    //9
    {"BCC", 2, REL, 2}, //TODO MAYBE CHANGE BACK TO 2
    {"STA", 2, INDY, 6},
    {"XXX", 0, IMP, 0},
    {"XXX", 0, IMP, 0},
    {"STY", 2, ZPX, 4},
    {"STA", 2, ZPX, 4},
    {"STX", 2, ZPY, 4},
    {"XXX", 0, IMP, 0},
    {"TYA", 1, IMP, 2},
    {"STA", 3, ABSY, 5},
    {"TXS", 1, IMP, 2},
    {"XXX", 0, IMP, 0},
    {"XXX", 0, IMP, 0},
    {"STA", 3, ABSX, 5},
    {"XXX", 0, IMP, 0},
    {"XXX", 0, IMP, 0},
    
    //A
    {"LDY", 2, IMM, 2},
    {"LDA", 2, INDX, 6},
    {"LDX", 2, IMM, 2},
    {"XXX", 0, IMP, 0},
    {"LDY", 2, ZP, 3},
    {"LDA", 2, ZP, 3},
    {"LDX", 2, ZP, 3},
    {"XXX", 0, IMP, 0},
    {"TAY", 1, IMP, 2},
    {"LDA", 2, IMM, 2},
    {"TAX", 1, IMP, 2},
    {"XXX", 0, IMP, 0},
    {"LDY", 3, ABS, 4},
    {"LDA", 3, ABS, 4},
    {"LDX", 3, ABS, 4},
    {"XXX", 0, IMP, 0},
    
    //B
    {"BCS", 2, REL, 2},
    {"LDA", 2, INDY, 5},
    {"XXX", 0, IMP, 0},
    {"XXX", 0, IMP, 0},
    {"LDY", 2, ZPX, 4},
    {"LDA", 2, ZPX, 4},
    {"LDX", 2, ZPY, 4},
    {"XXX", 0, IMP, 0},
    {"CLV", 1, IMP, 2},
    {"LDA", 3, ABSY, 4},
    {"TSX", 1, IMP, 2},
    {"XXX", 0, IMP, 0},
    {"LDY", 3, ABSX, 4},
    {"LDA", 3, ABSX, 4},
    {"LDX", 3, ABSY, 4},
    {"XXX", 0, IMP, 0},
    
    //C
    {"CPY", 2, IMM, 2},
    {"CMP", 2, INDX, 6},
    {"XXX", 0, IMP, 0},
    {"XXX", 0, IMP, 0},
    {"CPY", 2, ZP, 3},
    {"CMP", 2, ZP, 3},
    {"DEC", 2, ZP, 5},
    {"XXX", 0, IMP, 0},
    {"INY", 1, IMP, 2},
    {"CMP", 2, IMM, 2},
    {"DEX", 1, IMP, 2},
    {"XXX", 0, IMP, 0},
    {"CPY", 3, ABS, 4},
    {"CMP", 3, ABS, 4},
    {"DEC", 3, ABS, 6},
    {"XXX", 0, IMP, 0},
    
    //D
    {"BNE", 2, REL, 2},
    {"CMP", 2, INDY, 5},
    {"XXX", 0, IMP, 0},
    {"XXX", 0, IMP, 0},
    {"XXX", 0, IMP, 0},
    {"CMP", 2, ZPX, 4},
    {"DEC", 2, ZPX, 5},
    {"XXX", 0, IMP, 0},
    {"CLD", 1, IMP, 2},
    {"CMP", 3, ABSY, 4},
    {"XXX", 0, IMP, 0},
    {"XXX", 0, IMP, 0},
    {"XXX", 0, IMP, 0},
    {"CMP", 3, ABSX, 4},
    {"DEC", 3, ABSX, 7},
    {"XXX", 0, IMP, 0},
    
    //E
    {"CPX", 2, IMM, 2},
    {"SBC", 2, INDX, 6},
    {"XXX", 0, IMP, 0},
    {"XXX", 0, IMP, 0},
    {"CPX", 2, ZP, 3},
    {"SBC", 2, ZP, 3},
    {"INC", 2, ZP, 5},
    {"XXX", 0, IMP, 0},
    {"INX", 1, IMP, 2},
    {"SBC", 2, IMM, 2},
    {"NOP", 1, IMP, 2},
    {"XXX", 0, IMP, 0},
    {"CPX", 3, ABS, 4},
    {"SBC", 3, ABS, 4},
    {"INC", 3, ABS, 6},
    {"XXX", 0, IMP, 0},
    
    //F
    {"BEQ", 2, REL, 2},
    {"SBC", 2, INDY, 5},
    {"XXX", 0, IMP, 0},
    {"XXX", 0, IMP, 0},
    {"XXX", 0, IMP, 0},
    {"SBC", 2, ZPX, 4},
    {"INC", 2, ZPX, 6},
    {"XXX", 0, IMP, 0},
    {"SED", 1, IMP, 2},
    {"SBC", 3, ABSY, 4},
    {"XXX", 0, IMP, 0},
    {"XXX", 0, IMP, 0},
    {"XXX", 0, IMP, 0},
    {"SBC", 3, ABSX, 4},
    {"INC", 3, ABSX, 7},
    {"XXX", 0, IMP, 0},
    
};
