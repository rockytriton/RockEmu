//
//  system.h
//  RockEmu
//
//  Created by Rocky Pulley on 9/4/19.
//  Copyright © 2019 Rocky Pulley. All rights reserved.
//

#ifndef system_h
#define system_h

#include "common.h"

void system_save_state(void);
void system_load_state(void);

void system_select_bank(byte bank);
byte system_get_bank(void);

#endif /* system_h */
