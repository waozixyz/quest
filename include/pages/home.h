// home.h
#ifndef HOME_PAGE_H
#define HOME_PAGE_H

#include <stdint.h>
#include <stdbool.h>
#include "state/habits_state.h"


extern HabitCollection habits; 

void InitializeHomePage(void);
void CleanupHomePage(void);
void RenderHomePage(void);
void HandleHomePageInput(InputEvent event);

#endif