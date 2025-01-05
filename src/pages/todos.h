#ifndef TODOS_H
#define TODOS_H


#include "../components/text_input.h"
#include "../styles.h"
#include "../events.h"
#include "../state/todos_state.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void InitializeTodosPage(void);
void RenderTodosPage(void);
void CleanupTodosPage(void);

void HandleTodosPageInput(InputEvent event);

#endif