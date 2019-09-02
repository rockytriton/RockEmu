//
//  mapper.c
//  Emu6502
//
//  Created by Rocky Pulley on 8/28/19.
//  Copyright © 2019 Rocky Pulley. All rights reserved.
//

#include "mapper.h"

static struct Mapper *current_mapper = 0;

struct Mapper *mapper_mm1_create(struct NesData *data);
struct Mapper *mapper_nrom_create(struct NesData *data);

struct Mapper *mapper_create(struct NesData *data) {
    
    switch(data->header.mainFlags.Bit.mn) {
        case 0: {
            current_mapper = mapper_nrom_create(data);
        } break;
        case 1: {
            current_mapper = mapper_mm1_create(data);
        } break;
    }
    
    return current_mapper;
}

struct Mapper *mapper_get_current(void) {
    return current_mapper;
}

