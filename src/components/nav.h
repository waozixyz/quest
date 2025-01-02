#ifndef NAV_H
#define NAV_H

// Clay configuration
#define CLAY_EXTEND_CONFIG_RECTANGLE Clay_String link; bool cursorPointer;
#define CLAY_EXTEND_CONFIG_IMAGE Clay_String sourceURL;
#define CLAY_EXTEND_CONFIG_TEXT bool disablePointerEvents;

#include "../../vendor/clay/clay.h"
#include "../state.h"
#include "../styles.h"

void RenderNavigationMenu(void);

#endif
