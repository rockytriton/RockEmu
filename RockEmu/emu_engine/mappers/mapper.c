//
//  mapper.c
//  Emu6502
//
//  Created by Rocky Pulley on 8/28/19.
//  Copyright © 2019 Rocky Pulley. All rights reserved.
//

#include "mapper.h"

static struct Mapper *current_mapper = 0;

static bool hasExtendedRam = false;
static bool fullRam = false;

struct Mapper *mapper_mm1_create(struct NesData *data);
struct Mapper *mapper_cnrom_create(struct NesData *data);
struct Mapper *mapper_nrom_create(struct NesData *data);
struct Mapper *mapper_mmc3_create(struct NesData *data) ;
struct Mapper *mapper_sxrom_create(struct NesData *data);

bool mapper_has_ext_ram() {
    return hasExtendedRam;
}

struct Mapper *mapper_create(struct NesData *data) {
    
    switch(data->header.mainFlags.Bit.mn) {
        case 0: {
            current_mapper = mapper_nrom_create(data);
        } break;
        case 1: {
            current_mapper = mapper_sxrom_create(data);
            fullRam = true;
        } break;
        case 3: {
            current_mapper = mapper_cnrom_create(data);
        } break;
        case 4: {
            current_mapper = mapper_mmc3_create(data);
            fullRam = true;
        } break;
        default: {
            printf("UNSUPPORTED: %d\r\n", data->header.mainFlags.Bit.mn);
            return 0;
        }
    }
    
    //hasExtendedRam = data->header.mainFlags.flags6 & 0x2;
    
    return current_mapper;
}

struct Mapper *mapper_get_current(void) {
    return current_mapper;
}

bool mapper_full_ram(void) {
    return fullRam;
}

