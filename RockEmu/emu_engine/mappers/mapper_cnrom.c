//
//  mapper_cnrom.c
//  RockEmu
//
//  Created by Rocky Pulley on 9/3/19.
//  Copyright © 2019 Rocky Pulley. All rights reserved.
//

#include "mapper.h"
#include "ppu_bus.h"


static uint16_t prgSize;
static uint16_t chrSize;
static bool oneBank;

static uint8_t selectChr = 0;

uint8_t mapper_cnrom_read_prg(uint16_t addr) {
    return *(oneBank ? cartridge_get_data()->prgData + ((addr - 0x8000) & 0x3FFF) : cartridge_get_data()->prgData + (addr - 0x8000));
}

void mapper_cnrom_write_prg(uint16_t addr, uint8_t value) {
    selectChr = value & 3;
}

uint8_t *mapper_cnrom_get_page(uint16_t addr) {
    return cartridge_get_data()->prgData + (oneBank ? (addr - 0x8000) & 0x3fff : addr - 0x8000);
}

uint8_t mapper_cnrom_read_chr(uint16_t addr) {
    return *(cartridge_get_data()->chrData + (addr | (selectChr << 13)));
}

void mapper_cnrom_write_chr(uint16_t addr, uint8_t value) {
    //no write this type.
}

struct Mapper *mapper_cnrom_create(struct NesData *data) {
    struct Mapper *m = (struct Mapper *)malloc(sizeof(struct Mapper));
    
    prgSize = data->header.prgSize * 0x4000;
    chrSize = data->header.chrSize * 0x2000;
    
    oneBank = prgSize == 0x4000;
    
    m->readPRG = mapper_cnrom_read_prg;
    m->writePRG = mapper_cnrom_write_prg;
    m->readCHR = mapper_cnrom_read_chr;
    m->writeCHR = mapper_cnrom_write_chr;
    m->getPagePointer = mapper_cnrom_get_page;
    
    m->mirroringType = data->header.mainFlags.flags6 & 1;
    
    return m;
}
