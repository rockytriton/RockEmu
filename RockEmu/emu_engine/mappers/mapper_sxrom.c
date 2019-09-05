//
//  mapper_sxrom.c
//  RockEmu
//
//  Created by Rocky Pulley on 9/4/19.
//  Copyright © 2019 Rocky Pulley. All rights reserved.
//

#include "common.h"
#include "mapper.h"

static byte regLoad = 0x10;
static byte regControl = 0;
static byte regChrBank0 = 0;
static byte regChrBank1 = 0;
static byte regPrgBank = 0;
static uint32_t prgSize = 0;

static byte *prgBanks[2];

static byte *chrBanks[2];

static byte prgRam[0x2000];

static bool shift_write(uint16_t addr, byte value) {
    bool shiftDone = regLoad & 1;
    
    regLoad >>= 1;
    regLoad |= (value & 1) << 4;
    
    if (!shiftDone) {
        return false;
    }
    
    return true;
}

void switch_chr_banks() {
    struct NesData *data = cartridge_get_data();
    
    if (regControl & 0x10) {
        //4kb mode
        //chrBanks[0] = &data->chrData[0x1000 * (regChrBank0 & 0x1E)];
        //chrBanks[1] = &data->chrData[0x1000 * (regChrBank1 & 0x1E)];
        
        chrBanks[0] = data->chrData + (0x1000 * (regChrBank0));
        chrBanks[1] = data->chrData + (0x1000 * (regChrBank1));
        //chrBanks[1] = data->chrData + (0x1000 * regLoad);
        
    } else {
        //8kb mode
        chrBanks[0] = &data->chrData[0x1000 * (regChrBank0 & 0x1E)];
        chrBanks[1] = chrBanks[0] + 0x1000;
    }
    
    //printf("SWITCHED CHR MODE: %d %0.8X - %0.8X, %0.8X\r\n", regControl & 0x10, data->chrData, chrBanks[0], chrBanks[1]);
    //printf("Switch mode 0 chr, %0.2X - %0.2X, %0.2X, %0.2X, %0.2X\r\n", regChrBank0, chrBanks[0][0] , chrBanks[0][1], chrBanks[1][0] , chrBanks[1][1] );
}
void switch_prg_banks() {
    struct NesData *data = cartridge_get_data();
    
    byte mode = (regControl >> 2) & 0x3;
    
    if (mode <= 1) {
        //32kb switch mode
        prgBanks[0] = &data->prgData[0x4000 * (regPrgBank & 0x0E)];
        prgBanks[1] = &data->prgData[0x4000 * (regPrgBank & 0x0E) + 0x4000];
    } else if (mode == 2) {
        //fix first, switch second
        prgBanks[0] = &data->prgData[0];
        prgBanks[1] = &data->prgData[0x4000 * (regPrgBank)];
    } else if (mode == 3) {
        //switch first, fix second
        prgBanks[0] = &data->prgData[0x4000 * (regPrgBank)];
        prgBanks[1] = &data->prgData[prgSize - 0x4000];
    }
    
    //printf("SWITCHED PRG MODE: %d\r\n", mode);
    //printf("STOP %0.2X - %0.8X, %0.8X, %0.8X\r\n", regPrgBank, data->prgData, prgBanks[0], prgBanks[1]);
    
    switch_chr_banks();
}

static void mapper_save(FILE *fp) {
    
    fwrite(&regLoad, sizeof(byte), 1, fp);
    fwrite(&regControl, sizeof(byte), 1, fp);
    fwrite(&regChrBank0, sizeof(byte), 1, fp);
    fwrite(&regChrBank1, sizeof(byte), 1, fp);
    fwrite(&regPrgBank, sizeof(byte), 1, fp);
    fwrite(&prgSize, sizeof(uint32_t), 1, fp);
    fwrite(prgRam, sizeof(prgRam), 1, fp);
    
}

static void mapper_load(FILE *fp) {
    
    fread(&regLoad, sizeof(byte), 1, fp);
    fread(&regControl, sizeof(byte), 1, fp);
    fread(&regChrBank0, sizeof(byte), 1, fp);
    fread(&regChrBank1, sizeof(byte), 1, fp);
    fread(&regPrgBank, sizeof(byte), 1, fp);
    fread(&prgSize, sizeof(uint32_t), 1, fp);
    fread(prgRam, sizeof(prgRam), 1, fp);
    
    switch_prg_banks();
}

void mapper_sxrom_write_prg(uint16_t addr, byte value) {
    if (addr >= 0x4020 && addr < 0x6000) {
        return; //nothing, exp rom
    }
    
    if (addr < 0x8000) {
        //chr ram
        prgRam[addr - 0x6000] = value;
        return;
    }
    
    if (value & 0x80) {
        regLoad = 0x10;
        regControl |= 0x0C;
        return;
    }
    
    if (!shift_write(addr, value)) {
        return;
    }
    
    if (addr <= 0x9FFF) {
        regControl = regLoad;
        
        switch(regControl & 0x03) {
            case 0: mapper_get_current()->mirroringType = OneScreenLower; break;
            case 1: mapper_get_current()->mirroringType = OneScreenHigher; break;
            case 2: mapper_get_current()->mirroringType = Vertical; break;
            case 3: mapper_get_current()->mirroringType = Horizontal; break;
        }
        
        ppu_bus_update_mirroring();
        switch_chr_banks();
        switch_prg_banks();
        
    } else if (addr <= 0xBFFF) {
        regChrBank0 = regLoad;
        switch_chr_banks();
    } else if (addr <= 0xDFFF) {
        regChrBank1 = regLoad;
        switch_chr_banks();
    } else {
        regPrgBank = regLoad;
        switch_prg_banks();
    }
    
    regLoad = 0x10;
}
extern int counter;

byte mapper_sxrom_read_prg(uint16_t addr) {
    if (counter >= 0x00006679) {
        struct NesData *data = cartridge_get_data();
        //printf("STOP %0.8X, %0.8X, %0.8X\r\n", data->prgData, prgBanks[0], prgBanks[1]);
    }
    
    if (addr < 0x8000) {
        //8kb ram mode.
        return prgRam[addr - 0x6000];
    }
    
    if (addr < 0xC000) {
        return prgBanks[0][addr - 0x8000];
    }
    
    byte b = prgBanks[1][addr - 0xC000];
    
    //printf("B: %0.2X\r\n", b);
    
    return b;
}

void mapper_sxrom_write_chr(uint16_t addr, byte value) {
    if (addr < 0x1000) {
        chrBanks[0][addr] = value;
    }
    
    if (addr > 0x2000) {
        printf("ERROR IN SIZE: %0.4X\r\n", addr);
    }
    
    chrBanks[1][addr - 0x1000] = value;
}

byte mapper_sxrom_read_chr(uint16_t addr) {
    if (addr < 0x1000) {
        return chrBanks[0][addr];
    }
    
    if (addr > 0x2000) {
        printf("ERROR IN SIZE: %0.4X\r\n", addr);
    }
    
    return chrBanks[1][addr - 0x1000];
}

uint8_t *mapper_sxrom_get_page(uint16_t addr) {
    if (addr < 0xC000) {
        return &prgBanks[0][addr - 0x8000];
    }
    
    return &prgBanks[1][addr - 0xC000];
}

struct Mapper *mapper_sxrom_create(struct NesData *data) {
    struct Mapper *m = (struct Mapper *)malloc(sizeof(struct Mapper));
    
    
    if (data->header.chrSize) {
        chrBanks[0] = data->chrData;
        chrBanks[1] = &data->chrData[0x1000];
    } else {
        data->chrData = (byte *)malloc(0x2000);
        chrBanks[0] = data->chrData;
        chrBanks[1] = data->chrData + 0x1000;
        data->header.chrSize = 1;
    }
    
    prgSize = (data->header.prgSize * 0x4000);
    
    prgBanks[0] = data->prgData;
    prgBanks[1] = &data->prgData[prgSize - 0x4000];

    byte b1 = prgBanks[1][0x3FFC];
    byte b2 = prgBanks[1][0x3FFD];
    
    printf("START ADDR: %0.2X%0.2X\r\n", b1, b2);
    
    m->readPRG = mapper_sxrom_read_prg;
    m->writePRG = mapper_sxrom_write_prg;
    m->readCHR = mapper_sxrom_read_chr;
    m->writeCHR = mapper_sxrom_write_chr;
    m->getPagePointer = mapper_sxrom_get_page;
    m->save = mapper_save;
    m->load = mapper_load;
    
    m->mirroringType = data->header.mainFlags.flags6 & 1;
    
    return m;
}

