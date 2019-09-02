//
//  controller.h
//  Emu6502
//
//  Created by Rocky Pulley on 8/27/19.
//  Copyright © 2019 Rocky Pulley. All rights reserved.
//

#ifndef controller_h
#define controller_h

#include "common.h"

extern const int BUTTON_A ;
extern const int BUTTON_B ;
extern const int BUTTON_SE;
extern const int BUTTON_ST;
extern const int BUTTON_U ;
extern const int BUTTON_D ;
extern const int BUTTON_L ;
extern const int BUTTON_R ;

uint8_t controller_read(uint8_t n);
void controller_write(uint8_t b);
void controller_set(uint8_t n, uint8_t b, uint8_t val);
void controller_button_down(uint8_t n, uint8_t b);
void controller_button_up(uint8_t n, uint8_t b);

#endif /* controller_h */
