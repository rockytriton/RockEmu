//
//  bus.c
//  Emu6502
//
//  Created by Rocky Pulley on 8/25/19.
//  Copyright © 2019 Rocky Pulley. All rights reserved.
//

#include "bus.h"
#include "ppu.h"
#include "controller.h"
#include "mapper.h"
#include "cpu.h"
#include "ppu.h"

uint8_t prg_ram[64 * 1024];
uint8_t ram[0x800];

uint8_t busInit = 0;

const int IOR_PPU_CTRL = 0x2000;

enum IORegisters
{
    PPUCTRL = 0x2000,
    PPUMASK,
    PPUSTATUS,
    OAMADDR,
    OAMDATA,
    PPUSCROL,
    PPUADDR,
    PPUDATA,
    OAMDMA = 0x4014,
    JOY1 = 0x4016,
    JOY2 = 0x4017,
};

void bus_init() {
    for (int i=0; i<64*1024; i++) {
        prg_ram[i] = 0;
    }
    for (int i=0; i<0x800; i++) {
        ram[i] = 0;
    }
    
    busInit = 1;
}

uint8_t *bus_read_ram(uint16_t address) {
    return ram + (address & 0x7FFF);
}

void bus_write(uint16_t address, uint8_t value) {
    if (address < 0x2000) {
        ram[address & 0x7FF] = value;
        //printf("wrote ram at %0.4X = %0.2X\r\n", address & 0x7FF, value);
    }
    else if ((address >= 0x4000 && address <= 0x4013) || address == 0x4015 || address == 0x4017) {
        //APU stuff
    }
    else if (address == 0x4014) {
        //DMA
        cpu_skip_dma_cycles();
        uint8_t *pp = bus_page_pointer(value);
        ppu_do_dma(pp);
        return;
    }
    else if (address == 0x4016) {
        //JOY1
        controller_write(value);
        return;
    }
    else if (address >= 0x8000) {
        //prg_ram[address] = value;
        mapper_get_current()->writePRG(address, value);
    } else if (address < 0x4000 && (address & 0x2007) == OAMDMA) {
        return ppu_oam_write(value);
    } else if (address < 0x4000 && (address & 0x2007) == OAMDATA) {
        return ppu_oam_data_write(value);
    } else if (address < 0x4000 && (address & 0x2007)  == PPUADDR) {
        printf("CALLING PPUADDRSET: %0.4X, %0.4X, %0.2X\r\n", address, address & 0x2007, value);
        return ppu_set_data_addr(value);
    } else if (address < 0x4000 && (address & 0x2007)  == PPUDATA) {
        return ppu_set_data(value);
    } else if (address < 0x4000 && (address & 0x2007)  == OAMADDR) {
        return ppu_set_oam_addr(value);
    } else if (address < 0x4000 && (address & 0x2007) == PPUCTRL) {
        return ppu_set_control(value);
    } else if (address < 0x4000 && (address & 0x2007) == PPUMASK) {
        return ppu_set_mask(value);
    } else if (address < 0x4000 && (address & 0x2007) == PPUSTATUS) {
        //ppu_set_status(value);
        return;
    } else if (address < 0x4000 && (address & 0x2007) == PPUSCROL) {
        ppu_set_scroll(value);
        return;
    } else if (address < 0x4000 && (address & 0x2007) == JOY1) {
        controller_write(value);
        return;
    } else {
        prg_ram[address] = value;
        return;
    }
}

uint8_t bus_read(uint16_t address) {
    if (address < 0x2000) {
        return ram[address & 0x7FF];
    }
    else if ((address >= 0x4000 && address <= 0x4013) || address == 0x4015) {
        //APU stuff
        return 0;
    }
    else if (address == 0x4014) {
        //DMA
        return 0;
    }
    else if (address == 0x4016) {
        //JOY1
        return controller_read(0);
    }
    else if (address == 0x4017) {
        //JOY2
        return controller_read(1);
    }
    else if (address < 0x4000 && address < 0x4000 && (address & 0x2007) == PPUSTATUS) {
        uint8_t s = ppu_read_status();
        
        if (s != 0) {
            return s;
        }
        
        return s;
    } else if (address < 0x4000 && address < 0x4000 && (address & 0x2007) == PPUDATA) {
        return ppu_read_data();
    } else if (address < 0x4000 && address < 0x4000 && (address & 0x2007) == OAMDATA) {
        return ppu_oam_read();
    } else if (address < 0x4000 && address < 0x4000 && (address & 0x2007) == PPUCTRL) {
        return ppu_read_control();
    } else if (address < 0x4000 && address < 0x4000 && (address & 0x2007) == PPUMASK) {
        return ppu_read_mask();
    } else if (address < 0x4000 && address < 0x4000 && (address & 0x2007) == OAMDMA) {
        return ppu_oam_read();
    } else if (address >= 0x8000) {
        return mapper_get_current()->readPRG(address);
        //return prg_ram[address];
    } else {
        
    }
    
    return prg_ram[address];
}

void bus_load(uint8_t *data, uint16_t address, uint16_t size) {
    memcpy(prg_ram + address, data, size);
}

uint8_t *bus_page_pointer(uint8_t page) {
    //printf("GETTING BYTE PAGE PTR: %0.2X\r\n", page);
    uint16_t addr = page << 8;
    
    //printf("GETTING BYTE PAGE ADDR: %0.4X\r\n", addr);
    
    if (addr < 0x2000) {
        //printf("LOCATION @ %0.4X = %0.8X\r\n", addr, &ram[addr & 0x7FF]);
        return &ram[addr & 0x7FF];
    } else if (addr >= 0x6000 && addr < 0x8000) {
        //ext ram
        return 0;
    }
    
    return 0;
}

