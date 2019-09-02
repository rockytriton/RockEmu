//
//  ppu_bus.h
//  Emu6502
//
//  Created by Rocky Pulley on 8/26/19.
//  Copyright © 2019 Rocky Pulley. All rights reserved.
//

#ifndef ppu_bus_h
#define ppu_bus_h

#include "common.h"

void ppu_bus_init(void);
uint8_t ppu_bus_read(uint16_t addr);

uint8_t ppu_bus_read_palette(uint8_t addr);
void ppu_bus_write(uint16_t addr, uint8_t value);
void ppu_bus_update_mirroring(void);

#endif /* ppu_bus_h */
