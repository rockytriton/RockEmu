//
//  cpu.c
//  Emu6502
//
//  Created by Rocky Pulley on 8/25/19.
//  Copyright © 2019 Rocky Pulley. All rights reserved.
//

#include "cpu.h"
#include "cartridge.h"
#include "bus.h"
#include "addr_modes.h"
#include "instructions.h"
#include "ppu.h"

#include <unistd.h>
#include <string.h>

const int STATREG_C = 1 << 0;
const int STATREG_Z = 1 << 1;
const int STATREG_I = 1 << 2;
const int STATREG_D = 1 << 3;
const int STATREG_B = 1 << 4;
const int STATREG_U = 1 << 5;
const int STATREG_V = 1 << 6;
const int STATREG_N = 1 << 7;

static uint8_t started = 0;
int counter = 0;

struct NesData *data = 0;
struct CpuData cpuData;

struct OpCode opCodeHistory[] = {{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}};

struct HistoryEntry history[HISTORY_SIZE];

int32_t cpu_get_history_size() {
    return HISTORY_SIZE;
}

struct HistoryEntry *cpu_get_history() {
    return history;
}

struct HistoryEntry cpu_get_history_item(int n) {
    return history[n];
}

void cpu_init() {
    cpuData.pc = 0x8000;
    cpuData.sp = 0xFD;
    cpuData.flagBreak = 0;
    cpuData.flagCarry = 0;
    cpuData.flagDI = 0;
    cpuData.flagDec = 0;
    cpuData.flagZero = 0;
    cpuData.flagOverflow = 0;
    cpuData.flagNegative = 0;
    cpuData.regA = 0;
    cpuData.regX = 0;
    cpuData.regY = 0;
    cpuData.fetched = 0;
    cpuData.addr_abs = 0;
    cpuData.addr_rel = 0;
    cpuData.regStatus = 0;
    cpuData.paused = 0;
    cpuData.stepping = 0;
    cpuData.clock_delta = 1;
    counter = 0;
    
    for (int i=0; i<HISTORY_SIZE; i++) {
        history[i].opCode.name = 0;
        //memset(history[i].opCode.name, 0, 4);
    }
    
    cpu_set_flag(STATREG_U, 1);
    
    data = 0;
    
    
}

struct NesData * cpu_open_file(const char *filename) {
    
    
    ppu_init();
    cpu_init();
    data = cartridge_load(filename);
    ppu_bus_update_mirroring();
    bus_load(data->prgData, 0x8000, MIN(data->header.prgSize, 2) * 0x4000);
    
    cpuData.addr_abs = 0xFFFC;
    uint16_t lo = bus_read(cpuData.addr_abs + 0);
    uint16_t hi = bus_read(cpuData.addr_abs + 1);
    
    // Set it
    cpuData.pc = (hi << 8) | lo;
    
    DOLOG("RESET AT: %0.4X\r\n", cpuData.pc);
    
    started = 1;
    return data;
}

extern struct PpuData ppuData;

static void fetch_abs_addr(struct OpCode opCode) {
    const char *fetch_op_codes = "ADC SBC AND ASL BIT CMP CPX CPY DEC EOR INC LDA LDX LDY LSR ORA ROL ROR";
    
    if (!strstr(fetch_op_codes, opCode.name)) {
        return;
    }
    
    if (opCode.address_mode != IMP && opCode.address_mode != ACC) {
        cpuData.fetched = bus_read(cpuData.addr_abs);
        
        if (opCode.code == 0xF9) {
            DOLOG("SBC FETCHED VALUE: %0.2X from %0.4X\r\n", cpuData.fetched, cpuData.addr_abs);
        }
    }
}
extern int onBcc;

void cpu_clock() {
    //char path[1024];
    //getcwd(path, sizeof(path));
    //puts(path);
    
    //DOLOG("PC: %0.4X\r\n", cpuData.pc);
    if (onBcc) {
        DOLOG("ON BCC CLOCK: %d\r\n", cpuData.cycles);
    }
    if (cpuData.cycles != 0) {
        cpuData.cycles--;
        return;
    }
    
    onBcc = 0;
    
    uint16_t oldPc = cpuData.pc;
    
    struct OpCode opCode = cpu_current_opcode();
    cpuData.pc++;
    
    cpuData.cycles = opCode.cycles;
    
    uint8_t amc = addr_mode_fetch(&cpuData, &opCode);
    
    cpuData.fetched = cpuData.regA;
    
    fetch_abs_addr(opCode);
    
    for (int i=0; i<HISTORY_SIZE; i++) {
        history[i] = history[i + 1];
    }
    
    history[HISTORY_SIZE - 1].opCode = opCode;
    history[HISTORY_SIZE - 1].abs_addr = cpuData.addr_abs;
    history[HISTORY_SIZE - 1].addr_rel = cpuData.addr_rel;
    history[HISTORY_SIZE - 1].fetched = cpuData.fetched;
    history[HISTORY_SIZE - 1].pc = oldPc;
    
    //DOLOG("ADDED HISTORY ITEM: %0.16X\r\n", history[HISTORY_SIZE - 1].opCode.name);
    
#if 0 //ISLOGGING
    printf("%0.8X %0.4X %s (%0.2X) $%0.4X, %0.4X = %0.2X - A: %0.2X X: %0.2X Y: %0.2X SP: %0.2X CYC: %4d  - P: %0.2X ADDRINFO\r\n", counter++,
           oldPc, opCode.name, opCode.code, cpuData.addr_abs, cpuData.addr_rel, cpuData.fetched,
           cpuData.regA, cpuData.regX, cpuData.regY, cpuData.sp, ppuData.cycle, cpuData.regStatus);
#endif
    //counter++;
    uint8_t imc = instruction_execute(&cpuData, &opCode);
    
    cpuData.cycles += (amc & imc);
    
    //DOLOG("CYCLES: %4d\r\n", cpuData.cycles);
    
    cpuData.cycles--;
}

struct OpCode cpu_current_opcode() {
    uint8_t code = bus_read(cpuData.pc);
    
    opCodeLookup[code].code = code;
    return opCodeLookup[code];
}

uint8_t cpu_started() {
    return started;
}

void cpu_run() {
    while(true) {
        //useconds_t t = 1 * cpuData.clock_delta;
        //usleep(t);
        
        if (!started) {
            continue;
        }
        
        if (cpuData.paused) {
            if (cpuData.stepping) {
                cpuData.stepping = 0;
            } else {
                continue;
            }
        }
        
        ppu_clock();
        ppu_clock();
        ppu_clock();
        cpu_clock();
    }
}

uint16_t cpu_pc() {
    return cpuData.pc;
}

struct CpuData cpu_data() {
    return cpuData;
}

void cpu_set_flag(uint8_t n, uint8_t v) {
    if (v) {
        cpuData.regStatus |= n;
    } else {
        cpuData.regStatus &= ~n;
    }
}

uint8_t cpu_get_flag(uint8_t n) {
    return (cpuData.regStatus & n) > 0 ? 1 : 0;
}

void cpu_pause(void) {
    cpuData.paused = 1;
}

void cpu_resume(void) {
    cpuData.paused = 0;
    cpuData.stepping = 0;
}

void cpu_step(void) {
    cpuData.paused = 1;
    cpuData.stepping = 1;
}

void cpu_clock_delta(int32_t n) {
    if (n < 0 && cpuData.clock_delta <= n) {
        cpuData.clock_delta /= 2;
    } else if ((int32_t)(cpuData.clock_delta + n) > 0) {
        cpuData.clock_delta += n;
    }
}

void cpu_skip_dma_cycles() {
    DOLOG("SKIPCYCLES: 01: %4d\r\n", cpuData.cycles);
    uint16_t ss = 513; // + cpuData.cycles;
    DOLOG("SKIPCYCLES: 02: %4d\r\n", ss);
    ss += ((cpuData.cycles - 1) & 1);
    DOLOG("SKIPCYCLES: 03: %4d\r\n", ss);
    cpuData.cycles += ss;
    DOLOG("SKIPCYCLES: 04: %4d\r\n", cpuData.cycles);
}
