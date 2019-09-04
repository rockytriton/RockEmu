//
//  addr_modes.c
//  Emu6502
//
//  Created by Rocky Pulley on 8/25/19.
//  Copyright © 2019 Rocky Pulley. All rights reserved.
//

#include "addr_modes.h"
#include "bus.h"

#include <string.h>

uint8_t addr_mode_fetch(struct CpuData *data, struct OpCode *opCode) {
    
    switch(opCode->address_mode) {
        case IMP: {
            data->fetched = data->regA;
            return 0;
        } 
        
        case IMM: {
            data->addr_abs = data->pc;
            data->pc++;
            return 0;
        }
            
        case ZP: {
            data->addr_abs = bus_read(data->pc);
            data->addr_abs &= 0x00FF;
            data->pc++;
            return 0;
        }
            
        case ZPX: {
            data->addr_abs = bus_read(data->pc) + data->regX;
            data->addr_abs &= 0x00FF;
            data->pc++;
            return 0;
        }
            
        case ZPY: {
            data->addr_abs = bus_read(data->pc) + data->regY;
            data->addr_abs &= 0x00FF;
            data->pc++;
            return 0;
        }
            
        case ABS: {
            uint16_t lo = bus_read(data->pc);
            uint16_t hi = bus_read(data->pc + 1);
            data->addr_abs = (hi << 8) | lo;
            
            data->pc += 2;
            
            return 0;
        }
            
        case ABSX: {
            uint16_t lo = bus_read(data->pc);
            uint16_t hi = bus_read(data->pc + 1);
            data->addr_abs = (hi << 8) | lo;
            data->addr_abs += data->regX;
            
            data->pc += 2;
            
            if ((data->addr_abs & 0xFF00) != (hi << 8)) {
                return 1;
            }
            
            return 0;
        }
            
        case ABSY: {
            uint16_t lo = bus_read(data->pc);
            uint16_t hi = bus_read(data->pc + 1);
            data->addr_abs = (hi << 8) | lo;
            data->addr_abs += data->regY;
            
            data->pc += 2;
            
            if ((data->addr_abs & 0xFF00) != (hi << 8)) {
                return 1;
            }
            
            return 0;
        }
        
        case IND: {
            uint16_t lo = bus_read(data->pc);
            uint16_t hi = bus_read(data->pc + 1);
            uint16_t ptr = (hi << 8) | lo;
            
            if (lo == 0x00FF) {
                data->addr_abs = (bus_read(ptr & 0xFF00) << 8) | bus_read(ptr);
            } else {
                data->addr_abs = (bus_read(ptr + 1) << 8) | bus_read(ptr);
            }
            
            data->pc += 2;
            
            return 0;
        }
            
        case INDX: {
            uint16_t t = bus_read(data->pc);
            
            data->pc++;
            
            uint16_t lo = bus_read((uint16_t)(t + (uint16_t)data->regX) & 0x00FF);
            uint16_t hi = bus_read((uint16_t)(t + (uint16_t)data->regX + 1) & 0x00FF);
            data->addr_abs = (hi << 8) | lo;
            
            return 0;
        }
            
        case INDY: {
            uint16_t t = bus_read(data->pc);
            //DOLOG("INDY 01: %0.4X\r\n", t);
            data->pc++;
            
            uint16_t lo = bus_read(t & 0x00FF);
            //DOLOG("INDY 02: %0.4X\r\n", lo);
            uint16_t hi = bus_read((t + 1) & 0x00FF);
            //DOLOG("INDY 03: %0.4X\r\n", hi);
            data->addr_abs = (hi << 8) | lo;
            data->addr_abs += data->regY;
            
            if ((data->addr_abs & 0xFF00) != (hi << 8)) {
                return 1;
            }
            
            return 0;
        }
         
        case REL: {
            data->addr_rel = bus_read(data->pc);
            
            if (data->addr_rel & 0x80) {
                data->addr_rel |= 0xFF00;
            }
            
            data->pc++;
            
            return 0;
        }
    }
    
    return 0;
}
