//
//  addr_modes.h
//  Emu6502
//
//  Created by Rocky Pulley on 8/25/19.
//  Copyright © 2019 Rocky Pulley. All rights reserved.
//

#ifndef addr_modes_h
#define addr_modes_h

#include "common.h"
#include "cpu.h"
#include "op_codes.h"

uint8_t addr_mode_fetch(struct CpuData *, struct OpCode *opCode);

#endif /* addr_modes_h */
