//
//  cartridge.h
//  Emu6502
//
//  Created by Rocky Pulley on 8/25/19.
//  Copyright © 2019 Rocky Pulley. All rights reserved.
//

#ifndef cartridge_h
#define cartridge_h

#include "common.h"

struct NesHeader {
    uint32_t type;
    uint8_t prgSize;
    uint8_t chrSize;
    union {
        uint8_t flags6;
        struct {
            char mt : 1;
            char b : 1;
            char tr : 1;
            char m4 : 1;
            char mn : 4;
        } Bit;
    } mainFlags;
    uint8_t flags7;
    uint8_t flags8;
    uint8_t flags9;
    uint8_t flags10;
    uint8_t padding;
    uint32_t morePadding;
};

struct NesData {
    struct NesHeader header;
    uint8_t *prgData;
    uint8_t *chrData;
    uint8_t valid;
};

struct NesData *cartridge_load(const char *filename);

struct NesData *cartridge_get_data(void);

struct NesHeader cartridge_get_header(void);

const char *cartridge_get_filename(void);

/*
 An iNES file consists of the following sections, in order:
 
 Header (16 bytes)
 Trainer, if present (0 or 512 bytes)
 PRG ROM data (16384 * x bytes)
 CHR ROM data, if present (8192 * y bytes)
 PlayChoice INST-ROM, if present (0 or 8192 bytes)
 PlayChoice PROM, if present (16 bytes Data, 16 bytes CounterOut) (this is often missing, see PC10 ROM-Images for details)
 Some ROM-Images additionally contain a 128-byte (or sometimes 127-byte) title at the end of the file.
 
 The format of the header is as follows:
 
 0-3: Constant $4E $45 $53 $1A ("NES" followed by MS-DOS end-of-file)
 4: Size of PRG ROM in 16 KB units
 5: Size of CHR ROM in 8 KB units (Value 0 means the board uses CHR RAM)
 6: Flags 6 - Mapper, mirroring, battery, trainer
 7: Flags 7 - Mapper, VS/Playchoice, NES 2.0
 8: Flags 8 - PRG-RAM size (rarely used extension)
 9: Flags 9 - TV system (rarely used extension)
 10: Flags 10 - TV system, PRG-RAM presence (unofficial, rarely used extension)
 11-15: Unused padding (should be filled with zero, but some rippers put their name across bytes 7-15)
 
 76543210
 ||||||||
 |||||||+- Mirroring: 0: horizontal (vertical arrangement) (CIRAM A10 = PPU A11)
 |||||||              1: vertical (horizontal arrangement) (CIRAM A10 = PPU A10)
 ||||||+-- 1: Cartridge contains battery-backed PRG RAM ($6000-7FFF) or other persistent memory
 |||||+--- 1: 512-byte trainer at $7000-$71FF (stored before PRG data)
 ||||+---- 1: Ignore mirroring control or above mirroring bit; instead provide four-screen VRAM
 ++++----- Lower nybble of mapper number
 */

#endif /* cartridge_h */
