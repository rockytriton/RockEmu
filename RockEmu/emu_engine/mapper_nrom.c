//
//  mapper_nrom.c
//  Emu6502
//
//  Created by Rocky Pulley on 8/28/19.
//  Copyright © 2019 Rocky Pulley. All rights reserved.
//

#include "mapper.h"

static uint16_t prgSize;
static uint16_t chrSize;
static bool oneBank;
static bool usesCharRam;
static uint8_t *chrRam;

uint8_t mapper_nrom_read_prg(uint16_t addr) {
    return *(oneBank ? cartridge_get_data()->prgData + ((addr - 0x8000) & 0x3FFF) : cartridge_get_data()->prgData + (addr - 0x8000));
}

void mapper_nrom_write_prg(uint16_t addr, uint8_t value) {
    
}

uint8_t *mapper_nrom_get_page(uint16_t addr) {
    return cartridge_get_data()->prgData + (oneBank ? (addr - 0x8000) & 0x3fff : addr - 0x8000);
}

uint8_t mapper_nrom_read_chr(uint16_t addr) {
    return usesCharRam ? chrRam[addr] : *(cartridge_get_data()->chrData + addr);
}

void mapper_nrom_write_chr(uint16_t addr, uint8_t value) {
    if (usesCharRam) {
        chrRam[addr] = value;
    }
}

struct Mapper *mapper_nrom_create(struct NesData *data) {
    struct Mapper *m = (struct Mapper *)malloc(sizeof(struct Mapper));
    
    prgSize = data->header.prgSize * 0x4000;
    chrSize = data->header.chrSize * 0x2000;
    
    oneBank = prgSize == 0x4000;
    
    if (chrSize == 0) {
        usesCharRam = true;
        chrRam = (uint8_t *)malloc(0x2000);
    } else {
        usesCharRam = false;
    }
    
    m->readPRG = mapper_nrom_read_prg;
    m->writePRG = mapper_nrom_write_prg;
    m->readCHR = mapper_nrom_read_chr;
    m->writeCHR = mapper_nrom_write_chr;
    m->getPagePointer = mapper_nrom_get_page;
    
    m->mirroringType = data->header.mainFlags.Bit.mt;
    
    return m;
}
