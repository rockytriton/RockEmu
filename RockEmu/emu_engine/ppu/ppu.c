//
//  ppu.c
//  Emu6502
//
//  Created by Rocky Pulley on 8/26/19.
//  Copyright © 2019 Rocky Pulley. All rights reserved.
//

#include "ppu.h"
#include "bus.h"
#include "pipeline.h"
#include "ppu_bus.h"
#include <errno.h>

const int AttributeOffset = 0x3C0;

const int PPUCTRL_V = 1 << 7; //NMI enable
const int PPUCTRL_P = 1 << 6; //PPU m/s
const int PPUCTRL_H = 1 << 5; //height
const int PPUCTRL_B = 1 << 4; //back tile select
const int PPUCTRL_S = 1 << 3; //sprite tile select
const int PPUCTRL_I = 1 << 2; //inc mode
const int PPUCTRL_N1 = 1 << 1; //namestable select
const int PPUCTRL_N2 = 1 << 0;

const int PPUSTAT_V = 1 << 7; //vblank
const int PPUSTAT_S = 1 << 6; //sprite 0 hit
const int PPUSTAT_O = 1 << 5; //sprite overflow

const int PPUMASK_B = 1 << 7;
const int PPUMASK_G = 1 << 6;
const int PPUMASK_R = 1 << 5;
const int PPUMASK_SE = 1 << 4;
const int PPUMASK_BE = 1 << 3;
const int PPUMASK_SLE = 1 << 2;
const int PPUMASK_BLE = 1 << 1;
const int PPUMASK_GS = 1 << 0;

const int PPU_STATE_PRE = 0;
const int PPU_STATE_RENDER = 1;
const int PPU_STATE_VBLANK = 2;
const int PPU_STATE_POST = 3;

extern const int VisibleScanlines;
extern const int ScanlineVisibleDots;

struct PpuData ppuData;

struct PpuData ppu_data(void) {
    return ppuData;
}

struct PpuData *ppu_data_pointer() {
    return &ppuData;
}

/*
 PPUCTRL    $2000    VPHB SINN    NMI enable (V), PPU master/slave (P), sprite height (H), background tile select (B), sprite tile select (S), increment mode (I), nametable select (NN)
 PPUMASK    $2001    BGRs bMmG    color emphasis (BGR), sprite enable (s), background enable (b), sprite left column enable (M), background left column enable (m), greyscale (G)
 PPUSTATUS    $2002    VSO- ----    vblank (V), sprite 0 hit (S), sprite overflow (O); read resets write pair for $2005/$2006
 */

void ppu_init() {
    ppuData.regStatus = 0;
    ppuData.regControl = 0;
    ppuData.regMask = 0;
    ppuData.pipelineState = PPU_STATE_PRE;
    ppuData.cycle = 0;
    ppuData.evenFrame = 1;
    ppuData.tempAddress = 0;
    ppuData.dataAddress = 0;
    ppuData.scanLine = 0;
    ppuData.firstWrite = true;
    ppuData.addIncrement = 1;
    ppuData.dataBuffer = 0;
    ppuData.fineXScroll = 0;
    ppuData.scanLineSpritesSize = 0;
    ppuData.spriteMemory = (uint8_t *)malloc(64 * 4);
    memset(ppuData.spriteMemory, 0, 64 * 4);
    ppuData.curFrame = 261;
    
    ppuData.pictureBuffer = (uint32_t **)malloc(sizeof(uint32_t *) * ScanlineVisibleDots);
    
    for (int i=0; i<ScanlineVisibleDots; i++) {
        ppuData.pictureBuffer[i] = (uint32_t *)malloc(sizeof(uint32_t) * VisibleScanlines);
        memset(ppuData.pictureBuffer[i], 0, sizeof(uint32_t) * VisibleScanlines);
    }
    
    ppuData.spriteDataAddress = 0;
    
    
    SET_FLAG(ppuData.regMask, PPUMASK_SE, 1);
    SET_FLAG(ppuData.regMask, PPUMASK_BE, 1);
    SET_FLAG(ppuData.regMask, PPUMASK_SLE, 1);
    SET_FLAG(ppuData.regMask, PPUMASK_BLE, 1);
    
    ppu_bus_init();
    
    SET_FLAG(ppuData.regStatus, PPUSTAT_O, 1);
    SET_FLAG(ppuData.regStatus, PPUSTAT_V, 1);
}

int ppuCount = 0;

int doneDma = 0;
extern int onBcc;

int dumped = 0;
extern int counter;

void ppu_clock() {
    if (doneDma) {
        doneDma++;
        
        if (doneDma > 3000) {
            doneDma = 0;
        }
    }
    
    if (!dumped && counter > 0x00202C63) {
        FILE *fp = fopen("/Users/rockypulley/Documents/spram.rnes.txt", "w");
        
        int err = errno;
        //printf("ERR: %d, %0.4X\r\n", errno, errno);
        
        for (int y=0; y<ScanlineVisibleDots; y++) {
            for (int x=0; x<VisibleScanlines; x++) {
                fprintf(fp, "%0.8X ", ppuData.pictureBuffer[y][x]);
            }
            fprintf(fp, "\r\n");
        }
        
        fclose(fp);
        
        dumped = 1;
    }
    
    //ppu_render_new(&ppuData);
    
    
    switch(ppuData.pipelineState) {
        case PPU_STATE_PRE:
            ppu_prerender(&ppuData);
            break;
        case PPU_STATE_RENDER:
            ppu_render(&ppuData);
            break;
        case PPU_STATE_POST:
            ppu_postrender(&ppuData);
            break;
        case PPU_STATE_VBLANK:
            ppu_vblank(&ppuData);
            break;
    }
    
    //printf("PPU: %0.2X, %0.2X\r\n", ppuData.regStatus, ppuData.regControl);
    
    if (ppuData.cycle > 300) {
        //printf("OK");
    }
    
    ppuData.cycle++;
}

void ppu_oam_data_write(uint8_t data) {
    //printf("OAM DATA WRITE %0.4X, %0.2X\r\n", ppuData.spriteDataAddress, data);
    //ppu_set_data(ppuData.dataAddress);
    //ppuData.dataAddress += ppuData.addIncrement;
    ppuData.spriteMemory[ppuData.spriteDataAddress++] = data;
}

uint8_t ppu_read_status(void) {
    uint8_t temp = ppuData.regStatus & 0xE0;
    temp |= ppuData.regControl & 0x1F;
    
    ppuData.firstWrite = true;
    SET_FLAG(ppuData.regStatus, PPUSTAT_V, 0);
    
    if (ppuData.scanLine == 241 && ppuData.cycle == 0)
        temp &= ~0x80; // set V to 0 in the retval
    
    //printf("Read status: %0.2X\r\n", temp);
    
    return temp;
}

uint8_t ppu_read_control(void) {
    return ppuData.regControl;
}

uint8_t ppu_read_mask(void) {
    return ppuData.regMask;
}

uint8_t ppu_read_data(void) {
    uint8_t data = ppu_bus_read(ppuData.dataAddress);
    bool bound = ppuData.dataAddress == 0x3FFF;
    
    ppuData.dataAddress += ppuData.addIncrement;
    //printf("PPUREADINC: %0.4X = %0.2X, %0.2X\r\n", ppuData.dataAddress, data, ppuData.dataBuffer);
    
    if (ppuData.dataAddress < 0x3F00) {
        uint8_t tmp = ppuData.dataBuffer;
        ppuData.dataBuffer = data;
        data = tmp;
        //printf("Swapped: %0.2X\r\n", data);
    }
    
    if (ppuData.dataAddress > 0x3FFF) {
        
    }
    
    return data;
}

void ppu_set_data_addr(uint16_t addr) {
    //printf("PPUDATAADDR SET: %0.4X\r\n", addr);
    
    if (ppuData.firstWrite) {
        ppuData.tempAddress &= ~0xff00;
        ppuData.tempAddress |= (addr & 0x3F) << 8;
        ppuData.firstWrite = false;
        //printf("FIRST WRITE: %0.4X\r\n", ppuData.tempAddress);
    } else {
        ppuData.tempAddress &= ~0xff;
        ppuData.tempAddress |= addr;
        ppuData.dataAddress = ppuData.tempAddress;
        //printf("PPUDATA TEMP: %0.4X\r\n", ppuData.dataAddress);
        ppuData.firstWrite = true;
    }
}

uint8_t ppu_oam_read_addr(uint16_t addr) {
    return ppuData.spriteMemory[addr];
}

uint8_t ppu_oam_read() {
    //printf("OAMDATA READ: %0.4X - %0.2X\r\n", ppuData.spriteDataAddress, ppu_oam_read_addr(ppuData.spriteDataAddress));
    return ppu_oam_read_addr(ppuData.spriteDataAddress);
}

void ppu_set_status(uint8_t data) {
    ppuData.regStatus = data;
}

void ppu_set_data(uint8_t data) {
    //printf("WRITEPPUDATA: %0.4X = %0.2X\r\n", ppuData.dataAddress, data);
    ppu_bus_write(ppuData.dataAddress, data);
    ppuData.dataAddress += ppuData.addIncrement;
}

void ppu_set_oam_addr(uint8_t addr) {
    //DOLOG("WRITEPPUDATA SET: %0.4X\r\n", addr);
    ppuData.spriteDataAddress = addr;
}

void ppu_oam_write_addr(uint16_t addr, uint8_t value) {
    //printf("WRITE OAM: %0.4X, %0.2X\r\n", addr, value);
    ppuData.spriteMemory[addr] = value;
}

void ppu_oam_write(uint8_t value) {
    ppu_oam_write_addr(ppuData.spriteDataAddress++, value);
}

void ppu_set_control(uint8_t value) {
    ppuData.regControl = value;
    
    if (value & 0x04) {
        ppuData.addIncrement = 0x20;
    } else {
        ppuData.addIncrement = 1;
    }
    
    ppuData.tempAddress &= ~0xC00;
    ppuData.tempAddress |= (value & 0x3) << 10;
    
}

void ppu_set_mask(uint8_t value) {
    ppuData.regMask = value;
}

void ppu_do_dma(uint8_t *page) {
    doneDma = 1;
    /*
    //printf("DO DMA: %0.4X\r\n", ppuData.spriteDataAddress);
    
    for (int i=0; i<(256 - ppuData.spriteDataAddress); i++) {
        //printf("%0.2X ", page[i]);
        
        if ((i % 32) == 0) {
            //printf("\r\n");
        }
    }
    
    //printf("\r\n\r\n");
    */
    memcpy(ppuData.spriteMemory + ppuData.spriteDataAddress, page, 256 - ppuData.spriteDataAddress);
    
    if (ppuData.spriteDataAddress) {
        memcpy(ppuData.spriteMemory, page + (256 - ppuData.spriteDataAddress), ppuData.spriteDataAddress);
    }
    
    /*
     
     std::memcpy(m_spriteMemory.data() + m_spriteDataAddress, page_ptr, 256 - m_spriteDataAddress);
     if (m_spriteDataAddress)
     std::memcpy(m_spriteMemory.data(), page_ptr + (256 - m_spriteDataAddress), m_spriteDataAddress);
     */
}

void ppu_set_scroll(uint8_t scroll) {
    if (ppuData.firstWrite)
    {
        ppuData.tempAddress &= ~0x1f;
        ppuData.tempAddress |= (scroll >> 3) & 0x1f;
        ppuData.fineXScroll = scroll & 0x7;
        ppuData.firstWrite = false;
    }
    else
    {
        ppuData.tempAddress &= ~0x73e0;
        ppuData.tempAddress |= ((scroll & 0x7) << 12) |
        ((scroll & 0xf8) << 2);
        ppuData.firstWrite = true;
    }
}
