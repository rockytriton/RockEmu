//
//  bus.h
//  Emu6502
//
//  Created by Rocky Pulley on 8/25/19.
//  Copyright © 2019 Rocky Pulley. All rights reserved.
//

#ifndef bus_h
#define bus_h

#include "common.h"

void bus_init(void);
void bus_write(uint16_t address, uint8_t value);
uint8_t bus_read(uint16_t address);
void bus_load(uint8_t *data, uint16_t address, uint16_t size);

uint8_t *bus_read_ram(uint16_t address);
uint8_t *bus_page_pointer(uint8_t page);

#endif /* bus_h */
