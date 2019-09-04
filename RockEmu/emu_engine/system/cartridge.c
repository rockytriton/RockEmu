//
//  cartridge.c
//  Emu6502
//
//  Created by Rocky Pulley on 8/25/19.
//  Copyright © 2019 Rocky Pulley. All rights reserved.
//

#include "cartridge.h"

#include "mapper.h"

struct NesHeader gameHeader;

struct NesData *nesData = 0;

struct NesData *cartridge_get_data(void) {
    return nesData;
}

struct NesHeader cartridge_get_header(void) {
    return gameHeader;
}

struct NesData *cartridge_load(const char *filename) {
    struct NesData *data = (struct NesData *)malloc(sizeof(struct NesData));
    data->valid = 0;
    
    FILE *fp = fopen(filename, "rb");
    if (!fread(&gameHeader, sizeof(gameHeader), 1, fp)) {
        return 0;
    }
    
    data->header = gameHeader;
    
    int trainer = gameHeader.mainFlags.flags6 & 0x04;
#ifdef ISLOGGING
    DOLOG("HEADER: %d, %d\r\n", gameHeader.mainFlags.Bit.tr, gameHeader.mainFlags.Bit.mn);
#endif
    if (trainer) {
        data->trainerData = (uint8_t *)malloc(0x200);
        
        if (!fread(data->trainerData, 0x200, 1, fp)) {
            return 0;
        }
    }
    
    int bytes = 0x4000 * gameHeader.prgSize;
    data->prgData = (uint8_t *)malloc(bytes);
    if (!fread(data->prgData, 0x4000 * gameHeader.prgSize, 1, fp)) {
        return 0;
    }
    
    if (gameHeader.chrSize) {
        data->chrData = (uint8_t *)malloc(0x2000 * gameHeader.chrSize);
        
        if (!fread(data->chrData, 0x2000 * gameHeader.chrSize, 1, fp)) {
            return 0;
        }
        
        printf("READ CHAR DATA: %0.4X - %d\r\n", 0x2000 * gameHeader.chrSize, gameHeader.chrSize);
    }
    
    nesData = data;
    
    mapper_create(nesData);
    
    return data;
}

