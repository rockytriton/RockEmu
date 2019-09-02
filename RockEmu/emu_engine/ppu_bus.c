//
//  ppu_bus.c
//  Emu6502
//
//  Created by Rocky Pulley on 8/26/19.
//  Copyright © 2019 Rocky Pulley. All rights reserved.
//

#include "ppu_bus.h"
#include "mapper.h"

uint8_t *ppu_bus_ram = 0;
uint8_t *ppu_bus_palette = 0;
uint16_t nameTables[] = {0, 0, 0, 0};

void ppu_bus_init() {
    memset(nameTables, 0, 4);
    ppu_bus_ram = (uint8_t *)malloc(0x800);
    ppu_bus_palette = (uint8_t *)malloc(0x20);
    
    memset(ppu_bus_ram, 0, 0x800);
    memset(ppu_bus_palette, 0, 0x20);
}

uint8_t ppu_bus_read(uint16_t addr) {
    if (addr < 0x2000) {
        return mapper_get_current()->readCHR(addr);
    } else if (addr < 0x3EFF) {
        uint16_t index = addr & 0x3FF;
        
        if (addr < 0x2400) {
            //NT0
            uint8_t b = ppu_bus_ram[nameTables[0] + index];
            
            if (addr == 0x23C0 && b != 0) {
                //printf("READ VALUE 0: %0.2X - %0.8X\r\n", b, nameTables[0]);
                //printf("OK\r\n");
            }
            
            return b;
        } else if (addr < 0x2800) {
            //NT1
            uint8_t b =  ppu_bus_ram[nameTables[1] + index];
            
            if (addr == 0x23C0 && b != 0) {
                //printf("READ VALUE 1: %0.2X\r\n", b);
            }
            
            return b;
        } else if (addr < 0x2c00) {
            //NT1
            uint8_t b =  ppu_bus_ram[nameTables[2] + index];
            
            if (addr == 0x23C0 && b != 0) {
                //printf("READ VALUE 2: %0.2X\r\n", b);
            }
            
            return b;
        } else {
            //NT1
            uint8_t b =  ppu_bus_ram[nameTables[3] + index];
            
            if (addr == 0x23C0 && b != 0) {
                //printf("READ VALUE 3: %0.2X\r\n", b);
            }
            
            return b;
        }
    } else if (addr < 0x3FFF) {
        return ppu_bus_palette[addr & 0x1F];
    }
    
    return 0;
}

uint8_t ppu_bus_read_palette(uint8_t addr) {
    return ppu_bus_palette[addr];
}

void ppu_bus_write(uint16_t addr, uint8_t value) {
    if (addr < 0x2000) {
        mapper_get_current()->writeCHR(addr, value);
        return;
    } else if (addr < 0x3EFF) {
        uint16_t index = addr & 0x3FF;
        
        if (index == 0x3C0 && value != 0) {
            //printf("WRITING TO 3C0: %0.2X\r\n", value);
        }
        
        if (value == 0x24) {
            //printf("WROTE 24 at %0.4X - %0.4X\r\n", addr, index);
            //printf("WROTE 24!\r\n");
        }
        
        if (addr < 0x2400) {
            //NT0
            ppu_bus_ram[nameTables[0] + index] = value;
            
            if (addr == 0x23C0 && value != 0) {
                //printf("WRITING VALUE: %0.2X\r\n", value);
                uint8_t b = ppu_bus_read(addr);
                //printf("REREAD VALUE: %0.2X - %0.8X\r\n", b, nameTables[0]);
                b = 0;
            }
        } else if (addr < 0x2800) {
            //NT1
            ppu_bus_ram[nameTables[1] + index] = value;
        } else if (addr < 0x2c00) {
            //NT1
            ppu_bus_ram[nameTables[2] + index] = value;
        } else {
            //NT1
            ppu_bus_ram[nameTables[3] + index] = value;
        }
    } else if (addr < 0x3FFF) {
        ppu_bus_palette[addr == 0x3F10 ? 0 : addr & 0x1F] = value;
    } else {
        return;
    }
}

void ppu_bus_update_mirroring() {
    memset(nameTables, 0, 8);
    
    switch(mapper_get_current()->mirroringType){
        case Horizontal:
            nameTables[0] = nameTables[1] = 0;
            nameTables[2] = nameTables[3] = 0x400;
            break;
        case Vertical:
            nameTables[0] = nameTables[2] = 0;
            nameTables[1] = nameTables[3] = 0x400;
            break;
        case OneScreenLower:
            nameTables[0] = nameTables[1] = 0;
            nameTables[2] = nameTables[3] = 0;
            break;
        case OneScreenHigher:
            nameTables[0] = nameTables[1] = 0x400;
            nameTables[2] = nameTables[3] = 0x400;
            break;
        default:
            nameTables[0] = nameTables[1] = 0;
            nameTables[2] = nameTables[3] = 0;
            break;
    }
}

