//
//  ppu.h
//  Emu6502
//
//  Created by Rocky Pulley on 8/26/19.
//  Copyright © 2019 Rocky Pulley. All rights reserved.
//

#ifndef ppu_h
#define ppu_h

#include "common.h"

extern const int PPUCTRL_V;
extern const int PPUCTRL_P;
extern const int PPUCTRL_H;
extern const int PPUCTRL_B;
extern const int PPUCTRL_S;
extern const int PPUCTRL_I;
extern const int PPUCTRL_N1;
extern const int PPUCTRL_N2;

extern const int PPUSTAT_V;
extern const int PPUSTAT_S;
extern const int PPUSTAT_O;

extern const int PPU_STATE_PRE ;
extern const int PPU_STATE_RENDER ;
extern const int PPU_STATE_VBLANK ;
extern const int PPU_STATE_POST ;

extern const int PPUMASK_B;
extern const int PPUMASK_G;
extern const int PPUMASK_R;
extern const int PPUMASK_SE;
extern const int PPUMASK_BE;
extern const int PPUMASK_SLE;
extern const int PPUMASK_BLE;
extern const int PPUMASK_GS;

struct PpuData {
    uint8_t regStatus;
    uint8_t regControl;
    uint8_t regMask;
    uint8_t pipelineState;
    uint16_t addIncrement;
    uint8_t dataBuffer;
    uint8_t spriteDataAddress;
    
    bool evenFrame;
    bool firstWrite;
    
    uint16_t dataAddress;
    uint16_t tempAddress;
    uint16_t cycle;
    uint16_t scanLine;
    uint16_t fineXScroll;
    
    uint8_t scanLineSprites[64];
    uint8_t scanLineSpritesSize;
    uint8_t *spriteMemory;
    
    uint32_t **pictureBuffer;
    uint32_t curFrame;
};

void ppu_init(void);
void ppu_clock(void);

struct PpuData *ppu_data_pointer();
struct PpuData ppu_data(void);

uint8_t ppu_read_status(void);
uint8_t ppu_read_control(void);
uint8_t ppu_read_mask(void);
uint8_t ppu_read_data(void);

void ppu_set_data_addr(uint16_t addr);
uint8_t ppu_oam_read_addr(uint16_t addr);
uint8_t ppu_oam_read();
void ppu_oam_data_write(uint8_t data);
void ppu_set_data(uint8_t data);
void ppu_set_oam_addr(uint8_t addr);
void ppu_oam_write_addr(uint16_t addr, uint8_t value);
void ppu_oam_write(uint8_t value);
void ppu_set_status(uint8_t data);

void ppu_set_control(uint8_t value);
void ppu_set_mask(uint8_t value);

void ppu_do_dma(uint8_t *page);
void ppu_set_scroll(uint8_t scroll);

#endif /* ppu_h */
