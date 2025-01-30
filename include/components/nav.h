#ifndef NAV_H
#define NAV_H

#include "../config.h"
#include <string.h>

#include "rocks.h"
#include "rocks_clay.h"

void InitializeNavIcons(Rocks* rocks);
void CleanupNavIcons(Rocks* rocks);
void RenderNavigationMenu(Rocks* rocks);

#endif