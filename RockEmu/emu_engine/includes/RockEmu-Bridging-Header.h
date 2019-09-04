
#include "bus.h"
#include "cpu.h"
#include "op_codes.h"
#include "cartridge.h"
#include "ppu.h"
#include "controller.h"
#include "system.h"

uint32_t ppu_get_pattern(uint16_t n, uint8_t x, uint8_t y);

void ppu_dump_ram() ;

