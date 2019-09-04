//
//  instructions.c
//  Emu6502
//
//  Created by Rocky Pulley on 8/25/19.
//  Copyright © 2019 Rocky Pulley. All rights reserved.
//

#include "instructions.h"
#include "bus.h"
#include "cpu.h"

static int initialized = 0;

extern int counter;

#define DEF_F(x) uint8_t x(struct CpuData *data, struct OpCode *opCode)

#define STACK_PUSH(x) bus_write(data->sp + 0x0100, x); data->sp--;// DOLOG("%0.8X STACK WRITE %0.2X = %0.2X\r\n", counter - 1, data->sp + 1, x);

#define STACK_POP(x) data->sp++; x = bus_read(data->sp + 0x0100);//DOLOG("%0.8X STACK READ %0.2X = %0.2X\r\n", counter - 1, data->sp - 1, x);

extern struct CpuData cpuData;

void setNZFlags(uint8_t n) {
    cpu_set_flag(STATREG_Z, n == 0);
    cpu_set_flag(STATREG_N, n & 0x80);
}

void setNZFlags16(uint16_t n) {
    cpu_set_flag(STATREG_Z, (n & 0x00FF) == 0x0000);
    cpu_set_flag(STATREG_N, n & 0x0080);
}

uint8_t handle_JMP(struct CpuData *data, struct OpCode *opCode) {
    data->pc = data->addr_abs;
    //DOLOG("SET PC 01: %0.4X - %0.4X\r\n", data->pc, data->addr_abs);
    return 0;
}

uint8_t handle_BRK(struct CpuData *data, struct OpCode *opCode) {
    cpu_interrupt(0);
    /*
    data->pc++;
    
    cpu_set_flag(STATREG_I, 1);
    
    STACK_PUSH(((data->pc) >> 8) & 0x00FF);
    STACK_PUSH((data->pc) & 0x00FF);
    
    cpu_set_flag(STATREG_B, 1);
    STACK_PUSH(data->regStatus);
    
    cpu_set_flag(STATREG_B, 0);
    
    uint16_t a = (uint16_t)bus_read(0xFFFE);
    uint16_t b = (uint16_t)bus_read(0xFFFF);
    
    data->pc = a | (b << 8);
     */
    //DOLOG("SET PC 02: %0.4X - %0.4X\r\n", data->pc, data->addr_abs);
    return 0;
}

DEF_F(handle_SEI) {
    cpu_set_flag(STATREG_I, 1);
    return 0;
}

DEF_F(handle_SEC) {
    cpu_set_flag(STATREG_C, 1);
    return 0;
}

DEF_F(handle_SED) {
    cpu_set_flag(STATREG_D, 1);
    return 0;
}

DEF_F(handle_CLC) {
    cpu_set_flag(STATREG_C, 0);
    return 0;
}

DEF_F(handle_CLD) {
    cpu_set_flag(STATREG_D, 0);
    return 0;
}

DEF_F(handle_CLI) {
    cpu_set_flag(STATREG_I, 0);
    return 0;
}

DEF_F(handle_CLV) {
    cpu_set_flag(STATREG_V, 0);
    return 0;
}

DEF_F(handle_LDA) {
    data->regA = data->fetched;
    setNZFlags(data->regA);
    return 1;
}

DEF_F(handle_LDX) {
    data->regX = data->fetched;
    setNZFlags(data->regX);
    return 1;
}

DEF_F(handle_LDY) {
    //DOLOG("LDY @ %0.4X = %0.2X ", data->addr_abs, data->fetched);
    data->regY = data->fetched;
    setNZFlags(data->regY);
    //DOLOG("NOW: %0.2X\r\n", data->regY);
    return 1;
}

DEF_F(handle_STA) {
    bus_write(data->addr_abs, data->regA);
    //DOLOG("STA: %0.4X\r\n", data->addr_abs);
    return 0;
}

DEF_F(handle_STX) {
    bus_write(data->addr_abs, data->regX);
    return 0;
}

DEF_F(handle_STY) {
    bus_write(data->addr_abs, data->regY);
    return 0;
}

DEF_F(handle_AND) {
    data->regA = data->regA & data->fetched;
    setNZFlags(data->regA);
    return 1;
}

DEF_F(handle_BEQ) {
    if (cpu_get_flag(STATREG_Z)) {
        data->cycles++;
        data->addr_abs = data->pc + data->addr_rel;
        
        if ((data->addr_abs & 0xFF00) != ((data->pc) & 0xFF00)) {
            data->cycles++;
            data->cycles++;
        }
        
        data->pc = data->addr_abs;
        //DOLOG("SET PC 03: %0.4X - %0.4X\r\n", data->pc, data->addr_abs);
    }
    
    return 0;
}

DEF_F(handle_DEX) {
    data->regX--;
    setNZFlags(data->regX);
    return 0;
}

DEF_F(handle_DEY) {
    data->regY--;
    setNZFlags(data->regY);
    return 0;
}

DEF_F(handle_TAX) {
    data->regX = data->regA;
    setNZFlags(data->regX);
    return 0;
}

DEF_F(handle_TAY) {
    data->regY = data->regA;
    setNZFlags(data->regY);
    return 0;
}

DEF_F(handle_TSX) {
    data->regX = data->sp;
    setNZFlags(data->regX);
    return 0;
}

DEF_F(handle_TXA) {
    data->regA = data->regX;
    setNZFlags(data->regA);
    return 0;
}

DEF_F(handle_TXS) {
    data->sp = data->regX;
    return 0;
}

DEF_F(handle_TYA) {
    data->regA = data->regY;
    setNZFlags(data->regA);
    return 0;
}

DEF_F(handle_BIT) {
    uint16_t temp = data->regA & data->fetched;
    cpu_set_flag(STATREG_Z, (temp & 0x00FF) == 0x00);
    cpu_set_flag(STATREG_N, data->fetched & (1 << 7));
    cpu_set_flag(STATREG_V, data->fetched & (1 << 6));
    return 0;
}

DEF_F(handle_BPL) {
    if (!cpu_get_flag(STATREG_N)) {
        
        data->cycles++;
        data->addr_abs = (data->pc) + data->addr_rel;
        
        if ((data->addr_abs & 0xFF00) != ((data->pc) & 0xFF00)) {
            data->cycles++;
        }
        
        data->pc = data->addr_abs;
        //DOLOG("SET PC 04: %0.4X - %0.4X - %0.4X - %0.4X\r\n", data->pc, data->addr_abs, data->addr_rel, (uint16_t)(data->addr_abs + data->addr_rel));
    } else {
        return 0;
    }
    return 0;
}

DEF_F(handle_BNE) {
    if (!cpu_get_flag(STATREG_Z)) {
        data->cycles++;
        data->addr_abs = (data->pc) + data->addr_rel;
        
        if ((data->addr_abs & 0xFF00) != ((data->pc) & 0xFF00)) {
            data->cycles++;
        }
        
        data->pc = data->addr_abs;
        //DOLOG("SET PC 05: %0.4X - %0.4X\r\n", data->pc, data->addr_abs);
    }
    
    return 0;
}

DEF_F(handle_CMP) {
    uint16_t temp = (uint16_t)data->regA - (uint16_t)data->fetched;
    cpu_set_flag(STATREG_C, data->regA >= data->fetched);
    setNZFlags16(temp);
    return 1;
}

DEF_F(handle_JSR) {
    data->pc--;
    
    //DOLOG("JSR PUSHING: %0.4X\r\n", data->pc);
    
    STACK_PUSH((data->pc >> 8) & 0x00FF);
    STACK_PUSH(data->pc & 0x00FF);
    
    data->pc = data->addr_abs;
    
    return 0;
}

DEF_F(handle_DEC) {
    uint16_t temp = data->fetched - 1;
    bus_write(data->addr_abs, temp & 0x00FF);
    setNZFlags16(temp);
    
    return 0;
}

DEF_F(handle_CPY) {
    uint16_t temp = (uint16_t)data->regY - (uint16_t)data->fetched;
    cpu_set_flag(STATREG_C, data->regY >= data->fetched);
    setNZFlags16(temp);
    return 0;
}

DEF_F(handle_CPX) {
    uint16_t temp = (uint16_t)data->regX - (uint16_t)data->fetched;
    cpu_set_flag(STATREG_C, data->regX >= data->fetched);
    setNZFlags16(temp);
    return 0;
}

DEF_F(handle_ORA) {
    data->regA |= data->fetched;
    setNZFlags(data->regA);
    return 1;
}

DEF_F(handle_PHA) {
    STACK_PUSH(data->regA);
    return 0;
}

DEF_F(handle_PHP) {
    STACK_PUSH(data->regStatus | STATREG_B | STATREG_U);
    cpu_set_flag(STATREG_B, 0);
    cpu_set_flag(STATREG_U, 0);
    
    return 0;
}

DEF_F(handle_PLA) {
    STACK_POP(data->regA);
    setNZFlags(data->regA);
    return 0;
}

DEF_F(handle_PLP) {
    STACK_POP(data->regStatus);
    cpu_set_flag(STATREG_U, 1);
    return 0;
}

DEF_F(handle_INC) {
    uint16_t temp = data->fetched + 1;
    bus_write(data->addr_abs, temp & 0x00FF);
    setNZFlags16(temp);
    return 0;
}

DEF_F(handle_INX) {
    data->regX++;
    setNZFlags(data->regX);
    return 0;
}

DEF_F(handle_INY) {
    data->regY++;
    setNZFlags(data->regY);
    return 0;
}

DEF_F(handle_ROL) {
    /*
    uint16_t temp = (uint16_t)(data->fetched << 1) | cpu_get_flag(STATREG_C);
    cpu_set_flag(STATREG_C, temp & 0x80);
    setNZFlags16(temp);
    
    if (opCode->address_mode == ACC) {
        cpu_set_flag(STATREG_C, data->regA & 0x80);
        data->regA = temp & 0x00FF;
    } else {
        bus_write(data->addr_abs, temp & 0x00FF);
    }
    */
    
    if (opCode->address_mode == ACC)
    {
        uint8_t prev_C = cpu_get_flag(STATREG_C);
        cpu_set_flag(STATREG_C, data->regA & 0x80);
        data->regA <<= 1;
        //If Rotating, set the bit-0 to the the previous carry
        data->regA = data->regA | (prev_C && (1));
        setNZFlags(data->regA);
    }
    else
    {
        uint8_t prev_C = cpu_get_flag(STATREG_C);
        data->fetched = bus_read(data->addr_abs);
        cpu_set_flag(STATREG_C, data->fetched & 0x80);
        data->fetched = data->fetched << 1 | (prev_C && (1));
        setNZFlags(data->fetched);
        bus_write(data->addr_abs, data->fetched);
    }
    
    return 0;
}

DEF_F(handle_ROR) {
    /*
    if (opCode->address_mode == ACC) {
        uint8_t prevC = cpu_get_flag(STATREG_C);
        cpu_set_flag(STATREG_C, data->regA & 0x80);
        data->regA <<= 1;
        data->regA = data->regA | prevC;
        setNZFlags(data->regA);
    } else {
        uint8_t prevC = cpu_get_flag(STATREG_C);
        uint8_t tmp = data->fetched;
        data->regA <<= 1;
        data->regA = data->regA | prevC;
        setNZFlags(data->regA);
        bus_write(data->addr_abs, tmp & 0x00FF);
    }
    */
    
    /*
    uint16_t temp = (uint16_t)(cpu_get_flag(STATREG_C) << 7) | (data->fetched >> 1);
    //uint16_t temp = (uint16_t)(data->fetched >> 1) | (uint16_t)(cpu_get_flag(STATREG_C) << 7);
    cpu_set_flag(STATREG_C, data->fetched & 0x80);
    setNZFlags16(temp);
    
    if (opCode->address_mode == ACC) {
        cpu_set_flag(STATREG_C, data->regA & 0x80);
        data->regA = temp & 0x00FF;
    } else {
        bus_write(data->addr_abs, temp & 0x00FF);
    }
    */
    
    if (opCode->address_mode == ACC)
    {
        //printf("LSR ACC1: %0.2X, %0.2X\r\n", r_A, f_C);
        uint8_t prev_C = cpu_get_flag(STATREG_C);
        cpu_set_flag(STATREG_C,  data->regA & 1);
        data->regA >>= 1;
        //printf("LSR ACC2: %0.2X, %0.2X\r\n", r_A, f_C);
        //If Rotating, set the bit-7 to the previous carry
        data->regA = data->regA | (prev_C && (1)) << 7;
        //printf("LSR ACC3: %0.2X, %0.2X\r\n", r_A, f_C);
        setNZFlags(data->regA);
    }
    else
    {
        //printf("LSR NOACC1: %0.2X, %0.2X\r\n", r_A, f_C);
        uint8_t prev_C = cpu_get_flag(STATREG_C);
        uint8_t operand = bus_read(data->addr_abs);
        cpu_set_flag(STATREG_C, operand & 1);
        operand = operand >> 1 | (prev_C && 1) << 7;
        //printf("LSR NOACC1: %0.2X, %0.2X, %0.4X, %0.4X\r\n", r_A, f_C, operand, location);
        setNZFlags(operand);
        bus_write(data->addr_abs, operand);
    }
    
    return 0;
}

DEF_F(handle_RTI) {
    STACK_POP(data->regStatus);
    data->regStatus &= ~STATREG_B;
    data->regStatus &= ~STATREG_U;
    
    uint16_t hi = 0;
    STACK_POP(hi);
    uint16_t lo = 0;
    STACK_POP(lo);
    
    data->pc = lo << 8;
    data->pc |= hi;
    
    //data->sp--;
    //data->pc = (uint16_t)bus_read(0x0100 + data->sp);
    //DOLOG("RTI: 01: %0.2X\r\n", data->pc);
    //data->sp--;
    //DOLOG("RTI: 02: %0.2X\r\n", (uint16_t)bus_read(0x0100 + data->sp) << 8);
    //data->pc |= (uint16_t)bus_read(0x0100 + data->sp) << 8;
    
    //DOLOG("RTI: 03: %0.2X %0.2X %0.2X %0.2X %0.2X %0.2X\r\n", data->sp + 2, data->sp + 1, data->sp, data->sp - 1, data->sp - 2, data->sp - 3);
    
    /*
    uint16_t hi = 0;
    uint16_t lo = 0;
    
    STACK_POP(hi);
    STACK_POP(lo);
    
    data->pc = lo << 8;
    data->pc |= hi;
    */
    //DOLOG("NEW PC: %0.2X\r\n", data->pc);
    
    return 0;
}

DEF_F(handle_RTS) {
    uint16_t hi = 0;
    STACK_POP(hi);
    uint16_t lo = 0;
    STACK_POP(lo);
    
    //DOLOG("HI: %0.2X\r\n", hi);
    //DOLOG("LO: %0.2X\r\n", lo);
    
    data->pc = lo << 8;
    //DOLOG("PC: %0.4X\r\n", data->pc);
    data->pc |= hi;
    
    data->pc++;
    
    //DOLOG("New PC: %0.4X\r\n", data->pc);
    
    return 0;
}

DEF_F(handle_ADC) {
    uint16_t temp = (uint16_t)data->regA + (uint16_t)(data->fetched) + (uint16_t)(cpu_get_flag(STATREG_C));
    cpu_set_flag(STATREG_C, temp > 255);
    cpu_set_flag(STATREG_Z, (temp & 0x00FF) == 0);
    cpu_set_flag(STATREG_V, (~((uint16_t)data->regA ^ (uint16_t)data->fetched) & ((uint16_t)data->regA ^ (uint16_t)temp)) & 0x0080);
    
    cpu_set_flag(STATREG_N, temp & 0x80);
    
    data->regA = temp & 0x00FF;
    
    return 1;
}

DEF_F(handle_SBC) {
    /*
    uint8_t beforeA = data->regA;
    
    uint16_t value = ((uint16_t)data->fetched) ^ 0x00FF;
    uint16_t temp = (uint16_t)data->regA + value + (uint16_t)cpu_get_flag(STATREG_C);
    uint16_t diff = data->regA - data->fetched - !cpu_get_flag(STATREG_C);
    
    if (0 == data->fetched && cpu_get_flag(STATREG_C) == 0) {
        temp = 0;
    }
    
    DOLOG("SBC 00: %0.4X %0.4X %0.4X, %0.4X - %0.4X\r\n", (uint16_t)data->fetched, value, (uint16_t)cpu_get_flag(STATREG_C), temp, diff);
    
    cpu_set_flag(STATREG_C, !(diff & 0x100));  //TODO: CHANGE BACK TO (temp & 0xFF00)...
    cpu_set_flag(STATREG_Z, (temp & 0x00FF) == 0);
    cpu_set_flag(STATREG_V, (temp ^ (uint16_t)data->regA) & (temp ^ value) & 0x0080);
    //cpu_set_flag(STATREG_Z, temp & 0x0080);
    
    data->regA = temp & 0x00FF;
    
    DOLOG("SBC 00: %0.2X %0.2X %0.2X\r\n", beforeA, data->regA, cpu_get_flag(STATREG_C));
    
    */
    
    //High carry means "no borrow", thus negate and subtract
    uint16_t subtrahend = bus_read(data->addr_abs);
    
    uint16_t diff = data->regA - subtrahend - !cpu_get_flag(STATREG_C);
    //if the ninth bit is 1, the resulting number is negative => borrow => low carry
    cpu_set_flag(STATREG_C, !(diff & 0x100));
    //Same as ADC, except instead of the subtrahend,
    //substitute with it's one complement
    cpu_set_flag(STATREG_V, (data->regA ^ diff) & (~subtrahend ^ diff) & 0x80);
    data->regA = diff;
    setNZFlags(diff);
    
    return 1;
}

DEF_F(handle_LSR) {
    cpu_set_flag(STATREG_C, data->fetched & 1);
    uint16_t temp = data->fetched >> 1;
    setNZFlags16(temp);
    
    
    if (opCode->address_mode == ACC) {
        data->regA = temp & 0x00FF;
    } else {
        bus_write(data->addr_abs, temp & 0x00FF);
    }
    
    return 0;
}

DEF_F(handle_ASL) {
    uint16_t temp = (uint16_t)data->fetched << 1;
    cpu_set_flag(STATREG_C, (temp & 0xFF00) > 0);
    setNZFlags16(temp);
    
    if (opCode->address_mode == ACC) {
        data->regA = temp & 0x00FF;
    } else {
        bus_write(data->addr_abs, temp & 0x00FF);
    }
    return 0;
}
int onBcc = 0;
DEF_F(handle_BCC) {
    onBcc = 1;
    if (!cpu_get_flag(STATREG_C)) {
        data->cycles++;
        data->addr_abs = data->pc + data->addr_rel;
        
        if ((data->addr_abs & 0xFF00) != (data->pc & 0xFF00)) {
            data->cycles++;
        }
        
        data->pc = data->addr_abs;
    }
    return 0;
}

DEF_F(handle_BCS) {
    if (cpu_get_flag(STATREG_C)) {
        data->cycles++;
        data->addr_abs = data->pc + data->addr_rel;
        
        if ((data->addr_abs & 0xFF00) != (data->pc & 0xFF00)) {
            data->cycles++;
        }
        
        data->pc = data->addr_abs;
        //DOLOG("SET PC 06: %0.4X - %0.4X\r\n", data->pc, data->addr_abs);
    }
    return 0;
}

DEF_F(handle_BMI) {
    if (cpu_get_flag(STATREG_N)) {
        data->cycles++;
        data->addr_abs = data->pc + data->addr_rel;
        
        if ((data->addr_abs & 0xFF00) != (data->pc & 0xFF00)) {
            data->cycles++;
        }
        
        data->pc = data->addr_abs;
    }
    
    return 0;
}

DEF_F(handle_EOR) {
    data->regA = data->regA ^ data->fetched;
    setNZFlags(data->regA);
    return 1;
}

DEF_F(handle_NOP) {
    return 0;
}

DEF_F(handle_BVC) {
    if (!cpu_get_flag(STATREG_V)) {
        data->cycles++;
        data->addr_abs = data->pc + data->addr_rel;
        
        
        if ((data->addr_abs & 0xFF00) != (data->pc & 0xFF00))
            data->cycles++;
        
        data->pc = data->addr_abs;
    }
    return 0;
}

DEF_F(handle_BVS) {
    if (cpu_get_flag(STATREG_V)) {
        data->cycles++;
        data->addr_abs = data->pc + data->addr_rel;
        
        if ((data->addr_abs & 0xFF00) != (data->pc & 0xFF00))
            data->cycles++;
        
        data->pc = data->addr_abs;
    }
    return 0;
}

/*

 DEF_F(handle_STY) {
 return 0;
 }
 uint8_t olc6502::BVS()
 {
 if (GetFlag(V) == 1)
 {
 cycles++;
 addr_abs = pc + addr_rel;
 
 if ((addr_abs & 0xFF00) != (pc & 0xFF00))
 cycles++;
 
 pc = addr_abs;
 }
 return 0;
 }
 */


void cpu_nmi() {
    cpu_interrupt(0xff);
}

void cpu_interrupt(uint8_t type) {
    struct CpuData *data = &cpuData;
    
    if (type == 0) { //BRK
        data->pc++;
    }
    
    STACK_PUSH((data->pc >> 8) & 0x00FF);
    STACK_PUSH(data->pc & 0x00FF);
    
    //cpu_set_flag(STATREG_B, 0);
    //cpu_set_flag(STATREG_U, 1);
    //cpu_set_flag(STATREG_I, 1);
    
    uint8_t sr = data->regStatus;
    sr |= STATREG_U;
    
    if (type == 0) {
        sr |= STATREG_B;
    }
    
    STACK_PUSH(sr);
    
    cpu_set_flag(STATREG_I, 1);
    
    data->addr_abs = 0xFFFA;
    
    if (type == 0) {
        data->addr_abs = 0xFFFE;
    } else if (type == 0xff) {
        
    }
    
    uint16_t lo = bus_read(data->addr_abs + 0);
    uint16_t hi = bus_read(data->addr_abs + 1);
    data->pc = (hi << 8) | lo;
    data->cycles += 7;
    
    
    /*
     write(0x0100 + stkp, (pc >> 8) & 0x00FF);
     stkp--;
     write(0x0100 + stkp, pc & 0x00FF);
     stkp--;
     
     SetFlag(B, 0);
     SetFlag(U, 1);
     SetFlag(I, 1);
     write(0x0100 + stkp, status);
     stkp--;
     
     addr_abs = 0xFFFA;
     uint16_t lo = read(addr_abs + 0);
     uint16_t hi = read(addr_abs + 1);
     pc = (hi << 8) | lo;
     
     cycles = 8;
     */
}


static void instruction_init() {
    initialized = 1;
    
    inst_add_handler("JMP", handle_JMP);
    inst_add_handler("BRK", handle_BRK);
    inst_add_handler("SEI", handle_SEI);
    inst_add_handler("SEC", handle_SEC);
    inst_add_handler("SED", handle_SED);
    inst_add_handler("CLC", handle_CLC);
    inst_add_handler("CLI", handle_CLI);
    inst_add_handler("CLV", handle_CLV);
    inst_add_handler("CLD", handle_CLD);
    inst_add_handler("LDA", handle_LDA);
    inst_add_handler("LDX", handle_LDX);
    inst_add_handler("LDY", handle_LDY);
    inst_add_handler("STA", handle_STA);
    inst_add_handler("STX", handle_STX);
    inst_add_handler("STY", handle_STY);
    inst_add_handler("AND", handle_AND);
    inst_add_handler("BEQ", handle_BEQ);
    inst_add_handler("BCS", handle_BCS);
    inst_add_handler("JSR", handle_JSR);
    
    inst_add_handler("DEC", handle_DEC);
    inst_add_handler("DEX", handle_DEX);
    inst_add_handler("DEY", handle_DEY);
    inst_add_handler("TAX", handle_TAX);
    inst_add_handler("TAY", handle_TAY);
    inst_add_handler("TSX", handle_TSX);
    inst_add_handler("TXA", handle_TXA);
    inst_add_handler("TXS", handle_TXS);
    inst_add_handler("TYA", handle_TYA);
    inst_add_handler("BIT", handle_BIT);
    inst_add_handler("BPL", handle_BPL);
    inst_add_handler("BNE", handle_BNE);
    inst_add_handler("CMP", handle_CMP);
    inst_add_handler("CPY", handle_CPY);
    inst_add_handler("CPX", handle_CPX);
    inst_add_handler("ORA", handle_ORA);
    inst_add_handler("PHA", handle_PHA);
    inst_add_handler("PHP", handle_PHP);
    inst_add_handler("PLA", handle_PLA);
    inst_add_handler("PLP", handle_PLP);
    inst_add_handler("INC", handle_INC);
    inst_add_handler("INX", handle_INX);
    inst_add_handler("INY", handle_INY);
    inst_add_handler("ROL", handle_ROL);
    inst_add_handler("ROR", handle_ROR);
    inst_add_handler("RTI", handle_RTI);
    inst_add_handler("RTS", handle_RTS);
    inst_add_handler("ADC", handle_ADC);
    inst_add_handler("SBC", handle_SBC);
    inst_add_handler("LSR", handle_LSR);
    inst_add_handler("ASL", handle_ASL);
    inst_add_handler("BCC", handle_BCC);
    inst_add_handler("BMI", handle_BMI);
    inst_add_handler("EOR", handle_EOR);
    inst_add_handler("NOP", handle_NOP);
    inst_add_handler("BVC", handle_BVC);
    inst_add_handler("BVS", handle_BVS);

}

uint8_t instruction_execute(struct CpuData *data, struct OpCode *opCode) {
    if (!initialized) {
        instruction_init();
    }
    
    handle_inst f = inst_get_handler(opCode->name);
    
    if (f) {
        //DOLOG("INST: %s\r\n", opCode->name);
        return f(data, opCode);
    } else {
        DOLOG("INST: %s\r\n", opCode->name);
        
        for (int i=data->pc - 10; i < data->pc + 10; i++) {
            printf("%0.4X %0.2X\r\n", i, bus_read(i));
        }
        
        return 0;
    }
    
    return 0;
}

