//
//  timing.c
//  RockEmu
//
//  Created by Rocky Pulley on 9/2/19.
//  Copyright © 2019 Rocky Pulley. All rights reserved.
//

#include "timing.h"
#include <iostream>
#include <chrono>

using TimePoint = std::chrono::high_resolution_clock::time_point;

std::chrono::high_resolution_clock::duration m_elapsedTime;
std::chrono::nanoseconds m_cpuCycleDuration = std::chrono::nanoseconds(59);
//std::chrono::nanoseconds m_cpuCycleDuration = std::chrono::microseconds(500);
std::chrono::nanoseconds longTime = std::chrono::nanoseconds(2059);

TimePoint m_cycleTimer;
TimePoint profileTimer;
std::chrono::high_resolution_clock::duration profileDuration;

void startProfile() {
    profileTimer = std::chrono::high_resolution_clock::now();
}

extern "C" int counter;
extern "C" int lastOpCode;

void endProfile() {
    profileDuration = std::chrono::high_resolution_clock::now() - profileTimer;
    
    if (counter > 0 && profileDuration > m_cpuCycleDuration) {
        //printf("%0.8X: %0.2X\r\n", counter, lastOpCode);
        //printf("HIGH DURATION\r\n");
    }
}

void timer_init() {
    m_cycleTimer = std::chrono::high_resolution_clock::now();
    m_elapsedTime = m_cycleTimer - m_cycleTimer;
}

void timer_update() {
    
    std::chrono::high_resolution_clock::duration next = std::chrono::high_resolution_clock::now() - m_cycleTimer;
    
    m_elapsedTime += next;
    
    m_cycleTimer = std::chrono::high_resolution_clock::now();
    
    
}

uint8_t timer_loop() {
    return m_elapsedTime > m_cpuCycleDuration;
}

void timer_cycle() {
    m_elapsedTime -= m_cpuCycleDuration;
}
