#include "pages/todos.h"

static TodoCollection todo_collection = {0};
static TextInput* todo_input = NULL;
static char* DAYS[] = {"Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"};
static char* DAY_SYMBOLS[] = {"🌙", "♂", "☿", "♃", "♀", "♄", "☉"};

static void OnTodoInputChanged(const char* text) {
    // Handle input changes if needed
}

static void OnTodoInputSubmit(const char* text) {
    if (text[0] != '\0') {
        AddTodo(&todo_collection, text);
        ClearTextInput(todo_input);
    }
}

void InitializeTodosPage() {
    LoadTodos(&todo_collection);
    todo_input = CreateTextInput(OnTodoInputChanged, OnTodoInputSubmit);
}

static void HandleTabInteraction(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData) {
    if (pointerInfo.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        SetActiveDay(&todo_collection, DAYS[(int)userData]);
        SaveTodos(&todo_collection);
    }
}


void RenderTodoTab(const char* day, const char* symbol, bool active, int index) {
    char id_buffer[32];
    snprintf(id_buffer, sizeof(id_buffer), "TodoTab_%s", day);
    
    CLAY(CLAY_IDI(id_buffer, index),
        CLAY_LAYOUT({ 
            .padding = { 16, 8 }, 
            .childGap = 8,
            .childAlignment = { CLAY_ALIGN_X_CENTER, CLAY_ALIGN_Y_CENTER }
        }),
        CLAY_BORDER({ .betweenChildren = { 1, COLOR_BORDER }}),
        CLAY_RECTANGLE({ 
            .color = active ? COLOR_SECONDARY : 
                     (Clay_Hovered() ? COLOR_PRIMARY_HOVER : COLOR_BACKGROUND),
            .cornerRadius = CLAY_CORNER_RADIUS(4)
        }),
        Clay_OnHover(HandleTabInteraction, index)
    ) {
        Clay_String symbol_str = { .chars = symbol, .length = strlen(symbol) };
        Clay_String day_str = { .chars = day, .length = strlen(day) };
        
        CLAY_TEXT(symbol_str, 
            CLAY_TEXT_CONFIG({ .fontSize = 24, .textColor = COLOR_TEXT }));
        CLAY_TEXT(day_str, 
            CLAY_TEXT_CONFIG({ .fontSize = 16, .textColor = COLOR_TEXT }));
    }
}

void RenderTodoItem(const Todo* todo, int index) {
    char id_buffer[32];
    snprintf(id_buffer, sizeof(id_buffer), "TodoItem_%d", todo->id);
    
    CLAY(CLAY_IDI(id_buffer, index),
        CLAY_LAYOUT({ 
            .padding = { 16, 8 },
            .childGap = 16,
            .childAlignment = { CLAY_ALIGN_X_LEFT, CLAY_ALIGN_Y_CENTER }
        }),
        CLAY_RECTANGLE({ 
            .color = todo->completed ? COLOR_SUCCESS : COLOR_CARD,
            .cornerRadius = CLAY_CORNER_RADIUS(4)
        })
    ) {
        Clay_String todo_text = { .chars = todo->text, .length = strlen(todo->text) };
        CLAY_TEXT(todo_text, 
            CLAY_TEXT_CONFIG({ 
                .fontSize = 16, 
                .textColor = COLOR_TEXT 
            }));
    }
}

void RenderTodosPage() {
    CLAY(CLAY_ID("TodosContainer"), 
        CLAY_LAYOUT({ 
            .sizing = { CLAY_SIZING_GROW(), CLAY_SIZING_GROW() },
            .padding = { 32, 32 },
            .childGap = 24,
            .layoutDirection = CLAY_TOP_TO_BOTTOM
        })
    ) {
        // Render tabs
        CLAY(CLAY_LAYOUT({ 
            .childGap = 8,
            .layoutDirection = CLAY_LEFT_TO_RIGHT
        })) {
            for (int i = 0; i < 7; i++) {
                RenderTodoTab(DAYS[i], DAY_SYMBOLS[i], 
                    strcmp(DAYS[i], todo_collection.active_day) == 0, i);
            }
        }

        // Todo input
        if (todo_input) {
            RenderTextInput(todo_input, 1); // Use numeric ID instead of string
        }

        // Todo list
        size_t todos_count;
        Todo* todos = GetTodosByDay(&todo_collection, 
                                  todo_collection.active_day, 
                                  &todos_count);

        CLAY(CLAY_LAYOUT({ 
            .sizing = { CLAY_SIZING_GROW(), CLAY_SIZING_GROW() },
            .childGap = 8,
            .layoutDirection = CLAY_TOP_TO_BOTTOM
        })) {
            for (size_t i = 0; i < todos_count; i++) {
                RenderTodoItem(&todos[i], (int)i);
            }
        }
    }
}

void HandleTodosPageInput(InputEvent event) {
    if (!todo_input) return;

    // Update text input
    if (event.delta_time > 0) {
        UpdateTextInput(todo_input, 0, event.delta_time);
    }

    // Handle text input
    if (event.isTextInput) {
        UpdateTextInput(todo_input, event.text[0], event.delta_time);
    } else {
        UpdateTextInput(todo_input, event.key, event.delta_time);
    }
}

void CleanupTodosPage() {
    if (todo_input) {
        DestroyTextInput(todo_input);
        todo_input = NULL;
    }
}