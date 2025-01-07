// state.h
#ifndef STATE_H
#define STATE_H

#include <stdint.h>
#include "../vendor/clay/clay.h" 

// Shared state
extern uint32_t ACTIVE_PAGE;
extern double windowWidth, windowHeight;
extern float globalScalingFactor;
extern float screenBreakpoint;
extern void HandleNavInteraction(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData);

#endif