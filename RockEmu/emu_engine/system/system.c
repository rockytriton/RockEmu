//
//  system.c
//  RockEmu
//
//  Created by Rocky Pulley on 9/4/19.
//  Copyright © 2019 Rocky Pulley. All rights reserved.
//

#include "system.h"

#include "bus.h"
#include "cpu.h"
#include "ppu.h"
#include "mapper.h"
#include "cartridge.h"

#include <errno.h>

extern struct CpuData cpuData;
extern struct PpuData ppuData;
extern struct NesData *nesData;

extern const int ScanlineCycleLength ;
extern const int ScanlineEndCycle ;
extern const int VisibleScanlines ;
extern const int ScanlineVisibleDots ;
extern const int FrameEndScanline ;

void bus_save_data(FILE *fp);
void bus_load_data(FILE *fp);
void ppu_bus_save_data(FILE *fp);
void ppu_bus_load_data(FILE *fp);

extern int counter;

char szCpuData[10];

void system_save_state() {
    cpu_pause();
    usleep(50000);
    
    printf("Saving State at %0.8X - %0.4X - %0.2X\r\n", counter, cpuData.pc, bus_read(cpuData.pc));
    
    FILE *fp = fopen("/Users/rockypulley/Documents/state.dat", "wb");
    
    if (!fp) {
        printf("Failed to open %d\r\n", errno);
        return;
    }
    
    fwrite(nesData, sizeof(struct NesData), 1, fp);
    
    fwrite(nesData->prgData, 0x4000, nesData->header.prgSize, fp);
    fwrite(nesData->chrData, 0x2000, nesData->header.chrSize, fp);
    
    strcpy(szCpuData, "CPUDATA");
    fwrite(szCpuData, sizeof(szCpuData), 1, fp);
    
    fwrite(&cpuData, sizeof(cpuData), 1, fp);
    fwrite(&ppuData, sizeof(ppuData), 1, fp);
    
    //fwrite(ppuData.pictureBuffer, 240 * 256 * 4, 1, fp);
    
    mapper_get_current()->save(fp);
    bus_save_data(fp);
    ppu_bus_save_data(fp);
    
    fflush(fp);
    fclose(fp);
    
    printf("SAVED\r\n");
    
    cpu_resume();
}

#include <unistd.h>

void system_load_state() {
    char sz[255];
    getcwd(sz, 255);
    printf("CWD: %s\r\n", sz);
    
    FILE *fp = fopen("/Users/rockypulley/Documents/state.dat", "rb");
    
    if (!fp) {
        printf("Failed to open\r\n");
        return;
    }
    
    cpu_pause();
    usleep(50000);
    
    fread(nesData, sizeof(struct NesData), 1, fp);
    
    nesData->prgData = (byte *)malloc(0x4000 * nesData->header.prgSize);
    nesData->chrData = (byte *)malloc(0x2000 * nesData->header.chrSize);
    
    fread(nesData->prgData, 0x4000, nesData->header.prgSize, fp);
    fread(nesData->chrData, 0x2000, nesData->header.chrSize, fp);
    
    fread(szCpuData, sizeof(szCpuData), 1, fp);
    
    printf("CPUDATA: %s\r\n", szCpuData);
    
    fread(&cpuData, sizeof(cpuData), 1, fp);
    fread(&ppuData, sizeof(ppuData), 1, fp);
    
    ppuData.pictureBuffer = (uint32_t **)malloc(sizeof(uint32_t *) * ScanlineVisibleDots);
    
    for (int i=0; i<ScanlineVisibleDots; i++) {
        ppuData.pictureBuffer[i] = (uint32_t *)malloc(sizeof(uint32_t) * VisibleScanlines);
    }
    
    //fread(ppuData.pictureBuffer, 240 * 256 * 4, 1, fp);
    
    mapper_get_current()->load(fp);
    bus_load_data(fp);
    ppu_bus_load_data(fp);
    
    fclose(fp);
    
    printf("Resuming State at %0.8X - %0.4X - %0.2X\r\n", counter, cpuData.pc, bus_read(cpuData.pc));
    
    cpu_resume();
}
