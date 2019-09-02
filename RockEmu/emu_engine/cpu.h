//
//  cpu.h
//  Emu6502
//
//  Created by Rocky Pulley on 8/25/19.
//  Copyright © 2019 Rocky Pulley. All rights reserved.
//

#ifndef cpu_h
#define cpu_h

#include "common.h"
#include "op_codes.h"

#define HISTORY_SIZE 20

extern const int STATREG_C;
extern const int STATREG_Z;
extern const int STATREG_I;
extern const int STATREG_D;
extern const int STATREG_B;
extern const int STATREG_U;
extern const int STATREG_V;
extern const int STATREG_N;

struct HistoryEntry {
    struct OpCode opCode;
    uint16_t pc;
    uint16_t abs_addr;
    uint16_t addr_rel;
    uint8_t fetched;
};

struct CpuData {
    uint8_t regA;
    uint8_t regX;
    uint8_t regY;
    uint16_t pc;
    uint16_t sp;
    uint16_t cycles;
    
    uint8_t regStatus;
    
    uint8_t flagCarry;
    uint8_t flagZero;
    uint8_t flagDI;
    uint8_t flagDec;
    uint8_t flagBreak;
    uint8_t flagOverflow;
    uint8_t flagNegative;
    
    uint8_t fetched;
    uint16_t addr_abs;
    uint16_t addr_rel;
    
    uint8_t paused;
    uint8_t stepping;
    
    uint32_t clock_delta;
};

struct NesData *cpu_open_file(const char *filename);
void cpu_run(void);

struct OpCode cpu_current_opcode(void);
uint8_t cpu_started(void);

uint16_t cpu_pc(void);

struct CpuData cpu_data(void);

void cpu_set_flag(uint8_t n, uint8_t v);
uint8_t cpu_get_flag(uint8_t n);

void cpu_pause(void);
void cpu_resume(void);
void cpu_step(void);

void cpu_clock_delta(int32_t n);

struct HistoryEntry *cpu_get_history();
struct HistoryEntry cpu_get_history_item(int n);

int32_t cpu_get_history_size();

void cpu_nmi(void);
void cpu_interrupt(uint8_t type);
void cpu_skip_dma_cycles(void);

#endif /* cpu_h */
