//
//  timing.h
//  RockEmu
//
//  Created by Rocky Pulley on 9/2/19.
//  Copyright © 2019 Rocky Pulley. All rights reserved.
//

#ifndef timing_h
#define timing_h

#include "common.h"

#ifdef __cplusplus
extern "C"
{
#endif
void timer_init();
void timer_update();
uint8_t timer_loop();
void timer_cycle();
    
    void startProfile();
    void endProfile();
    
#ifdef __cplusplus
}
#endif
#endif /* timing_h */
