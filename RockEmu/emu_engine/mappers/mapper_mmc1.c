//
//  mapper_mmc1.c
//  Emu6502
//
//  Created by Rocky Pulley on 8/28/19.
//  Copyright © 2019 Rocky Pulley. All rights reserved.
//

#include "mapper.h"
#include "ppu_bus.h"

static int writeCounter = 0;
static int modePRG = 3;
static int modeCHR = 0;
static int mirroring = 0;

static bool usesCharRam = false;

static uint8_t regTmp = 0;
static uint8_t regPrg = 0;
static uint8_t regChr0 = 0;
static uint8_t regChr1 = 0;

static uint8_t *prgBanks[2];
static uint8_t *chrBanks[2];
static uint8_t *chrRam;

static uint32_t chrSize = 0;
static uint32_t prgSize = 0;

int dumpedBank = 0;
extern int counter;

uint8_t mapper_mm1_read_prg(uint16_t addr) {
    
    if (addr < 0xC000) {
        return *(prgBanks[0] + (addr & 0x3FFF));
    }
    
    uint8_t bd = *(prgBanks[1] + (addr & 0x3FFF));

    return bd;
}

void mapper_mm1_calc_prg() {
    struct NesData *data = cartridge_get_data();
    
    uint8_t *old0 = prgBanks[0];
    uint8_t *old1 = prgBanks[1];
    
    if (modePRG <= 1) {
        prgBanks[0] = data->prgData + (0x4000 * (regPrg & ~1));
        prgBanks[1] = prgBanks[0] + 0x4000;
    } else if (modePRG == 2) {
        prgBanks[0] = data->prgData;
        prgBanks[1] = prgBanks[0] + (0x4000 * regPrg);
    } else {
        prgBanks[0] = data->prgData + (0x4000 * regPrg);
        prgBanks[1] = data->prgData + (prgSize - 0x4000);
    }
    
    if (prgBanks[0] != old0 || prgBanks[1] != old1) {

        //printf("%0.8X PRGOLD: %0.8X %0.8X  - PRGNEW: %0.8X %0.8X\r\n", counter, old0, old1, prgBanks[0], prgBanks[1]);
    }
}

void mapper_mm1_write_prg(uint16_t addr, uint8_t value) {

    uint8_t resetNotSet = (!(value & 0x80));
    
    if (!resetNotSet) {
        regTmp = 0;
        writeCounter = 0;
        modePRG = 3;
        mapper_mm1_calc_prg();
        return;
    }
    
    regTmp = (regTmp >> 1) | ((value & 1) << 4);
    writeCounter++;
    
    if (writeCounter != 5) {
        return;
    }
    
    struct NesData *data = cartridge_get_data();
    
    if (addr <= 0x9fff) {
        
        switch(regTmp & 0x3) {
            case 0: mapper_get_current()->mirroringType = OneScreenLower; break;
            case 1: mapper_get_current()->mirroringType = OneScreenHigher; break;
            case 2: mapper_get_current()->mirroringType = Vertical; break;
            case 3: mapper_get_current()->mirroringType = Horizontal; break;
        }
        
        ppu_bus_update_mirroring();
        
        modeCHR = (regTmp & 0x10) >> 4;
        modePRG = (regTmp & 0xC) >> 2;
        mapper_mm1_calc_prg();
        
        if (modeCHR == 0) {
            chrBanks[0] = data->chrData + (0x1000 * (regChr0 | 1));
            chrBanks[1] = chrBanks[0] + 0x1000;
        } else {
            chrBanks[0] = data->chrData + (0x1000 * regChr0);
            chrBanks[1] = data->chrData + (0x1000 * regChr1);
        }
    } else if (addr <= 0xbfff) {
        regChr0 = regTmp;
        chrBanks[0] = data->chrData + (0x1000 * (regTmp | (1 - modeCHR)));
        
        if (modeCHR == 0) {
            chrBanks[1] = chrBanks[0] + 0x1000;
        }
    } else if (addr <= 0xdfff) {
        regChr1 = regTmp;
        
        if (modeCHR == 1) {
            chrBanks[1] = data->chrData + (0x1000 * regTmp);
        }
    } else {
        //TODO PRG-RAM...
        
        regTmp &= 0xf;
        regPrg = regTmp;
        
        mapper_mm1_calc_prg();
    }
    
    writeCounter = 0;
    regTmp = 0;
    
}

uint8_t *mapper_mm1_get_page(uint16_t addr) {
    return (addr < 0xC000) ? (prgBanks[0] + (addr & 0x3FFF)) : (prgBanks[1] + (addr & 0x3FFF));
}

uint8_t mapper_mm1_read_chr(uint16_t addr) {
    if (usesCharRam) {
        return chrRam[addr];
    } else if (addr < 0x1000) {
        return *(chrBanks[0] + addr);
    } else {
        return *(chrBanks[1] + (addr & 0xFFF));
    }
}

void mapper_mm1_write_chr(uint16_t addr, uint8_t value) {
    if (usesCharRam) {
        chrRam[addr] = value;
    }
}

struct Mapper *mapper_mm1_create(struct NesData *data) {
    struct Mapper *m = (struct Mapper *)malloc(sizeof(struct Mapper));
    
    prgSize = data->header.prgSize * 0x4000;
    chrSize = data->header.chrSize * 0x2000;
    
    if (chrSize) {
        usesCharRam = false;
        chrBanks[0] = data->chrData;
        chrBanks[1] = data->chrData + (0x1000 * regChr1);
    } else {
        usesCharRam = true;
        chrRam = (uint8_t *)malloc(0x2000);
    }
    
    prgBanks[0] = data->prgData;
    prgBanks[1] = data->prgData + prgSize - 0x4000;
    
    if (*(prgBanks[0] + 0x30C) == 0x20) {
        //printf("SOMETHING GOOD\r\n");
        //printf("SOMETHING GOOD\r\n");
    }
    
    if (*(prgBanks[0] + 0x30C) == 0x66) {
        //printf("SOMETHING WORNG\r\n");
    }
    
    DOLOG("PRGBANKS: %0.16X, %0.16X\r\n", prgBanks[0], prgBanks[1]);
    
    DOLOG("DATA START: %0.8X\r\n", data->prgData);
    DOLOG("DATA END  : %0.8X\r\n", data->prgData + prgSize);
    
    m->readPRG = mapper_mm1_read_prg;
    m->writePRG = mapper_mm1_write_prg;
    m->readCHR = mapper_mm1_read_chr;
    m->writeCHR = mapper_mm1_write_chr;
    m->getPagePointer = mapper_mm1_get_page;
    
    m->mirroringType = data->header.mainFlags.flags6 & 1;
    
    return m;
}
