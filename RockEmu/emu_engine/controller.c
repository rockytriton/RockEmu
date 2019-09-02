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

bool keyPressed[] = {
    false,
    false,
    false,
    false,
    false,
    false,
    false,
    false
};

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
    
    if (!controller_strobe) {
        uint8_t keyStates = 0;
        int shift = 0;
        
        for (int i=0; i<8; i++) {
            keyStates |= (keyPressed[i] << shift);
            shift++;
        }
        
        printf("CHANGING STATE: %0.2X, %0.2X\r\n", controller_data[0], keyStates);
        controller_data[0] = keyStates;
    }
}

void controller_set(uint8_t n, uint8_t b, uint8_t val) {
    printf("controller_set called: %0.2X, %0.2X, %0.2X\r\n", n, b, val);
    if (val) {
        controller_data[n] |= b;
    } else {
        controller_data[n] &= ~b;
    }
    
    if (b == BUTTON_A) keyPressed[0] = val;
    if (b == BUTTON_B) keyPressed[1] = val;
    if (b == BUTTON_SE) keyPressed[2] = val;
    if (b == BUTTON_ST) keyPressed[3] = val;
    if (b == BUTTON_U) keyPressed[4] = val;
    if (b == BUTTON_D) keyPressed[5] = val;
    if (b == BUTTON_L) keyPressed[6] = val;
    if (b == BUTTON_R) keyPressed[7] = val;
    
}

void controller_button_down(uint8_t n, uint8_t b) {
    
}

void controller_button_up(uint8_t n, uint8_t b) {
    
}
