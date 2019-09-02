//
//  instructions.cpp
//  Emu6502
//
//  Created by Rocky Pulley on 8/26/19.
//  Copyright © 2019 Rocky Pulley. All rights reserved.
//

#include "instructions.h"

#include <stdio.h>
#include <string>
#include <map>

std::map<const std::string, handle_inst> handler_map;

void inst_add_handler(const char *name, handle_inst handler) {
    handler_map.insert(std::pair<const std::string, handle_inst>(std::string(name), handler));
}

handle_inst inst_get_handler(const char *name) {
    if (handler_map.find(std::string(name)) != handler_map.end()) {
        return handler_map[std::string(name)];
    }
    
    return 0;
}
