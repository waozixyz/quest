#ifndef PROGRESS_BAR_H
#define PROGRESS_BAR_H


#include "../clay_extensions.h"
#include "clay.h"
#include "config.h"
#include "utils.h"
#include "state/habits_state.h"

#include <string.h>

extern HabitCollection habits; 

void RenderProgressBar(float completion, Clay_Color color);
void RenderHabitProgressBar(const Habit* habit, float completion);

#endif
