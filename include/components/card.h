#ifndef CARD_H
#define CARD_H


#include "rocks_clay.h"
#include "config.h"
#include <string.h>

typedef void (*RenderContentCallback)();

void RenderCard(const char* title, int card_id, bool* is_minimized, RenderContentCallback render_content);
void HandleMinimizeClick(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData);

#endif