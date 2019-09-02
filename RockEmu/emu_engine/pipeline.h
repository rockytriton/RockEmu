//
//  pipeline.h
//  Emu6502
//
//  Created by Rocky Pulley on 8/26/19.
//  Copyright © 2019 Rocky Pulley. All rights reserved.
//

#ifndef pipeline_h
#define pipeline_h

#include "ppu.h"

void ppu_prerender(struct PpuData *ppuData);
void ppu_render(struct PpuData *ppuData);
void ppu_postrender(struct PpuData *ppuData);
void ppu_vblank(struct PpuData *ppuData);

#endif /* pipeline_h */
