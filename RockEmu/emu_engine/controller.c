//
//  controller.c
//  Emu6502
//
//  Created by Rocky Pulley on 8/27/19.
//  Copyright © 2019 Rocky Pulley. All rights reserved.
//

#include "controller.h"

const int BUTTON_A = (1 << 0);
const int BUTTON_B = (1 << 1);
const int BUTTON_SE = (1 << 2);
const int BUTTON_ST = (1 << 3);
const int BUTTON_U = (1 << 4);
const int BUTTON_D = (1 << 5);
const int BUTTON_L = (1 << 6);
const int BUTTON_R = (1 << 7);

void ignoreLog(const char *fmt, ...) {
    
}

uint8_t controller_data[] = {0, 0};
bool controller_strobe = false;

uint8_t controller_read(uint8_t n) {
    if (controller_strobe) {
        return controller_data[n] & BUTTON_A;
    }
    
    uint8_t v = controller_data[n] & 1;
    controller_data[n] >>= 1;
    
    return v | 0x40;
}

void controller_write(uint8_t b) {
    controller_strobe = b & 1;
}

void controller_set(uint8_t n, uint8_t b, uint8_t val) {
    printf("controller_set called: %0.2X, %0.2X, %0.2X\r\n", n, b, val);
    if (val) {
        controller_data[n] |= b;
    } else {
        controller_data[n] &= ~b;
    }
}

void controller_button_down(uint8_t n, uint8_t b) {
    
}

void controller_button_up(uint8_t n, uint8_t b) {
    
}
