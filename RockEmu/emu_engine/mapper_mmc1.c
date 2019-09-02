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

static struct Mapper *thisMapper;

int dumpedBank = 0;
extern int counter;

uint8_t mapper_mm1_read_prg(uint16_t addr) {
    
    if (*(prgBanks[0] + 0x30C) == 0x20) {
        //printf("SOMETHING GOOD\r\n");
        //printf("SOMETHING GOOD\r\n");
    }
    
    if (*(prgBanks[0] + 0x30C) == 0x66) {
        //printf("SOMETHING WORNG\r\n");
    }
    
    if (addr == 0x830C) {
        
        
        
        {
            //printf("LOCATIONOF PRGBANK1: %0.8X\r\n", prgBanks[0]);
            for (int i=0; i<0x4000; i++) {
                //printf("%0.2X ", *(prgBanks[0] + (i & 0x3FFF)));
                
                if ((i % 16) == 0) {
                    //printf("\r\n");
                }
                
                dumpedBank = 1;
            }
        }
        
        DOLOG("PRGBANKS: %0.16X, %0.16X - %0.4X - %0.4X\r\n", prgBanks[0], prgBanks[1], addr, addr & 0x3FFF);
        
        uint8_t *p = prgBanks[0];
        uint8_t bt = p[addr & 0x3FFF];
        
        int a = addr & 0x3FFF;
        
        for (int i=a - 10; i < a + 10; i++) {
            //printf("%0.4X %0.2X", i, p[i]);
            
            if (i == 0x30C) {
                //printf("*");
            }
            
            //printf("\r\n");
        }
        
        
        
        if (*(prgBanks[0] + 0x30C) == 0x66) {
            //printf("SOMETHING WORNG\r\n");
        } else {
            //printf("NOPE: %0.2X (%0.4X)\r\n", *(prgBanks[0] + 0x30C), addr & 0x3FFF);
            //printf("NOPE: %0.2X (%0.4X)\r\n", *(p + 0x30C), addr & 0x3FFF);
        }
        
        //printf("SUMFIN FUKY\r\n");
    }
    
    if (addr < 0xC000) {
        return *(prgBanks[0] + (addr & 0x3FFF));
    }
    
    uint8_t *p = prgBanks[1];
    uint32_t t1 = p;
    uint16_t m = addr & 0x3FFF;
    p += m;
    uint32_t t2 = p;
    
    uint8_t b = *p;
    
    uint8_t bd = *(prgBanks[1] + (addr & 0x3FFF));
    
    if (counter > 100 && (addr == 0xC191 && bd == 0)) {
        ///printf("HIT BAD SPOT: %0.4X - %0.2X\r\n", addr, bd);
        dumpedBank = 0;
        //return bd;
    }
    
    if (!dumpedBank) {
        //printf("LOCATIONOF PRGBANK1: %0.8X\r\n", prgBanks[0]);
    for (int i=0; i<0x4000; i++) {
        //printf("%0.2X ", *(prgBanks[0] + (i & 0x3FFF)));
        
        if ((i % 16) == 0) {
            //printf("\r\n");
        }
        
        dumpedBank = 1;
    }
    }
    if (counter > 100 && (addr == 0xC191 && bd == 0)) {
        //printf("CHZ\r\n");
    }
    
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
        prgBanks[1] = prgBanks[0] + 0x4000 * regPrg;
    } else {
        if (regPrg != 0) {
        ///printf("OK REG IS %0.2X = %d\r\n", regPrg, regPrg);
        //printf("OLD ADDR: %0.8X\r\n", data->prgData);
        //printf("ADD ADDR: %0.8X\r\n", (0x4000 * regPrg));
        //printf("NEW ADDR: %0.8X\r\n", data->prgData + (0x4000 * regPrg));
        }
        prgBanks[0] = data->prgData + (0x4000 * regPrg);
        prgBanks[1] = data->prgData + (prgSize - 0x4000);
    }
    
    if (prgBanks[0] != old0 || prgBanks[1] != old1) {

        //printf("%0.8X PRGOLD: %0.8X %0.8X  - PRGNEW: %0.8X %0.8X\r\n", counter, old0, old1, prgBanks[0], prgBanks[1]);
    }
}

void mapper_mm1_write_prg(uint16_t addr, uint8_t value) {
    
    if (addr == 0xFFFF) {
        //printf("OK: %0.2X\r\n", value);
    }
    
    uint8_t resetNotSet = (!(value & 0x80));
    
    if (!resetNotSet) {
        //printf("NOT RESET NOT SET\r\n");
        regTmp = 0;
        writeCounter = 0;
        modePRG = 3;
        mapper_mm1_calc_prg();
        return;
    }
    
    regTmp = (regTmp >> 1) | ((value & 1) << 4);
    //printf("REGTMP: %0.2X\r\n", regTmp);
    writeCounter++;
    
    if (writeCounter != 5) {
        //printf("NOT COUNTER: %d\r\n", writeCounter);
        return;
    }
    
    struct NesData *data = cartridge_get_data();
    
    if (addr <= 0x9fff) {
        
        switch(regTmp) {
            case 0: thisMapper->mirroringType = OneScreenLower; break;
            case 1: thisMapper->mirroringType = OneScreenHigher; break;
            case 2: thisMapper->mirroringType = Vertical; break;
            case 3: thisMapper->mirroringType = Horizontal; break;
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
            chrBanks[1] = chrBanks[0] + (0x1000 * regChr1);
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
            chrBanks[1] = chrBanks[0] + (0x1000 * regTmp);
        }
    } else {
        //TODO PRG-RAM...
        
        regTmp &= 0xf;
        regPrg = regTmp;
        
        ///printf("AND REGTMP %0.2X\r\n", regTmp);
        
        if (regPrg != 0) {
            //printf("REGPRG: %0.2X\r\n", regPrg);
        }
        
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
        return *(chrBanks[0] + (addr & 0xFFF));
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
        chrBanks[1] = data->chrData;
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
    
    m->mirroringType = Horizontal;
    
    return m;
}
