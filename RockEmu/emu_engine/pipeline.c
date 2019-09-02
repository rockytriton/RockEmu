//
//  pipeline.c
//  Emu6502
//
//  Created by Rocky Pulley on 8/26/19.
//  Copyright © 2019 Rocky Pulley. All rights reserved.
//

#include "pipeline.h"
#include "ppu_bus.h"
#include "cpu.h"

const int ScanlineCycleLength = 341;
const int ScanlineEndCycle = 340;
const int VisibleScanlines = 240;
const int ScanlineVisibleDots = 256;
const int FrameEndScanline = 261;

const uint32_t colors[] = {
    0x666666ff, 0x002a88ff, 0x1412a7ff, 0x3b00a4ff, 0x5c007eff, 0x6e0040ff, 0x6c0600ff, 0x561d00ff,
    0x333500ff, 0x0b4800ff, 0x005200ff, 0x004f08ff, 0x00404dff, 0x000000ff, 0x000000ff, 0x000000ff,
    0xadadadff, 0x155fd9ff, 0x4240ffff, 0x7527feff, 0xa01accff, 0xb71e7bff, 0xb53120ff, 0x994e00ff,
    0x6b6d00ff, 0x388700ff, 0x0c9300ff, 0x008f32ff, 0x007c8dff, 0x000000ff, 0x000000ff, 0x000000ff,
    0xfffeffff, 0x64b0ffff, 0x9290ffff, 0xc676ffff, 0xf36affff, 0xfe6eccff, 0xfe8170ff, 0xea9e22ff,
    0xbcbe00ff, 0x88d800ff, 0x5ce430ff, 0x45e082ff, 0x48cddeff, 0x4f4f4fff, 0x000000ff, 0x000000ff,
    0xfffeffff, 0xc0dfffff, 0xd3d2ffff, 0xe8c8ffff, 0xfbc2ffff, 0xfec4eaff, 0xfeccc5ff, 0xf7d8a5ff,
    0xe4e594ff, 0xcfef96ff, 0xbdf4abff, 0xb3f3ccff, 0xb5ebf2ff, 0xb8b8b8ff, 0x000000ff, 0x000000ff,
};

int nmiNum = 0;

uint8_t ppu_read(uint16_t addr) {
    return ppu_bus_read(addr);
}

uint32_t ppu_get_pattern(uint16_t n, uint8_t x, uint8_t y) {
    uint16_t addr1 = (n * 16) + y;
    uint16_t addr2 = addr1 + 8;
    
    uint8_t b1 = (ppu_bus_read(addr1) & (1 << x)) != 0;
    uint8_t b2 = (ppu_bus_read(addr2) & (1 << x)) != 0;
    
    uint8_t b3 = b1 << 1;
    
    uint8_t ret = b3 | b2;
    
    return ret;
}



bool veryfirst = true;

int dumpingRam = 0;
int doDumpRam= 0;

void ppu_prerender(struct PpuData *ppuData) {
    //DOLOG("PR: %4d\r\n", ppuData->cycle);
    if (ppuData->cycle == 1) {
        SET_FLAG(ppuData->regStatus, PPUSTAT_V, 0);
        SET_FLAG(ppuData->regStatus, PPUSTAT_S, 0);
        
        if (doDumpRam) {
            doDumpRam = 0;
            dumpingRam = 1;
        }
        
    } else if (ppuData->cycle == ScanlineVisibleDots + 2 && GET_FLAG(ppuData->regMask, PPUMASK_BE) && GET_FLAG(ppuData->regMask, PPUMASK_SE)) {
        ppuData->dataAddress &= ~0x41F;
        ppuData->dataAddress |= ppuData->tempAddress & 0x41F;
        //DOLOG("PPUDATA PR1: %0.4X\r\n", ppuData->dataAddress);
        
    } else if (ppuData->cycle > 280 && ppuData->cycle <= 304 && GET_FLAG(ppuData->regMask, PPUMASK_BE) && GET_FLAG(ppuData->regMask, PPUMASK_SE)) {
        ppuData->dataAddress &= ~0x7BE0;
        ppuData->dataAddress |= ppuData->tempAddress & 0x7BE0;
        //DOLOG("PPUDATA PR2: %0.4X\r\n", ppuData->dataAddress);
        
    }
    
    if (ppuData->cycle >= ScanlineEndCycle - (!ppuData->evenFrame && GET_FLAG(ppuData->regMask, PPUMASK_BE) && GET_FLAG(ppuData->regMask, PPUMASK_SE))) {
        ppuData->pipelineState = PPU_STATE_RENDER;
        ppuData->cycle = 0; //veryfirst ? -1 : 0;
        ppuData->scanLine = 0;
        veryfirst = false;
        DOLOG("---- SCANLINE %d\r\n", ppuData->scanLine);
    }
}

void render_visible_dots(struct PpuData *ppuData) {
    uint8_t bgColor = 0;
    uint8_t sprColor = 0;
    bool bgOpaque = false;
    bool sprOpaque = true;
    bool spriteForeground = false;
    
    uint16_t x = ppuData->cycle - 1;
    uint16_t y = ppuData->scanLine;
    bool logging = false;
    
    if (doDumpRam) {
        printf("Dumping\r\n");
    }
    
    if (ppuData->scanLine == 0x42 && x == 0x33 && y == 0x42 && ppuData->scanLineSpritesSize == 6) {
        DOLOG("HIT NEXT IF %d, %d\r\n", x, ppuData->scanLineSpritesSize);
        //logging = true;
    }
    
    if (GET_FLAG(ppuData->regMask, PPUMASK_BE)) {
        uint16_t xFine = (ppuData->fineXScroll + x) % 8;
        if (logging) printf("XFINE: %0.4x\r\n", xFine);
        //DOLOG("MASK BLE: %0.2X %0.2X\r\n", GET_FLAG(ppuData->regMask, PPUMASK_BLE), GET_FLAG(ppuData->regControl, PPUCTRL_B));
        
        if (GET_FLAG(ppuData->regMask, PPUMASK_BLE) || x >= 8) {
            uint16_t addr = 0x2000 | (ppuData->dataAddress & 0x0FFF); //tile
            uint8_t tile = ppu_read(addr);
            addr = (tile * 16) + ((ppuData->dataAddress >> 12) & 0x7);
            addr |= GET_FLAG(ppuData->regControl, PPUCTRL_B) << 12;
            //DOLOG("ADDR: %0.4X - %0.4X\r\n", addr, ppuData->dataAddress);
            bgColor = (ppu_read(addr) >> (7 ^ xFine)) & 1;
            //DOLOG("BGC1: %0.2X\r\n", bgColor);
            bgColor |= ((ppu_read(addr + 8) >> (7 ^ xFine)) & 1) << 1;
            //DOLOG("BGC2: %0.2X\r\n", bgColor);
            
            if (nmiNum == 0x61) {
                //printf("BGCOLOR INFO 01: %0.4X %0.4X %0.4X - %0.2X %0.2X %0.2X - %0.2X %0.2X %0.2X - %0.2X\r\n", ppuData->dataAddress, 0x2000 | (ppuData->dataAddress & 0x0FFF), addr, //ppu_read(addr), xFine, (ppu_read(addr) >> (7 ^ xFine)) & 1,
                      // ppu_read(addr + 8), ((ppu_read(addr + 8) >> (7 ^ xFine)) & 1) << 1, bgColor, tile);
                
                
            }
            
            bgOpaque = bgColor;
            
            addr = 0x23C0 | (ppuData->dataAddress & 0x0C00) | ((ppuData->dataAddress >> 4) & 0x38) | ((ppuData->dataAddress >> 2) & 0x07);
            uint8_t att = ppu_read(addr);
            int shift = ((ppuData->dataAddress >> 4) & 4) | (ppuData->dataAddress & 2);
            bgColor |= ((att >> shift) & 0x3) << 2;
            
            if (nmiNum == 0x61) {
                //printf("BGCOLOR INFO 02: %0.4X %0.2X %0.4X %0.2X %0.2X\r\n", addr, att, shift, ((att >> shift) & 0x3) << 2, bgColor);
                
                
            }
            //DOLOG("BGC3: %0.2X\r\n", bgColor);
        }
        
        if (xFine == 7) {
            if ((ppuData->dataAddress & 0x001F) == 31) {
                ppuData->dataAddress &= ~0x001F;
                ppuData->dataAddress ^= 0x0400;
                //DOLOG("PPUDATA XF1: %0.4X\r\n", ppuData->dataAddress);
            } else {
                ppuData->dataAddress++;
                //DOLOG("PPUDATA INC1: %0.4X\r\n", ppuData->dataAddress);
            }
        }
    }
    
    if (GET_FLAG(ppuData->regMask, PPUMASK_SE) && (GET_FLAG(ppuData->regMask, PPUMASK_SLE) || x >= 8)) {
        
        if (logging) {
            for (int y=0; y<4; y++) {
                for (int x=0; x<64; x++) {
                    printf("%0.2X ", ppuData->spriteMemory[(y * 4) + x]);
                }
                
                printf("\r\n");
            }
            
            printf("\r\n");
            
            for (int i=0; i<ppuData->scanLineSpritesSize; i++) {
                printf("%0.2X - %0.2X\r\n", ppuData->scanLineSprites[i],  ppuData->spriteMemory[i * 4 + 3]);
            }
        }
        
        for (int ii=0; ii<ppuData->scanLineSpritesSize; ii++) {
            int i = ppuData->scanLineSprites[ii];
            uint8_t sx = ppuData->spriteMemory[i * 4 + 3];

            
            
            if (0 > x - sx || x - sx >= 8) {
                //DOLOG("NOPE CONTINUE: %d, %d\r\n", x, sx);
                continue;
            }
            
            //DOLOG("YEP: %d, %d\r\n", x, sx);
            
            uint8_t sy = ppuData->spriteMemory[i * 4 + 0] + 1;
            uint8_t tile = ppuData->spriteMemory[i * 4 + 1];
            uint8_t att = ppuData->spriteMemory[i * 4 + 2];
            
            int len = GET_FLAG(ppuData->regControl, PPUCTRL_H) ? 16 : 8;
            int xShift = (x - sx) % 8;
            int yOffset = (y - sy) % len;
            
            if ((att & 0x40) == 0) {
                xShift ^= 7;
            }
            
            if ((att & 0x80) != 0) {
                yOffset ^= (len - 1);
            }
            
            uint16_t addr = 0;
            
            if (!(GET_FLAG(ppuData->regControl, PPUCTRL_H))) {
                addr = tile * 16 + yOffset;
                
                if (GET_FLAG(ppuData->regControl, PPUCTRL_S)) {
                    addr += 0x1000;
                }
            } else {
                yOffset = (yOffset & 7) | ((yOffset & 8) << 1);
                addr = (tile >> 1) * 32 + yOffset;
                addr |= (tile & 1) << 12;
            }
            
            sprColor |= (ppu_read(addr) >> (xShift)) & 1;
            sprColor |= ((ppu_read(addr + 8) >> (xShift)) & 1) << 1;
            
            if (!(sprOpaque = sprColor)) {
                sprColor = 0;
                //DOLOG("NOCOLOREQ: %d, %d - %0.4X, %0.4X, %0.4X, %0.4X\r\n", sprOpaque, sprOpaque, sx, sy, tile, addr);
                continue;
            }
            
            sprColor |= 0x10;
            sprColor |= (att & 0x3) << 2;
            
            spriteForeground = !(att & 0x20);
            
            //DOLOG("CHECKHIT: %0.2X, %0.2X %d, %d - %0.4X, %0.4X, %0.4X, %0.4X\r\n", spriteForeground, sprColor, sprOpaque, sprOpaque, sx, sy, tile, addr);
            //DOLOG("CHECKHIT: %d, %d %d, %d - %d\r\n", GET_FLAG(ppuData->regStatus, PPUSTAT_S), GET_FLAG(ppuData->regMask, PPUMASK_BE), i, sprOpaque, bgOpaque);
            
            if (!(GET_FLAG(ppuData->regStatus, PPUSTAT_S)) && GET_FLAG(ppuData->regMask, PPUMASK_BE) && i == 0 && sprOpaque && bgOpaque) {
                DOLOG("SET SP ZERO HIT\r\n");
                SET_FLAG(ppuData->regStatus, PPUSTAT_S, 1);
            }
            
            break;
        }
    }
    
    uint8_t paletteAddr = bgColor;
    
    if ((!bgOpaque && sprOpaque) || (bgOpaque && sprOpaque && spriteForeground)) {
        paletteAddr = sprColor;
    } else if (!bgOpaque && !sprOpaque) {
        paletteAddr = 0;
    }
    
    uint8_t color = ppu_bus_read_palette(paletteAddr);
    //printf("BUS COLOR %0.8X\r\n");
    
    uint32_t hi = (colors[color] & 0x0000FFFF) << 16;
    uint16_t lo = (colors[color] & 0xFFFF0000) >> 16;
    
    ppuData->pictureBuffer[x][y] = colors[color];
    
    if (dumpingRam) {
        
        printf("SCANLINE: %0.2X, X: %0.2X, Y: %0.2X, P: %0.2X, SLS: %d\r\n", ppuData->scanLine, x, y, color, ppuData->scanLineSpritesSize);
        printf("COLORDATA: %0.2X %0.2X %0.2X %0.2X %0.2X\r\n", bgOpaque, sprOpaque, spriteForeground, bgColor, paletteAddr);
        
        for (int i=0; i<ppuData->scanLineSpritesSize; i++) {
            printf("%0.2X ", ppuData->scanLineSprites[i]);
        }
        
        printf("\r\n\r\n");
    }
    
    if (nmiNum == 0x61) {
        //printf("COLORDATA: %0.2X %0.2X %0.2X %0.2X %0.2X\r\n", bgOpaque, sprOpaque, spriteForeground, bgColor, paletteAddr);
        
        //printf("COLOR: %0.2X - %0.8X - %0.8X\r\n", color, colors[color], ppuData->pictureBuffer[x][y]);
        return;
    }
}

extern struct PpuData ppuData;

void ppu_dump_ram() {
    doDumpRam = 1;
    printf("DUMPING RAM\r\n\r\n");
    
    for (int y=0; y<4; y++) {
        for (int x=0; x<64; x++) {
        printf("%0.2X ", ppuData.spriteMemory[(y * 64) + x]);
        }
        printf("\r\n");
    }
    
    printf("\r\n\r\n");
}

void ppu_render(struct PpuData *ppuData) {
    
    
    //DOLOG(" R: %4d - %d\r\n", ppuData->cycle, ppuData->scanLineSpritesSize);
    if (ppuData->cycle > 0 && ppuData->cycle <= ScanlineVisibleDots) {
        render_visible_dots(ppuData);
    } else if (ppuData->cycle == ScanlineVisibleDots + 1 && GET_FLAG(ppuData->regMask, PPUMASK_BE)) {
        if ((ppuData->dataAddress & 0x7000) != 0x7000) {
            ppuData->dataAddress += 0x1000;
            //DOLOG("PPUDATA REN1: %0.4X\r\n", ppuData->dataAddress);
        } else {
            ppuData->dataAddress &= ~0x7000;
            uint16_t y = (ppuData->dataAddress & 0x03E0) >> 5;
            if (y == 29) {
                y = 0;
                ppuData->dataAddress ^= 0x0800;
                //DOLOG("PPUDATA REN2: %0.4X\r\n", ppuData->dataAddress);
            } else if (y == 31) {
                y = 0;
            } else {
                y++;
            }
            
            //DOLOG("RENDERING SET DATA ADDR: %0.4X", ppuData->dataAddress);
            ppuData->dataAddress = (ppuData->dataAddress & ~0x03E0) | (y << 5);
            //DOLOG(" TO %0.4X\r\n", ppuData->dataAddress);
        }
    } else if (ppuData->cycle == ScanlineVisibleDots + 2 && GET_FLAG(ppuData->regMask, PPUMASK_BE) && GET_FLAG(ppuData->regMask, PPUMASK_SE)) {
        //DOLOG("REG MASK: %0.2X\r\n", ppuData->regMask);
        ppuData->dataAddress &= ~0x41F;
        ppuData->dataAddress |= ppuData->tempAddress & 0x41F;
        //DOLOG("PPUDATA REN3: %0.4X\r\n", ppuData->dataAddress);
    }
    
    if (ppuData->cycle >= ScanlineEndCycle) {
        ppuData->scanLineSpritesSize = 0;
        memset(ppuData->scanLineSprites, 0, 64);
        
        int range = 8;
        
        if (GET_FLAG(ppuData->regControl, PPUCTRL_H)) {
            range = 16;
        }
        
        int j = 0;
        //DOLOG("SPRADDR: %0.4X\r\n", ppuData->spriteDataAddress);
        for (uint16_t i = ppuData->spriteDataAddress / 4; i < 64; i++) {
            //DOLOG("SLINFO: %d, %0.2X - %d\r\n", ppuData->scanLine, ppuData->spriteMemory[i * 4], range);
            int diff = (ppuData->scanLine - ppuData->spriteMemory[i * 4]);
            
            if (0 <= diff && diff < range) {
                ppuData->scanLineSprites[ppuData->scanLineSpritesSize++] = i;
                j++;
                
                if (j >= 8) {
                    break;
                }
            }
        }
        
        ppuData->scanLine++;
        ppuData->cycle = 0;
        DOLOG("---- SCANLINE %d\r\n", ppuData->scanLine);
    }
    
    if (ppuData->scanLine >= VisibleScanlines) {
        ppuData->pipelineState = PPU_STATE_POST;
    }
}

void ppu_postrender(struct PpuData *ppuData) {
    dumpingRam = 0;
    
    if (ppuData->cycle >= ScanlineEndCycle) {
        ppuData->scanLine++;
        DOLOG("---- SCANLINE %d\r\n", ppuData->scanLine);
        ppuData->cycle = 0;
        ppuData->pipelineState = PPU_STATE_VBLANK;
        
        for (int x = 0; x < ScanlineVisibleDots; x++) {
            for (int y=0; y < VisibleScanlines; y++) {
                //TODO m_screen.setPixel(x, y, m_pictureBuffer[x][y]);
            }
        }
    }
}

void ppu_vblank(struct PpuData *ppuData) {
    if (ppuData->cycle == 1 && ppuData->scanLine == VisibleScanlines + 1) {
        SET_FLAG(ppuData->regStatus, PPUSTAT_V, 1);
        
        if (GET_FLAG(ppuData->regControl, PPUCTRL_V)) {
            
            nmiNum++;
            
            //printf("NMI: %0.8X\r\n", nmiNum);
            
            if (nmiNum == 0x62) {
                
                //printf("\r\n\r\nPICTUREDUMP\r\n");
                for (int y=0; y<ScanlineVisibleDots; y++) {
                    for (int x=0; x<VisibleScanlines; x++) {
                        //printf("%0.8X ", ppuData->pictureBuffer[y][x]);
                    }
                    //printf("\r\n");
                }
                
                //printf("\r\n\r\nSPRITEDUMP\r\n");
                for (int n=0; n<4; n++) {
                    for (int j=0; j<64; j++) {
                        //printf("%0.2X ", ppuData->spriteMemory[(n * 64) + j]);
                        
                    }
                    
                    //printf("\r\n");
                }
                
                //printf("\r\n\r\n");
            }
            
            cpu_nmi();
        }
    }
    
    if (ppuData->cycle >= ScanlineEndCycle) {
        ppuData->scanLine++;
        ppuData->cycle = 0;
        DOLOG("---- SCANLINE %d\r\n", ppuData->scanLine);
    }
    
    if (ppuData->scanLine >= FrameEndScanline) {
        ppuData->pipelineState = PPU_STATE_PRE;
        ppuData->scanLine = 0;
        DOLOG("---- SCANLINE %d\r\n", ppuData->scanLine);
        ppuData->evenFrame = !ppuData->evenFrame;
    }
}
