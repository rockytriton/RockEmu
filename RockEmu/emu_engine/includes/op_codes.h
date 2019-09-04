//
//  op_codes.h
//  Emu6502
//
//  Created by Rocky Pulley on 8/25/19.
//  Copyright © 2019 Rocky Pulley. All rights reserved.
//

#ifndef op_codes_h
#define op_codes_h

#include "common.h"

static const int IMM = 0;
static const int IMP = 1;
static const int INDX = 2;
static const int INDY = 3;
static const int ZP = 4;
static const int ACC = 5;
static const int ABS = 6;
static const int ABSY = 7;
static const int ABSX = 8;
static const int REL = 9;
static const int IND = 10;
static const int ZPX = 11;
static const int ZPY = 12;

struct OpCode {
    const char *name;
    uint8_t size;
    uint8_t address_mode;
    uint8_t cycles;
    uint8_t code;
};

extern struct OpCode opCodeLookup[];

#endif /* op_codes_h */
