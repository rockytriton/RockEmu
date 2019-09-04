//
//  mapper_mmc1.c
//  Emu6502
//
//  Created by Rocky Pulley on 8/28/19.
//  Copyright © 2019 Rocky Pulley. All rights reserved.
//

#include "mapper.h"
#include "ppu_bus.h"
#include "cpu.h"

static int writeCounter = 0;
static int modePRG = 3;
static int modeCHR = 0;
static int mirroring = 0;

static bool writeEnable = false;

static uint8_t regTmp = 0;
static uint8_t regPrg = 0;
static uint8_t regChr0 = 0;
static uint8_t regChr1 = 0;

union {
    uint8_t val;
    struct {
        char bank : 3;
        char unused : 3;
        char prgRomMode : 1;
        char chrInversion : 1;
    } Bit;
} bankSelect = {0};

static uint32_t chrBanksSize = 8;
static uint32_t prgBankSize = 4;
static uint8_t *mmc3PrgBanks[16];
static uint8_t *prgBanks[16];
static uint8_t *chrBanks[8];
static uint8_t *mmc3ChrBanks[8];
static uint8_t *chrRam;

static uint8_t prgRam[0x2000];
static uint8_t fourScreenRam[0x1000];
static bool useFourScreenRam;
static bool lastA12 = false;
static uint8_t irqCounter = 0;
static uint8_t irqLatch = 0;
static bool irqEnabled = false;
static uint8_t bankValues[8];
static bool protectRam = false;

static uint32_t chrSize = 0;
static uint32_t prgSize = 0;

static struct Mapper *thisMapper;

static int dumpedBank = 0;
extern int counter;

uint8_t *getChrBank(uint8_t n) {
    return chrBanks[n % chrBanksSize];
}

uint8_t *getPrgBank(uint8_t n) {
    return prgBanks[n % prgBankSize];
}

uint32_t getPrgBankSize() {
    return prgBankSize;
}

void mapper_mmc3_update_banks() {

    if (bankSelect.Bit.prgRomMode == 0) {
        //printf("SWITCHING BANKS\r\n");
        mmc3PrgBanks[0] = getPrgBank(bankValues[6]);
        mmc3PrgBanks[1] = getPrgBank(bankValues[7]);
        mmc3PrgBanks[2] = getPrgBank(getPrgBankSize() - 2);
        mmc3PrgBanks[3] = getPrgBank(getPrgBankSize() - 1);
    } else {
        //printf("SWITCHING BANKS2\r\n");
        mmc3PrgBanks[0] = getPrgBank(getPrgBankSize() - 2);
        mmc3PrgBanks[1] = getPrgBank(bankValues[7]);
        mmc3PrgBanks[2] = getPrgBank(bankValues[6]);
        mmc3PrgBanks[3] = getPrgBank(getPrgBankSize() - 1);
    }
    
    if (bankSelect.val & 0x80) {
        mmc3ChrBanks[0] = getChrBank(bankValues[0] & 0xFE);
        mmc3ChrBanks[1] = getChrBank(bankValues[0] | 1);
        mmc3ChrBanks[2] = getChrBank(bankValues[1] & 0xFE);
        mmc3ChrBanks[3] = getChrBank(bankValues[1] | 1);
        mmc3ChrBanks[4] = getChrBank(bankValues[2]);
        mmc3ChrBanks[5] = getChrBank(bankValues[3]);
        mmc3ChrBanks[6] = getChrBank(bankValues[4]);
        mmc3ChrBanks[7] = getChrBank(bankValues[5]);
    } else {
        mmc3ChrBanks[4] = getChrBank(bankValues[0] & 0xFE);
        mmc3ChrBanks[5] = getChrBank(bankValues[0] | 1);
        mmc3ChrBanks[6] = getChrBank(bankValues[1] & 0xFE);
        mmc3ChrBanks[7] = getChrBank(bankValues[1] | 1);
        mmc3ChrBanks[0] = getChrBank(bankValues[2]);
        mmc3ChrBanks[1] = getChrBank(bankValues[3]);
        mmc3ChrBanks[2] = getChrBank(bankValues[4]);
        mmc3ChrBanks[3] = getChrBank(bankValues[5]);
    }
}

uint8_t mapper_mmc3_read_prg(uint16_t addr) {
    if (addr < 0x6000) {
        return 0; // Nothing in "Expansion ROM"
    } else if (addr < 0x8000) {
        return protectRam ? 0 : prgRam[addr - 0x6000];
    } else {
        //printf("Reading Bank: %0.4X - %0.4X[%0.2X] = %0.2X\r\n", addr, (addr - 0x8000) / 0x2000, addr % 0x2000, mmc3PrgBanks[(addr - 0x8000) / 0x2000][addr % 0x2000]);
        
        
        if (addr % 0x2000 == 0x1F6B) {
            //printf("\r\n\r\n");
        for (int i=0; i<0x2000; i++) {
            //printf("%0.2X ", mmc3PrgBanks[3][i]);
            
            if ((i % 32) == 0) {
                //printf("\r\n");
            }
        }
        
        //printf("\r\n");
        }
        
        uint8_t b = mmc3PrgBanks[(addr - 0x8000) / 0x2000][addr % 0x2000];
        
        return b;
    }
}

void irqService() {
    printf("NOTHING TO DO SO FAR FOR IRQ\r\n");
    cpu_interrupt(1);
}

void mapper_mmc3_write_prg(uint16_t addr, uint8_t value) {
    
    if (addr < 0x6000) {
        return; // nothing to expansion ROM.
    } else if (addr < 0x8000) {
        if (!protectRam) {
            prgRam[addr - 0x6000] = value;
        }
        
        return;
    }
    
    //printf("ADDR: %0.4X - %0.4X\r\n", addr, addr & 0xE001);
    
    switch(addr & 0xE001) {
        case 0x8000: {
            bankSelect.val = value;
            mapper_mmc3_update_banks();
            return;
        }
        case 0x8001: {
            //printf("SETTING SEL: %0.2X, %0.2X\r\n", bankSelect.val, bankSelect.val & 0x7);
            bankValues[bankSelect.val & 0x7] = value;
            mapper_mmc3_update_banks();
            return;
        }
        case 0xA000: {
            mirroring = value & 1;
            mapper_mmc3_update_banks();
            return;
        }
        case 0xA001: {
            protectRam = !(value & 8);
            writeEnable = (value & 4) != 0;
            mapper_mmc3_update_banks();
            return;
        }
        case 0xC000: {
            irqLatch = value;
            return;
        }
        case 0xC001: {
            irqCounter = 0;
            return;
        }
        case 0xE000: {
            irqService();
            irqEnabled = false;
            return;
        }
        case 0xE001: {
            irqEnabled = true;
            return;
        }
    }
}

uint8_t *mapper_mmc3_get_page(uint16_t addr) {
    if (addr < 0x6000) {
        return 0; // Nothing in "Expansion ROM"
    } else if (addr < 0x8000) {
        return protectRam ? 0 : &prgRam[addr - 0x6000];
    } else {
        return &mmc3PrgBanks[(addr - 0x8000) / 0x2000][addr % 0x2000];
    }
}

uint8_t mapper_mmc3_read_chr(uint16_t addr) {
    bool next = (addr & (1 << 12)) != 0;
    
    //printf("CHR READ: %0.2X, %0.2X\r\n", addr, next);
    
    if (!lastA12 && next) {
        if (irqCounter == 0) {
            irqCounter = irqLatch;
        } else {
            irqCounter--;
        }
        
        //TODO IRQ STUFFS
        //printf("TODO IRQ\r\n");
        cpu_interrupt(1);
    }
    
    lastA12 = next;
    
    return mmc3PrgBanks[addr / 0x400][addr % 0x400];
}

void mapper_mmc3_write_chr(uint16_t addr, uint8_t value) {
    
    //printf("CHR WRITE: %0.4X, %0.4X, %0.2X\r\n", addr, addr / 0x400, value);
    chrBanks[addr / 0x400][addr % 0x400] = value;
}

struct Mapper *mapper_mmc3_create(struct NesData *data) {
    struct Mapper *m = (struct Mapper *)malloc(sizeof(struct Mapper));
    
    prgSize = data->header.prgSize * 0x4000;
    chrSize = data->header.chrSize * 0x2000;
    
    chrBanksSize = data->header.chrSize * 8;
    prgBankSize = data->header.prgSize * 2;
    
    if (chrSize) {
        for (int i=0; i<data->header.chrSize * 8; i++) {
            chrBanks[i] = data->chrData + (i * 0x400);
            mmc3ChrBanks[i] = data->chrData + (i * 0x400);
        }
    } else {
        //printf("NO CHR DATA\r\n");
    }
    
    for (int i=0; i<data->header.prgSize * 2; i++) {
        prgBanks[i] = data->prgData + (i * 0x2000);
        mmc3PrgBanks[i] = data->prgData + (i * 0x2000);
        //printf("LOADING PRG BANK %d %0.8X\r\n", i, mmc3PrgBanks[i]);
    }
    
    mmc3PrgBanks[2] = prgBanks[prgBankSize - 2];
    mmc3PrgBanks[3] = prgBanks[prgBankSize - 1];
    
    DOLOG("DATA START: %0.8X\r\n", data->prgData);
    DOLOG("DATA END  : %0.8X\r\n", data->prgData + prgSize);
    
    for (int i=0; i<0x2000; i++) {
        //printf("%0.2X ", mmc3PrgBanks[3][i]);
        
        if ((i % 32) == 0) {
            //printf("\r\n");
        }
    }
    
    //printf("\r\n");
    
    m->readPRG = mapper_mmc3_read_prg;
    m->writePRG = mapper_mmc3_write_prg;
    m->readCHR = mapper_mmc3_read_chr;
    m->writeCHR = mapper_mmc3_write_chr;
    m->getPagePointer = mapper_mmc3_get_page;
    m->mirroringType = data->header.mainFlags.flags6 & 1;
    
    memset(bankValues, 0, sizeof(bankValues));
    
    useFourScreenRam = (data->header.mainFlags.flags6 & 0x08) != 0;
    
    if (useFourScreenRam) {
        m->mirroringType = FourScreen;
    }
    
    return m;
}
