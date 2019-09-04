//
//  mapper.h
//  Emu6502
//
//  Created by Rocky Pulley on 8/28/19.
//  Copyright © 2019 Rocky Pulley. All rights reserved.
//

#ifndef mapper_h
#define mapper_h

#include "common.h"
#include "cartridge.h"

enum MirroringType
{
    Horizontal  = 0,
    Vertical    = 1,
    FourScreen  = 8,
    OneScreenLower,
    OneScreenHigher,
};

typedef void (*MapperWrite)(uint16_t addr, uint8_t value);
typedef uint8_t (*MapperRead)(uint16_t addr);
typedef uint8_t *(*MapperReadPtr)(uint16_t addr);

struct Mapper {
    MapperRead readPRG;
    MapperWrite writePRG;
    
    MapperRead readCHR;
    MapperWrite writeCHR;
    
    MapperReadPtr getPagePointer;
    
    enum MirroringType mirroringType;
};

struct Mapper *mapper_create(struct NesData *data);

struct Mapper *mapper_get_current(void);

bool mapper_has_ext_ram(void);

bool mapper_full_ram(void);

/*
 MapperNROM(Cartridge& cart);
 void writePRG (Address addr, Byte value);
 Byte readPRG (Address addr);
 const Byte* getPagePtr(Address addr);
 
 Byte readCHR (Address addr);
 void writeCHR (Address addr, Byte value);
*/

#endif /* mapper_h */
