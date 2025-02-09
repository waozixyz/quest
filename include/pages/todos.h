#ifndef TODOS_H
#define TODOS_H
#include "rocks.h"
#include "rocks_clay.h"
#include "../components/text_input.h"
#include "../config.h"
#include "../state/todos_state.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void InitializeTodosPage(Rocks* rocks);
void RenderTodosPage(float dt);
void CleanupTodosPage(Rocks* rocks);
void HandleTodosPageInput(InputEvent event);

#endif