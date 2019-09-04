//
//  mapper_mmc3_2.c
//  RockEmu
//
//  Created by Rocky Pulley on 9/4/19.
//  Copyright © 2019 Rocky Pulley. All rights reserved.
//

#include "mapper.h"
#include "ppu_bus.h"

byte prgRam[0x2000];
byte *prgBank[4];
byte *chrBank[8];

byte bankValues[8];

uint16_t bankSelectAddress;

byte regBankSelect = 0;
byte regBankData = 0;
byte regMirroring = 0;
byte regRamProtect = 0;
byte regIrqLatch = 0;
byte regIrqReload = 0;
byte regIrqDisable = 0;
byte regIrqEnable = 0;

static void onBankSelect(uint16_t addr) {
    byte bank = regBankSelect & 7;
    bankValues[bank] = regBankData;
    
    switch(bank) {
        case 0: {
            
        } break;
    }
}

static void mapper_mmc3_write_prg(uint16_t addr, byte value) {
    if (addr < 0xA000 && (addr % 2) == 0) {
        bankSelectAddress = addr;
        regBankSelect = value;
    } else if (addr < 0xA000 && (addr % 2) == 1) {
        regBankData = value;
        onBankSelect(addr);
    } else if (addr < 0xC000 && (addr % 2) == 0) {
        regMirroring = value;
    } else if (addr < 0xC000 && (addr % 2) == 1) {
        regRamProtect = value;
    } else if (addr < 0xE000 && (addr % 2) == 0) {
        regIrqLatch = value;
    } else if (addr < 0xE000 && (addr % 2) == 1) {
        regIrqReload = value;
    } else if (addr <= 0xFFFF && (addr % 2) == 0) {
        regIrqDisable = value;
        regIrqEnable = !(regIrqDisable);
    } else if (addr <= 0xFFFF && (addr % 2) == 1) {
        regIrqEnable = value;
        regIrqDisable = !(regIrqEnable);
    }
}
