#ifndef HOME_CARD_H
#define HOME_CARD_H

#include <stdint.h>

// Clay configuration
#define CLAY_EXTEND_CONFIG_RECTANGLE Clay_String link; bool cursorPointer;
#define CLAY_EXTEND_CONFIG_IMAGE Clay_String sourceURL;
#define CLAY_EXTEND_CONFIG_TEXT bool disablePointerEvents;

#include "../../vendor/clay/clay.h"
#include "../state.h"
#include "../styles.h"
#include "../state.h"  


void RenderHomeCard(uint32_t index);

#endif