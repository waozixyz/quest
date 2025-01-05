#ifndef CALENDAR_BOX_H
#define CALENDAR_BOX_H

#include <stdbool.h>
#include "../../vendor/clay/clay.h"
#include "../styles.h"
#include <stdio.h>
#include <math.h>

// Struct to hold calendar box configuration
typedef struct {
    int day_number;
    bool is_today;
    bool is_past;
    int unique_index;
    bool is_completed;
    void (*on_click)(Clay_ElementId, Clay_PointerData, intptr_t);
    Clay_Color custom_color;
} CalendarBoxProps;

// Function to render a calendar box
void RenderCalendarBox(CalendarBoxProps props);

#endif // CALENDAR_BOX_H
