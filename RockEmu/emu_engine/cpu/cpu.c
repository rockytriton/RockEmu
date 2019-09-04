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
#include "timing.h"

#include <unistd.h>
#include <string.h>
#include <time.h>
#include <inttypes.h>
#include <sys/timeb.h>


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
    cpuData.clock_delta = 5000;
    counter = 0;
    
    for (int i=0; i<HISTORY_SIZE; i++) {
        history[i].opCode.name = 0;
        //memset(history[i].opCode.name, 0, 4);
    }
    
    cpu_set_flag(STATREG_U, 1);
    
    data = 0;
    
    
}

struct NesData * cpu_open_file(const char *filename) {
    
    printf("Opening file %s\r\n", filename);
    ppu_init();
    cpu_init();
    printf("Init\r\n");
    data = cartridge_load(filename);
    printf("Loaded\r\n");
    ppu_bus_update_mirroring();
    bus_load(data->prgData, 0x8000, MIN(data->header.prgSize, 2) * 0x4000);
    
    cpuData.addr_abs = 0xFFFC;
    uint16_t lo = bus_read(cpuData.addr_abs + 0);
    uint16_t hi = bus_read(cpuData.addr_abs + 1);
    
    // Set it
    cpuData.pc = (hi << 8) | lo;
    
    DOLOG("RESET AT: %0.4X\r\n", cpuData.pc);
    
    printf("Starting\r\n");
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

    }
}
extern int onBcc;

int lastOpCode = 0;

void cpu_clock() {

    if (cpuData.cycles != 0) {
        cpuData.cycles--;
        return;
    }
    
    onBcc = 0;
    
    uint16_t oldPc = cpuData.pc;
    
    struct OpCode opCode = cpu_current_opcode();
    cpuData.pc++;
    
    lastOpCode = opCode.code;
    cpuData.cycles = opCode.cycles;
    
    uint8_t amc = addr_mode_fetch(&cpuData, &opCode);
    
    cpuData.fetched = cpuData.regA;
    
    fetch_abs_addr(opCode);
    
    /*
    for (int i=0; i<HISTORY_SIZE; i++) {
        history[i] = history[i + 1];
    }
    
    history[HISTORY_SIZE - 1].opCode = opCode;
    history[HISTORY_SIZE - 1].abs_addr = cpuData.addr_abs;
    history[HISTORY_SIZE - 1].addr_rel = cpuData.addr_rel;
    history[HISTORY_SIZE - 1].fetched = cpuData.fetched;
    history[HISTORY_SIZE - 1].pc = oldPc;
    */
    
    //DOLOG("ADDED HISTORY ITEM: %0.16X\r\n", history[HISTORY_SIZE - 1].opCode.name);
    
#if 0
    printf("%0.8X %0.4X %s (%0.2X) $%0.4X, %0.4X = %0.2X - A: %0.2X X: %0.2X Y: %0.2X SP: %0.2X CYC: %4d  - P: %0.2X ADDRINFO\r\n", counter,
           oldPc, opCode.name, opCode.code, cpuData.addr_abs, cpuData.addr_rel, cpuData.fetched,
           cpuData.regA, cpuData.regX, cpuData.regY, cpuData.sp, ppuData.cycle, cpuData.regStatus);
#endif
    counter++;
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

#define NES_CLOCK_HZ (21477272ll / 4)
#define NES_CLOCK_MS (21477272ll / 4) / 1000 / 1000


unsigned long prevMs = 0;

#define BILLION  1000000000.0;
//#define BILLION  1000000000L

bool wasPause = false;

void cpu_run() {
    timer_init();
    
    printf("RUNNING CPU\r\n");
    
    //usleep(1);
    
    int cycleCount = 0;
    
    while(true) {
        
        //useconds_t t = 1 * cpuData.clock_delta;
        //usleep(t);
        
        if (!started) {
            //printf("not start\r\n");
            usleep(1500);
            continue;
        }
        
        if (cpuData.paused) {
            wasPause = true;
            
            if (cpuData.stepping) {
                cpuData.stepping = 0;
            } else {
                printf("paused\r\n");
                usleep(1500);
                continue;
            }
            
        }
        if (wasPause) {
            printf("Resuming...\r\n");
            wasPause = false;
        }
        
        timer_update();
        
        //printf("Cycle Count: %d - (%0.2X)\r\n", cycleCount, lastOpCode);
        cycleCount = 0;
        
        struct PpuData *p = ppu_data_pointer();
        
        struct timeb start, end;
        int curFrame = 0;
        ftime(&start);
        
        
        while(true) { //timer_loop()) {
            
            ppu_clock();
            ppu_clock();
            ppu_clock();
            
            //startProfile();
            cpu_clock();
            //endProfile();
            
            //timer_cycle();
            
            
            if (curFrame < p->curFrame) {
                ftime(&end);
                int diff = (int) (1000.0 * (end.time - start.time) + (end.millitm - start.millitm));
                
                usleep(300 - diff);
                curFrame = p->curFrame;
                ftime(&start);
            }
            
            cycleCount++;
            
            if (cpuData.paused) {
                break;
            }
        }
        
        usleep(1000);
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
    uint16_t ss = 513; // + cpuData.cycles;
    ss += ((cpuData.cycles - 1) & 1);
    cpuData.cycles += ss;
}
