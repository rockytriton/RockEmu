//
//  instructions.h
//  Emu6502
//
//  Created by Rocky Pulley on 8/25/19.
//  Copyright © 2019 Rocky Pulley. All rights reserved.
//

#ifndef instructions_h
#define instructions_h

#include "common.h"
#include "op_codes.h"
#include "cpu.h"

typedef uint8_t (*handle_inst)(struct CpuData *, struct OpCode *);

uint8_t instruction_execute(struct CpuData *, struct OpCode *opCode);

#ifdef __cplusplus
extern "C"
{
#endif
    
    void inst_add_handler(const char *name, handle_inst handler);
    handle_inst inst_get_handler(const char *name);
    
#ifdef __cplusplus
} // extern "C"
#endif

#endif /* instructions_h */
