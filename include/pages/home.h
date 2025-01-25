// home.h
#ifndef HOME_PAGE_H
#define HOME_PAGE_H

#include <stdint.h>
#include <stdbool.h>

#include "components/card.h"
#include "components/progress_bar.h"

void InitializeHomePage();
void CleanupHomePage();
void HandleHomePageInput(InputEvent event);
void RenderHomePage();
float CalculateHabitCompletion(const Habit* habit);

#endif