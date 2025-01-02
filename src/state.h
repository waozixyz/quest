#ifndef STATE_H
#define STATE_H

#include <stdint.h>

// Shared state
extern uint32_t ACTIVE_PAGE;
extern void HandleNavInteraction(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData);

#endif
