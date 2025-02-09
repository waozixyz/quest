#include "pages/todos.h"
#include "utils.h"
#include "components/modal.h"

#include "rocks.h"
#include "quest_theme.h"

static TodoCollection todo_collection = {0};
static Rocks_TextInput* todo_input = NULL;

static uint32_t pending_delete_todo_id = 0;
static char pending_delete_todo_text[MAX_TODO_TEXT] = {0};

Modal delete_todo_modal = {
   .is_open = false,
   .width = 300,
   .height = 300
};
typedef struct {
    Clay_String url;
    Clay_Dimensions dimensions;
} DaySymbol;

typedef struct {
    const char* url;
    Clay_Dimensions dimensions;
} TodoIcon;

static TodoIcon TODO_ICONS[] = {
    {.url = "images/icons/check.png", .dimensions = {24, 24}},
    {.url = "images/icons/edit.png", .dimensions = {24, 24}},
    {.url = "images/icons/trash.png", .dimensions = {24, 24}}
};

static DaySymbol DAY_SYMBOLS[] = {
    {.url = CLAY_STRING("images/icons/moon.png"), .dimensions = {55, 55}},
    {.url = CLAY_STRING("images/icons/mars.png"), .dimensions = {60, 60}},
    {.url = CLAY_STRING("images/icons/mercury.png"), .dimensions = {55, 55}},
    {.url = CLAY_STRING("images/icons/jupiter.png"), .dimensions = {55, 55}},
    {.url = CLAY_STRING("images/icons/venus.png"), .dimensions = {55, 55}},
    {.url = CLAY_STRING("images/icons/saturn.png"), .dimensions = {60, 60}},
    {.url = CLAY_STRING("images/icons/sun.png"), .dimensions = {55, 55}}
};

static void* todo_icon_images[3] = {NULL};
static void* day_symbol_images[7] = {NULL};


static void OnTodoInputChanged(const char* text) {}
static void OnTodoInputSubmit(const char* text) {
   if (text[0] != '\0') {
       AddTodo(&todo_collection, text);
       Rocks_ClearTextInput(todo_input);
   }
}

void InitializeTodoIcons(Rocks* rocks) {
    for (int i = 0; i < 3; i++) {
        if (todo_icon_images[i]) {
            Rocks_UnloadImage(rocks, todo_icon_images[i]);
            todo_icon_images[i] = NULL;
        }

        todo_icon_images[i] = Rocks_LoadImage(rocks, TODO_ICONS[i].url);
        if (!todo_icon_images[i]) {
            fprintf(stderr, "Failed to load todo icon %s\n", TODO_ICONS[i].url);
            continue;
        }
    }
}

void InitializeDaySymbols(Rocks* rocks) {
    for (int i = 0; i < 7; i++) {
        if (day_symbol_images[i]) {
            Rocks_UnloadImage(rocks, day_symbol_images[i]);
            day_symbol_images[i] = NULL;
        }

        day_symbol_images[i] = Rocks_LoadImage(rocks, DAY_SYMBOLS[i].url.chars);
        if (!day_symbol_images[i]) {
            fprintf(stderr, "Failed to load day symbol %s\n", DAY_SYMBOLS[i].url.chars);
            continue;
        }
    }
}

void InitializeTodosPage(Rocks* rocks) {
    LoadTodos(&todo_collection);
    time_t now = time(NULL);
    struct tm *tm_now = localtime(&now);
    
    int day_index;
    if (tm_now->tm_wday == 0) {
        day_index = 6;
    } else {
        day_index = tm_now->tm_wday - 1;
    }
    
    SetActiveDay(&todo_collection, DAYS[day_index]);
    
    todo_input = Rocks_CreateTextInput(OnTodoInputChanged, OnTodoInputSubmit);
    todo_collection.todo_edit_input = Rocks_CreateTextInput(NULL, NULL);

    InitializeTodoIcons(rocks);
    InitializeDaySymbols(rocks);
}

static void HandleModalConfirm(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData) {
   if (pointerInfo.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
       DeleteTodo(&todo_collection, pending_delete_todo_id);
       delete_todo_modal.is_open = false;
       SaveTodos(&todo_collection);
   }
}

static void HandleModalCancel(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData) {
   if (pointerInfo.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
       delete_todo_modal.is_open = false;
   }
}

void RenderDeleteTodoModalContent() {
   Rocks_Theme base_theme = Rocks_GetTheme(GRocks);
   QuestThemeExtension* theme = (QuestThemeExtension*)base_theme.extension;

   CLAY(CLAY_ID("DeleteTodoModalContent"),
       CLAY_LAYOUT({
           .sizing = { CLAY_SIZING_GROW(), CLAY_SIZING_GROW() },
           .childGap = 24,
           .layoutDirection = CLAY_TOP_TO_BOTTOM
       })
   ) {
       CLAY_TEXT(CLAY_STRING("Delete Todo"), 
           CLAY_TEXT_CONFIG({
               .fontSize = 24,
               .fontId = FONT_ID_BODY_24,
               .textColor = base_theme.text
           })
       );

       CLAY_TEXT(CLAY_STRING("Are you sure you want to delete:"),
           CLAY_TEXT_CONFIG({
               .fontSize = 16,
               .fontId = FONT_ID_BODY_16,
               .textColor = base_theme.text
           })
       );

       CLAY(CLAY_LAYOUT({
           .padding = { 16, 16, 16, 16 },
           .childAlignment = { .x = CLAY_ALIGN_X_CENTER }
       }),
       CLAY_RECTANGLE({
           .color = theme->card,
           .cornerRadius = CLAY_CORNER_RADIUS(4)
       })) {
           Clay_String todo_text = {
               .length = strlen(pending_delete_todo_text),
               .chars = pending_delete_todo_text
           };
           CLAY_TEXT(todo_text,
               CLAY_TEXT_CONFIG({
                   .fontSize = 18,
                   .fontId = FONT_ID_BODY_16,
                   .textColor = base_theme.text
               })
           );
       }

       CLAY(CLAY_LAYOUT({
           .layoutDirection = CLAY_LEFT_TO_RIGHT,
           .childGap = 8,
           .childAlignment = { .x = CLAY_ALIGN_X_RIGHT }
       })) {
           CLAY(CLAY_ID("CancelButton"),
               CLAY_LAYOUT({
                   .padding = { 8, 8, 8, 8 },
                   .sizing = { CLAY_SIZING_FIT(0), CLAY_SIZING_FIT(0) }
               }),
               CLAY_RECTANGLE({
                   .color = theme->card,
                   .cornerRadius = CLAY_CORNER_RADIUS(4),
                   .cursorPointer = true
               }),
               Clay_OnHover(HandleModalCancel, 0)
           ) {
               CLAY_TEXT(CLAY_STRING("Cancel"),
                   CLAY_TEXT_CONFIG({
                       .fontSize = 16,
                       .fontId = FONT_ID_BODY_16,
                       .textColor = base_theme.text
                   })
               );
           }

           CLAY(CLAY_ID("ConfirmDeleteButton"),
               CLAY_LAYOUT({
                   .padding = { 8, 8, 8, 8 },
                   .sizing = { CLAY_SIZING_FIT(0), CLAY_SIZING_FIT(0) }
               }),
               CLAY_RECTANGLE({
                   .color = theme->danger,
                   .cornerRadius = CLAY_CORNER_RADIUS(4),
                   .cursorPointer = true
               }),
               Clay_OnHover(HandleModalConfirm, 0)
           ) {
               CLAY_TEXT(CLAY_STRING("Delete"),
                   CLAY_TEXT_CONFIG({
                       .fontSize = 16,
                       .fontId = FONT_ID_BODY_16,
                       .textColor = base_theme.text
                   })
               );
           }
       }
   }
}

void RenderDeleteTodoModal(void) {
   Rocks_Theme base_theme = Rocks_GetTheme(GRocks);
   QuestThemeExtension* theme = (QuestThemeExtension*)base_theme.extension;

   if (!delete_todo_modal.is_open) return;
   
   CLAY(CLAY_ID("DeleteTodoModalOverlay"),
       CLAY_LAYOUT({
           .sizing = { CLAY_SIZING_GROW(), CLAY_SIZING_GROW() }
       }),
       CLAY_FLOATING({
           .parentId = Clay__HashString(CLAY_STRING("TodosContainer"), 0, 0).id,
           .attachment = { 
               .element = CLAY_ATTACH_POINT_CENTER_CENTER,
               .parent = CLAY_ATTACH_POINT_CENTER_CENTER
           },
           .zIndex = 1000
       }),
       CLAY_RECTANGLE({
           .color = { 0, 0, 0, 128 }
       })
   ) {}

   CLAY(CLAY_ID("DeleteTodoModalContent"),
       CLAY_LAYOUT({
           .sizing = { 
               CLAY_SIZING_FIXED(delete_todo_modal.width), 
               CLAY_SIZING_FIXED(delete_todo_modal.height) 
           },
           .padding = { 24, 24, 24, 24 }
       }),
       CLAY_FLOATING({
           .parentId = Clay__HashString(CLAY_STRING("TodosContainer"), 0, 0).id,
           .attachment = { 
               .element = CLAY_ATTACH_POINT_CENTER_CENTER,
               .parent = CLAY_ATTACH_POINT_CENTER_CENTER
           },
           .zIndex = 1001
       }),
       CLAY_RECTANGLE({
           .color = base_theme.background,
           .cornerRadius = CLAY_CORNER_RADIUS(8)
       })
   ) {
       RenderDeleteTodoModalContent();
   }
}


static void HandleTabInteraction(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData) {
   if (pointerInfo.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
       SetActiveDay(&todo_collection, DAYS[(int)userData]);
       SaveTodos(&todo_collection);
   }
}
void RenderTodoTab(const char* day, const DaySymbol* symbol, bool active, int index) {
    Rocks_Theme base_theme = Rocks_GetTheme(GRocks);
    QuestThemeExtension* theme = (QuestThemeExtension*)base_theme.extension;

    char id_buffer[32];
    int horizontal_padding = windowWidth >= BREAKPOINT_MEDIUM ? 32 : 16;

    CLAY(Clay__AttachId(Clay__HashString((Clay_String){.chars = id_buffer, .length = strlen(id_buffer)}, index, 0)),
        CLAY_LAYOUT({ 
            .padding = { horizontal_padding, horizontal_padding, 8, 8 }, 
            .childGap = 8,
            .childAlignment = { CLAY_ALIGN_X_CENTER, CLAY_ALIGN_Y_CENTER }
        }),
        CLAY_BORDER({ .betweenChildren = { 1, theme->border }}),
        CLAY_RECTANGLE({ 
            .color = active ? base_theme.secondary : 
                     (Clay_Hovered() ? base_theme.primary_hover : base_theme.background),
            .cornerRadius = CLAY_CORNER_RADIUS(4),
            .cursorPointer = true
        }),
        Clay_OnHover(HandleTabInteraction, index)
    ) {
        CLAY(CLAY_LAYOUT({ 
            .sizing = { 
                CLAY_SIZING_FIXED(symbol->dimensions.width), 
                CLAY_SIZING_FIXED(symbol->dimensions.height)
            }
        }),
        CLAY_IMAGE({ 
            .sourceDimensions = symbol->dimensions,
            .imageData = day_symbol_images[index]
        })) {}
    }
}

static void HandleSubmitButtonClick(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData) {
   if (pointerInfo.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
       OnTodoInputSubmit(Rocks_GetTextInputText(todo_input));
   }
}

static void HandleEditTodoButtonClick(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData) {
   if (pointerInfo.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
       todo_collection.editing_todo_id = (uint32_t)userData;
       
       for (size_t i = 0; i < todo_collection.todos_count; i++) {
           if (todo_collection.todos[i].id == todo_collection.editing_todo_id) {
               Rocks_SetTextInputText(todo_collection.todo_edit_input, todo_collection.todos[i].text);
               break;
           }
       }
       
       #ifdef CLAY_MOBILE
       SDL_StartTextInput();
       #endif
   }
}

static void HandleTodoEditConfirm(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData) {
   if (pointerInfo.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
       const char* new_text = Rocks_GetTextInputText(todo_collection.todo_edit_input);
       if (new_text[0] != '\0') {
           for (size_t i = 0; i < todo_collection.todos_count; i++) {
if (todo_collection.todos[i].id == todo_collection.editing_todo_id) {
                   strncpy(todo_collection.todos[i].text, new_text, MAX_TODO_TEXT - 1);
                   todo_collection.todos[i].text[MAX_TODO_TEXT - 1] = '\0';
                   break;
               }
           }
           SaveTodos(&todo_collection);
       }
       todo_collection.editing_todo_id = 0;
       Rocks_ClearTextInput(todo_collection.todo_edit_input);
       
       #ifdef CLAY_MOBILE
       SDL_StopTextInput();
       #endif
   }
}

static void HandleTodoDeleteClick(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData) {
   if (pointerInfo.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
       uint32_t todo_id = (uint32_t)userData;
       pending_delete_todo_id = todo_id;
       for (size_t i = 0; i < todo_collection.todos_count; i++) {
           if (todo_collection.todos[i].id == todo_id) {
               strncpy(pending_delete_todo_text, todo_collection.todos[i].text, MAX_TODO_TEXT - 1);
               break;
           }
       }
       delete_todo_modal.is_open = true;
       
       #ifdef CLAY_MOBILE
       SDL_StopTextInput();
       #endif
   }
}

static void HandleTodoCompleteClick(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData) {
   if (pointerInfo.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
       uint32_t todo_id = (uint32_t)userData;
       ToggleTodo(&todo_collection, todo_id);
       SaveTodos(&todo_collection);
   }
}
void RenderTodoItem(const Todo* todo, int index) {
    Rocks_Theme base_theme = Rocks_GetTheme(GRocks);
    QuestThemeExtension* theme = (QuestThemeExtension*)base_theme.extension;

    char id_buffer[32];
    snprintf(id_buffer, sizeof(id_buffer), "TodoItem_%d", todo->id);
    
    bool is_editing = todo_collection.editing_todo_id == todo->id;
    
    CLAY(Clay__AttachId(Clay__HashString((Clay_String){.chars = id_buffer, .length = strlen(id_buffer)}, index, 0)),
        CLAY_LAYOUT({ 
            .sizing = { CLAY_SIZING_GROW(), CLAY_SIZING_FIT(0) },
            .padding = { 16, 16, 8, 8 },
            .childGap = 16,
            .childAlignment = { CLAY_ALIGN_X_LEFT, CLAY_ALIGN_Y_CENTER },
            .layoutDirection = CLAY_LEFT_TO_RIGHT
        }),
        CLAY_RECTANGLE({ 
            .color = todo->completed ? base_theme.primary : theme->card,
            .cornerRadius = CLAY_CORNER_RADIUS(4)
        })
    ) {
        CLAY(CLAY_LAYOUT({
            .sizing = { CLAY_SIZING_GROW(), CLAY_SIZING_FIT(0) },
            .childAlignment = { CLAY_ALIGN_X_LEFT, CLAY_ALIGN_Y_CENTER }
        })) {
            if (is_editing) {
                CLAY(CLAY_LAYOUT({
                    .sizing = { CLAY_SIZING_GROW(), CLAY_SIZING_FIT(0) }
                })) {
                    Rocks_RenderTextInput(todo_collection.todo_edit_input, todo->id);
                }
            } else {
                Clay_String todo_text = { .chars = todo->text, .length = strlen(todo->text) };
                CLAY_TEXT(todo_text, 
                    CLAY_TEXT_CONFIG({ 
                        .fontSize = 16, 
                        .textColor = base_theme.text 
                    }));
            }
        }

        CLAY(CLAY_LAYOUT({
            .childGap = 8,
            .layoutDirection = CLAY_LEFT_TO_RIGHT
        })) {
            if (is_editing) {
                CLAY(CLAY_IDI("ConfirmEditButton", todo->id),
                    CLAY_LAYOUT({
                        .sizing = { CLAY_SIZING_FIXED(32), CLAY_SIZING_FIXED(32) },
                        .childAlignment = { .x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_CENTER }
                    }),
                    CLAY_RECTANGLE({
                        .color = Clay_Hovered() ? base_theme.primary : base_theme.secondary,
                        .cornerRadius = CLAY_CORNER_RADIUS(4),
                        .cursorPointer = true
                    }),
                    Clay_OnHover(HandleTodoEditConfirm, todo->id)
                ) {
                    CLAY(CLAY_LAYOUT({
                        .sizing = { CLAY_SIZING_FIXED(24), CLAY_SIZING_FIXED(24) }
                    }),
                    CLAY_IMAGE({
                        .sourceDimensions = TODO_ICONS[0].dimensions,
                        .imageData = todo_icon_images[0]
                    })) {}
                }

                CLAY(CLAY_IDI("DeleteTodoButton", todo->id),
                    CLAY_LAYOUT({
                        .sizing = { CLAY_SIZING_FIXED(32), CLAY_SIZING_FIXED(32) },
                        .childAlignment = { .x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_CENTER }
                    }),
                    CLAY_RECTANGLE({
                        .color = Clay_Hovered() ? theme->danger : theme->card,
                        .cornerRadius = CLAY_CORNER_RADIUS(4),
                        .cursorPointer = true
                    }),
                    Clay_OnHover(HandleTodoDeleteClick, todo->id)
                ) {
                    CLAY(CLAY_LAYOUT({
                        .sizing = { CLAY_SIZING_FIXED(24), CLAY_SIZING_FIXED(24) }
                    }),
                    CLAY_IMAGE({
                        .sourceDimensions = TODO_ICONS[2].dimensions,
                        .imageData = todo_icon_images[2]
                    })) {}
                }
            } else {
                CLAY(CLAY_IDI("EditTodoButton", todo->id),
                    CLAY_LAYOUT({
                        .sizing = { CLAY_SIZING_FIXED(32), CLAY_SIZING_FIXED(32) },
                        .childAlignment = { .x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_CENTER }
                    }),
                    CLAY_RECTANGLE({
                        .color = Clay_Hovered() ? base_theme.primary_hover : theme->card,
                        .cornerRadius = CLAY_CORNER_RADIUS(4),
                        .cursorPointer = true
                    }),
                    Clay_OnHover(HandleEditTodoButtonClick, todo->id)
                ) {
                    CLAY(CLAY_LAYOUT({
                        .sizing = { CLAY_SIZING_FIXED(24), CLAY_SIZING_FIXED(24) }
                    }),
                    CLAY_IMAGE({
                        .sourceDimensions = TODO_ICONS[1].dimensions,
                        .imageData = todo_icon_images[1]
                    })) {}
                }
                
                CLAY(CLAY_IDI("CompleteTodoButton", todo->id),
                    CLAY_LAYOUT({
                        .sizing = { CLAY_SIZING_FIXED(32), CLAY_SIZING_FIXED(32) },
                        .childAlignment = { .x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_CENTER }
                    }),
                    CLAY_RECTANGLE({
                        .color = todo->completed ? base_theme.primary : 
                                (Clay_Hovered() ? base_theme.primary : theme->card),
                        .cornerRadius = CLAY_CORNER_RADIUS(4),
                        .cursorPointer = true
                    }),
                    Clay_OnHover(HandleTodoCompleteClick, todo->id)
                ) {
                    CLAY(CLAY_LAYOUT({
                        .sizing = { CLAY_SIZING_FIXED(24), CLAY_SIZING_FIXED(24) }
                    }),
                    CLAY_IMAGE({
                        .sourceDimensions = TODO_ICONS[0].dimensions,
                        .imageData = todo_icon_images[0]
                    })) {}
                }
            }
        }
    }
}


void RenderTodosPage(float dt) {
    Rocks_Theme base_theme = Rocks_GetTheme(GRocks);
    QuestThemeExtension* theme = (QuestThemeExtension*)base_theme.extension;
 

    if (todo_input) {
        Rocks_UpdateTextInputFromRocksInput(todo_input, GRocks->input, dt);
    }
    if (todo_collection.todo_edit_input && todo_collection.editing_todo_id) {
        Rocks_UpdateTextInputFromRocksInput(todo_collection.todo_edit_input, GRocks->input, dt);
    }
        
    CLAY(CLAY_ID("TodosContainer"), 
        CLAY_LAYOUT({ 
            .sizing = { CLAY_SIZING_GROW(), CLAY_SIZING_GROW() },
            .padding = { 32, 32, 32, 32 },
            .childGap = 24,
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
            .childAlignment = { .x = CLAY_ALIGN_X_CENTER } 
        })
    ) {
        // Render the day tabs
        CLAY(CLAY_LAYOUT({ 
            .sizing = { 
                windowWidth > BREAKPOINT_MEDIUM + 40 ? 
                    CLAY_SIZING_FIXED(BREAKPOINT_MEDIUM + 40) : 
                    CLAY_SIZING_GROW(),
                CLAY_SIZING_FIT(0)
            },
            .childGap = 8,
            .layoutDirection = CLAY_LEFT_TO_RIGHT,
            .childAlignment = { .x = CLAY_ALIGN_X_CENTER }
        })) {
            for (int i = 0; i < 7; i++) {
                RenderTodoTab(DAYS[i], &DAY_SYMBOLS[i], 
                    strcmp(DAYS[i], todo_collection.active_day) == 0, i);
            }
        }

        // Render the input field and submit button
        if (todo_input) {
            CLAY(CLAY_LAYOUT({
                .sizing = {
                    windowWidth > BREAKPOINT_MEDIUM + 40 ? 
                        CLAY_SIZING_FIXED(BREAKPOINT_MEDIUM + 40) : 
                        CLAY_SIZING_GROW(),
                    CLAY_SIZING_FIT(0)
                },
                .layoutDirection = CLAY_LEFT_TO_RIGHT,
                .childGap = 8
            })) {
                CLAY(CLAY_LAYOUT({
                    .sizing = { CLAY_SIZING_GROW(), CLAY_SIZING_FIT(0) }
                })) {
                    Rocks_RenderTextInput(todo_input, 1);
                }

                CLAY(CLAY_ID("SubmitTodoButton"),
                    CLAY_LAYOUT({
                        .sizing = { CLAY_SIZING_FIXED(32), CLAY_SIZING_FIXED(32) },
                        .childAlignment = { .x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_CENTER }
                    }),
                    CLAY_RECTANGLE({
                        .color = Clay_Hovered() ? base_theme.primary : base_theme.secondary,
                        .cornerRadius = CLAY_CORNER_RADIUS(4),
                        .cursorPointer = true
                    }),
                    Clay_OnHover(HandleSubmitButtonClick, 0)
                ) {
                    CLAY(CLAY_LAYOUT({
                        .sizing = { CLAY_SIZING_FIXED(24), CLAY_SIZING_FIXED(24) }
                    }),
                    CLAY_IMAGE({
                        .sourceDimensions = TODO_ICONS[0].dimensions,
                        .imageData = todo_icon_images[0]  // Use the "check" icon
                    })) {}
                }
            }
        }

        // Render the list of todos
        CLAY(CLAY_ID("TodosScrollContainer"),
            CLAY_LAYOUT({ 
                .sizing = { 
                    windowWidth > BREAKPOINT_MEDIUM + 40 ? 
                        CLAY_SIZING_FIXED(BREAKPOINT_MEDIUM + 40) : 
                        CLAY_SIZING_GROW(),
                    CLAY_SIZING_GROW() 
                },
                .childGap = 8,
                .layoutDirection = CLAY_TOP_TO_BOTTOM
            }),
            CLAY_SCROLL({ .vertical = true })
        ) {
            size_t todos_count;
            Todo* todos = GetTodosByDay(&todo_collection, 
                                    todo_collection.active_day, 
                                    &todos_count);

            for (size_t i = 0; i < todos_count; i++) {
                if (!todos[i].completed) {
                    RenderTodoItem(&todos[i], (int)i);
                }
            }
        }
    }
    
    RenderDeleteTodoModal();
}

void CleanupTodoIcons(Rocks* rocks) {
    for (int i = 0; i < 3; i++) {
        if (todo_icon_images[i]) {
            Rocks_UnloadImage(rocks, todo_icon_images[i]);
            todo_icon_images[i] = NULL;
        }
    }
}

void CleanupDaySymbols(Rocks* rocks) {
    for (int i = 0; i < 7; i++) {
        if (day_symbol_images[i]) {
            Rocks_UnloadImage(rocks, day_symbol_images[i]);
            day_symbol_images[i] = NULL;
        }
    }
}

void CleanupTodosPage(Rocks* rocks) {
    if (todo_input) {
        Rocks_DestroyTextInput(todo_input);
        todo_input = NULL;
    }
    if (todo_collection.todo_edit_input) {
        Rocks_DestroyTextInput(todo_collection.todo_edit_input);
        todo_collection.todo_edit_input = NULL;
    }

    CleanupTodoIcons(rocks);
    CleanupDaySymbols(rocks);
}