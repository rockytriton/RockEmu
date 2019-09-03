//
//  pipeline_new.c
//  RockEmu
//
//  Created by Rocky Pulley on 9/2/19.
//  Copyright © 2019 Rocky Pulley. All rights reserved.
//

#include "pipeline.h"
#include "cpu.h"
#include "ppu_bus.h"

extern bool keyPressed[];

enum RenderState {
    PreRender,
    Render,
    PostRender,
    VBlank
};

extern uint32_t colors[];

enum RenderState pipelineState = PreRender;

int curFrame = 0;
int readCycle = 0;

static struct PpuData *ppuData = 0;

uint8_t curTile = 0;
uint8_t curAtt = 0;
uint8_t secondaryOAM[64];

enum RenderState getRenderState() {
    if (ppuData->scanLine == 261) {
        return PreRender;
    }
    
    if (ppuData->scanLine == 240) {
        return PostRender;
    }
    
    if (ppuData->scanLine > 240) {
        return VBlank;
    }
    
    return Render;
}

void ppu_render_new_pre() {
    if (ppuData->cycle == 1) {
        SET_FLAG(ppuData->regStatus, PPUSTAT_V, 0);
        SET_FLAG(ppuData->regStatus, PPUSTAT_S, 0);
    }
    
}

uint8_t patternHi = 0;
uint8_t patternLo = 0;

void ppu_render_new_render(enum RenderState rs) {
    if (ppuData->cycle == 0) {
        readCycle = 0;
        return; //idle cycle
    } else if (ppuData->cycle == 1) {
        memset(secondaryOAM, 0xFF, 64);
    }
    
    if (ppuData->cycle < 240) {
        //render cycle
        
        uint8_t x = ppuData->cycle - 1;
        uint16_t addr = (ppuData->dataAddress & 0xFFF) | 0x2000;
        curTile = ppu_bus_read(addr);
        
        addr = (curTile * 0x10) + ((ppuData->dataAddress >> 12) & 7);
        
        if (GET_FLAG(ppuData->regControl, PPUCTRL_B)) {
            addr |= (1 << 12);
        }
        
        uint8_t color = ppu_bus_read(addr) >> (7 - x) & 1;
        color |= (ppu_bus_read(addr + 8) >> (7 - x) & 1) << 1;
        
        addr = 0x23C0 + (x == 0 ? 0 : 32 / x);
        curAtt = ppu_bus_read(addr);
        
        int shift = ((ppuData->dataAddress >> 4) & 4) | (ppuData->dataAddress & 2);
        
        color |= ((curAtt >> shift) & 3) << 2;
        
        uint8_t p = ppu_bus_read_palette(color);
        
        uint32_t c = colors[p];
        
        if (rs == Render) {
            ppuData->pictureBuffer[x][ppuData->scanLine] = c;
        }
        
        if (readCycle == 8) {
            
            if ((ppuData->dataAddress & 0x001F) == 31) // if coarse X == 31
            {
                ppuData->dataAddress &= ~0x001F;          // coarse X = 0
                ppuData->dataAddress ^= 0x0400;           // switch horizontal nametable
            } else {
                ppuData->dataAddress += 1;
            }
        }
        
        
    }
    
    
    
    if (ppuData->cycle >= 65 && ppuData->cycle <= 256) {
        //sprite data eval:
        bool odd = ppuData->cycle % 2;
        
        if (odd) {
            
        }
    }
}

void ppu_render_new(struct PpuData *p) {
    ppuData = p;
    
    enum RenderState rs = getRenderState();
    
    if (keyPressed[2]) {
        printf("PPU: %0.2X - %d\r\n", ppuData->cycle, rs);
    }
    
    if (rs == PreRender) {
        ppu_render_new_pre();
    }
    
    if (rs == PostRender) {
        if (ppuData->cycle >= 340) {
            ppuData->scanLine++;
            ppuData->cycle = 0;
        }
        
        return;
    }
    
    if (rs == VBlank) {
        if (ppuData->cycle == 1) {
            SET_FLAG(ppuData->regStatus, PPUSTAT_V, 1);
            cpu_nmi();
        }
        
        if (ppuData->cycle >= 340) {
            ppuData->scanLine++;
            ppuData->cycle = 0;
        }
        
        return;
    }
    
    ppu_render_new_render(rs);
    
    if (rs == PreRender && ppuData->cycle == 239 && (curFrame % 2) == 1) {
        ppuData->cycle++;
    }
    
    if (ppuData->cycle >= 340) {
        ppuData->cycle = 0;
        ppuData->scanLine++;
    }
    
    if (ppuData->scanLine >= 261) {
        ppuData->scanLine = 0;
        ppuData->evenFrame = !ppuData->evenFrame;
    }
}
