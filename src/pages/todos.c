#include "pages/todos.h"
#include "utils.h"
#include "components/modal.h"

static TodoCollection todo_collection = {0};
static TextInput* todo_input = NULL;
static char* DAYS[] = {"Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"};

static uint32_t pending_delete_todo_id = 0;
static char pending_delete_todo_text[MAX_TODO_TEXT] = {0};


Modal delete_todo_modal = {
    .is_open = false,
    .width = 300,
    .height = 300
};

#ifdef __EMSCRIPTEN__
#define ICON_PATH "/images/icons/"

typedef struct {
    Clay_String url;
    Clay_Dimensions dimensions;
} DaySymbol;

#else
#define ICON_PATH "images/icons/"

typedef struct {
    Clay_String url;
    Clay_Dimensions dimensions;
} DaySymbol;

static SDL_Texture* day_symbols_textures[7] = {NULL};


#endif

static DaySymbol DAY_SYMBOLS[] = {
    {.url = CLAY_STRING("icons/moon.png"), .dimensions = {55, 55}},
    {.url = CLAY_STRING("icons/mars.png"), .dimensions = {60, 60}},
    {.url = CLAY_STRING("icons/mercury.png"), .dimensions = {55, 55}},
    {.url = CLAY_STRING("icons/jupiter.png"), .dimensions = {55, 55}},
    {.url = CLAY_STRING("icons/venus.png"), .dimensions = {55, 55}},
    {.url = CLAY_STRING("icons/saturn.png"), .dimensions = {60, 60}},
    {.url = CLAY_STRING("icons/sun.png"), .dimensions = {55, 55}}
};

static void OnTodoInputChanged(const char* text) {
    // Handle input changes if needed
}

static void OnTodoInputSubmit(const char* text) {
    if (text[0] != '\0') {
        AddTodo(&todo_collection, text);
        ClearTextInput(todo_input);
    }
}

#ifdef __EMSCRIPTEN__
void InitializeTodosPage(void) {
    LoadTodos(&todo_collection);
    todo_input = CreateTextInput(OnTodoInputChanged, OnTodoInputSubmit);
    todo_collection.todo_edit_input = CreateTextInput(NULL, NULL);

}
#else
static SDL_Texture* check_texture = NULL;
static SDL_Texture* edit_texture = NULL;
static SDL_Texture* trash_texture = NULL;

void InitializeTodosPage(SDL_Renderer* renderer) {
    LoadTodos(&todo_collection);
    todo_input = CreateTextInput(OnTodoInputChanged, OnTodoInputSubmit);
    todo_collection.todo_edit_input = CreateTextInput(NULL, NULL);


    SDL_Surface* surface;
    
    surface = load_image("icons/check.png");
    check_texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    
    surface = load_image("icons/edit.png");
    edit_texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    
    surface = load_image("icons/trash.png");
    trash_texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);


    for (int i = 0; i < 7; i++) {
        if (day_symbols_textures[i]) {
            SDL_DestroyTexture(day_symbols_textures[i]);
            day_symbols_textures[i] = NULL;
        }

        SDL_Surface* surface = load_image(DAY_SYMBOLS[i].url.chars);
        if (!surface) {
            fprintf(stderr, "Failed to load image %s: %s\n", DAY_SYMBOLS[i].url.chars, IMG_GetError());
            continue;
        }

        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);

        day_symbols_textures[i] = texture;
        fprintf(stderr, "Texture %d initialized: %p\n", i, (void*)texture);
    }
}

#endif

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
                .textColor = COLOR_TEXT
            })
        );

        CLAY_TEXT(CLAY_STRING("Are you sure you want to delete:"),
            CLAY_TEXT_CONFIG({
                .fontSize = 16,
                .fontId = FONT_ID_BODY_16,
                .textColor = COLOR_TEXT
            })
        );

        CLAY(CLAY_LAYOUT({
            .padding = { 16, 16, 16, 16 },
            .childAlignment = { .x = CLAY_ALIGN_X_CENTER }
        }),
        CLAY_RECTANGLE({
            .color = COLOR_PANEL,
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
                    .textColor = COLOR_TEXT
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
                    .color = COLOR_PANEL,
                    .cornerRadius = CLAY_CORNER_RADIUS(4),
                    .cursorPointer = true
                }),
                Clay_OnHover(HandleModalCancel, 0)
            ) {
                CLAY_TEXT(CLAY_STRING("Cancel"),
                    CLAY_TEXT_CONFIG({
                        .fontSize = 16,
                        .fontId = FONT_ID_BODY_16,
                        .textColor = COLOR_TEXT
                    })
                );
            }

            CLAY(CLAY_ID("ConfirmDeleteButton"),
                CLAY_LAYOUT({
                    .padding = { 8, 8, 8, 8 },
                    .sizing = { CLAY_SIZING_FIT(0), CLAY_SIZING_FIT(0) }
                }),
                CLAY_RECTANGLE({
                    .color = COLOR_DANGER,
                    .cornerRadius = CLAY_CORNER_RADIUS(4),
                    .cursorPointer = true
                }),
                Clay_OnHover(HandleModalConfirm, 0)
            ) {
                CLAY_TEXT(CLAY_STRING("Delete"),
                    CLAY_TEXT_CONFIG({
                        .fontSize = 16,
                        .fontId = FONT_ID_BODY_16,
                        .textColor = COLOR_TEXT
                    })
                );
            }
        }
    }
}
void RenderDeleteTodoModal(void) {
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
            .color = COLOR_BACKGROUND,
            .cornerRadius = CLAY_CORNER_RADIUS(8)
        })
    ) {
        RenderDeleteTodoModalContent();
    }
}

void HandleTodosPageInput(InputEvent event) {
    if (!todo_input || !todo_collection.todo_edit_input) return;
    
    TextInput* active_input = todo_collection.editing_todo_id ? 
                            todo_collection.todo_edit_input : 
                            todo_input;

    // Update text input
    if (event.delta_time > 0) {
        UpdateTextInput(active_input, 0, event.delta_time);
    }

    // Handle text input
    if (event.isTextInput) {
        UpdateTextInput(active_input, event.text[0], event.delta_time);
    } else {
        UpdateTextInput(active_input, event.key, event.delta_time);
    }
}

static void HandleTabInteraction(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData) {
    if (pointerInfo.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        SetActiveDay(&todo_collection, DAYS[(int)userData]);
        SaveTodos(&todo_collection);
    }
}
void RenderTodoTab(const char* day, const DaySymbol* symbol, bool active, int index) {
    char id_buffer[32];
    
    // Increase padding on larger screens
    int horizontal_padding = windowWidth >= BREAKPOINT_MEDIUM ? 32 : 16;

    CLAY(Clay__AttachId(Clay__HashString((Clay_String){.chars = id_buffer, .length = strlen(id_buffer)}, index, 0)),
        CLAY_LAYOUT({ 
            .padding = { horizontal_padding, horizontal_padding, 8, 8 }, 
            .childGap = 8,
            .childAlignment = { CLAY_ALIGN_X_CENTER, CLAY_ALIGN_Y_CENTER }
        }),
        CLAY_BORDER({ .betweenChildren = { 1, COLOR_BORDER }}),
        CLAY_RECTANGLE({ 
            .color = active ? COLOR_SECONDARY : 
                     (Clay_Hovered() ? COLOR_PRIMARY_HOVER : COLOR_BACKGROUND),
            .cornerRadius = CLAY_CORNER_RADIUS(4),
            .cursorPointer = true

        }),
        Clay_OnHover(HandleTabInteraction, index)
    ) {
        CLAY(CLAY_LAYOUT({ 
            .sizing = { 
                CLAY_SIZING_FIXED(24), 
                CLAY_SIZING_FIXED(24)
            }
        }),
        #ifdef __EMSCRIPTEN__
        CLAY_IMAGE({ 
            .sourceDimensions = symbol->dimensions,
            .sourceURL = symbol->url
        })
        #else
        CLAY_IMAGE({ 
            .sourceDimensions = symbol->dimensions,
            .imageData = day_symbols_textures[index]
        })
        #endif
        ){}
    }
}

static void HandleSubmitButtonClick(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData) {
    if (pointerInfo.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        OnTodoInputSubmit(GetTextInputText(todo_input));
    }
}

static void HandleEditTodoButtonClick(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData) {
    if (pointerInfo.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        todo_collection.editing_todo_id = (uint32_t)userData;
        
        // Set the text input to current todo text
        for (size_t i = 0; i < todo_collection.todos_count; i++) {
            if (todo_collection.todos[i].id == todo_collection.editing_todo_id) {
                SetTextInputText(todo_collection.todo_edit_input, todo_collection.todos[i].text);
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
        const char* new_text = GetTextInputText(todo_collection.todo_edit_input);
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
        ClearTextInput(todo_collection.todo_edit_input);
        
        #ifdef CLAY_MOBILE
        SDL_StopTextInput();
        #endif
    }
}

static void HandleTodoDeleteClick(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData) {
    if (pointerInfo.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        uint32_t todo_id = (uint32_t)userData;
        // Set up delete modal state similar to habits
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

void RenderTodoItem(const Todo* todo, int index) {
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
            .color = todo->completed ? COLOR_SUCCESS : COLOR_CARD,
            .cornerRadius = CLAY_CORNER_RADIUS(4)
        })
    ) {
        // Main content area
        CLAY(CLAY_LAYOUT({
            .sizing = { CLAY_SIZING_GROW(), CLAY_SIZING_FIT(0) },
            .childAlignment = { CLAY_ALIGN_X_LEFT, CLAY_ALIGN_Y_CENTER }
        })) {
            if (is_editing) {
                CLAY(CLAY_LAYOUT({
                    .sizing = { CLAY_SIZING_GROW(), CLAY_SIZING_FIT(0) }
                })) {
                    RenderTextInput(todo_collection.todo_edit_input, todo->id);
                }
            } else {
                Clay_String todo_text = { .chars = todo->text, .length = strlen(todo->text) };
                CLAY_TEXT(todo_text, 
                    CLAY_TEXT_CONFIG({ 
                        .fontSize = 16, 
                        .textColor = COLOR_TEXT 
                    }));
            }
        }

        // Action buttons
        CLAY(CLAY_LAYOUT({
            .childGap = 8,
            .layoutDirection = CLAY_LEFT_TO_RIGHT
        })) {
            if (is_editing) {
                // Confirm button
                CLAY(CLAY_IDI("ConfirmEditButton", todo->id),
                    CLAY_LAYOUT({
                        .sizing = { CLAY_SIZING_FIXED(32), CLAY_SIZING_FIXED(32) },
                        .childAlignment = { .x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_CENTER }
                    }),
                    CLAY_RECTANGLE({
                        .color = Clay_Hovered() ? COLOR_SUCCESS : COLOR_SECONDARY,
                        .cornerRadius = CLAY_CORNER_RADIUS(4),
                        .cursorPointer = true
                    }),
                    Clay_OnHover(HandleTodoEditConfirm, todo->id)
                ) {
                    #ifdef __EMSCRIPTEN__
                    CLAY(CLAY_LAYOUT({
                        .sizing = { CLAY_SIZING_FIXED(24), CLAY_SIZING_FIXED(24) }
                    }),
                    CLAY_IMAGE({
                        .sourceDimensions = { 24, 24 },
                        .sourceURL = CLAY_STRING("icons/check.png")
                    })) {}
                    #else
                    CLAY(CLAY_LAYOUT({
                        .sizing = { CLAY_SIZING_FIXED(24), CLAY_SIZING_FIXED(24) }
                    }),
                    CLAY_IMAGE({
                        .sourceDimensions = { 24, 24 },
                        .imageData = check_texture
                    })) {}
                    #endif
                }

                // Delete button
                CLAY(CLAY_IDI("DeleteTodoButton", todo->id),
                    CLAY_LAYOUT({
                        .sizing = { CLAY_SIZING_FIXED(32), CLAY_SIZING_FIXED(32) },
                        .childAlignment = { .x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_CENTER }
                    }),
                    CLAY_RECTANGLE({
                        .color = Clay_Hovered() ? COLOR_DANGER : COLOR_PANEL,
                        .cornerRadius = CLAY_CORNER_RADIUS(4),
                        .cursorPointer = true
                    }),
                    Clay_OnHover(HandleTodoDeleteClick, todo->id)
                ) {
                    #ifdef __EMSCRIPTEN__
                    CLAY(CLAY_LAYOUT({
                        .sizing = { CLAY_SIZING_FIXED(24), CLAY_SIZING_FIXED(24) }
                    }),
                    CLAY_IMAGE({
                        .sourceDimensions = { 24, 24 },
                        .sourceURL = CLAY_STRING("icons/trash.png")
                    })) {}
                    #else
                    CLAY(CLAY_LAYOUT({
                        .sizing = { CLAY_SIZING_FIXED(24), CLAY_SIZING_FIXED(24) }
                    }),
                    CLAY_IMAGE({
                        .sourceDimensions = { 24, 24 },
                        .imageData = trash_texture
                    })) {}
                    #endif
                }
            } else {
                // Edit button
                CLAY(CLAY_IDI("EditTodoButton", todo->id),
                    CLAY_LAYOUT({
                        .sizing = { CLAY_SIZING_FIXED(32), CLAY_SIZING_FIXED(32) },
                        .childAlignment = { .x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_CENTER }
                    }),
                    CLAY_RECTANGLE({
                        .color = Clay_Hovered() ? COLOR_PRIMARY_HOVER : COLOR_PANEL,
                        .cornerRadius = CLAY_CORNER_RADIUS(4),
                        .cursorPointer = true
                    }),
                    Clay_OnHover(HandleEditTodoButtonClick, todo->id)
                ) {
                    #ifdef __EMSCRIPTEN__
                    CLAY(CLAY_LAYOUT({
                        .sizing = { CLAY_SIZING_FIXED(24), CLAY_SIZING_FIXED(24) }
                    }),
                    CLAY_IMAGE({
                        .sourceDimensions = { 24, 24 },
                        .sourceURL = CLAY_STRING("icons/edit.png")
                    })) {}
                    #else
                    CLAY(CLAY_LAYOUT({
                        .sizing = { CLAY_SIZING_FIXED(24), CLAY_SIZING_FIXED(24) }
                    }),
                    CLAY_IMAGE({
                        .sourceDimensions = { 24, 24 },
                        .imageData = edit_texture
                    })) {}
                    #endif
                }
            }
        }
    }
}


void RenderTodosPage() {
    CLAY(CLAY_ID("TodosContainer"), 
        CLAY_LAYOUT({ 
            .sizing = { CLAY_SIZING_GROW(), CLAY_SIZING_GROW() },
            .padding = { 32, 32, 32, 32 },
            .childGap = 24,
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
            .childAlignment = { .x = CLAY_ALIGN_X_CENTER } 
        })
    ) {
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

        // Todo input with max width
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
                    // Input field
                    CLAY(CLAY_LAYOUT({
                        .sizing = { CLAY_SIZING_GROW(), CLAY_SIZING_FIT(0) }
                    })) {
                        RenderTextInput(todo_input, 1);
                    }

                    // Submit button
                    CLAY(CLAY_ID("SubmitTodoButton"),
                        CLAY_LAYOUT({
                            .sizing = { CLAY_SIZING_FIXED(32), CLAY_SIZING_FIXED(32) },
                            .childAlignment = { .x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_CENTER }
                        }),
                        CLAY_RECTANGLE({
                            .color = Clay_Hovered() ? COLOR_SUCCESS : COLOR_SECONDARY,
                            .cornerRadius = CLAY_CORNER_RADIUS(4),
                            .cursorPointer = true
                        }),
                        Clay_OnHover(HandleSubmitButtonClick, 0)
                    ) {
                        #ifdef __EMSCRIPTEN__
                        CLAY(CLAY_LAYOUT({
                            .sizing = { CLAY_SIZING_FIXED(24), CLAY_SIZING_FIXED(24) }
                        }),
                        CLAY_IMAGE({
                            .sourceDimensions = { 24, 24 },
                            .sourceURL = CLAY_STRING("icons/check.png")
                        })) {}
                        #else
                        CLAY(CLAY_LAYOUT({
                            .sizing = { CLAY_SIZING_FIXED(24), CLAY_SIZING_FIXED(24) }
                        }),
                        CLAY_IMAGE({
                            .sourceDimensions = { 24, 24 },
                            .imageData = check_texture // Make sure to add this to the textures array
                        })) {}
                        #endif
                    }
                }
            }

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
        CLAY_SCROLL({ .vertical = true })) {
            size_t todos_count;
            Todo* todos = GetTodosByDay(&todo_collection, 
                                      todo_collection.active_day, 
                                      &todos_count);

            for (size_t i = 0; i < todos_count; i++) {
                RenderTodoItem(&todos[i], (int)i);
            }
        }
    }
    
    // Render delete modal on top if open
    RenderDeleteTodoModal();
}

void CleanupTodosPage() {
    #ifndef __EMSCRIPTEN__

    if (check_texture) {
        SDL_DestroyTexture(check_texture);
        check_texture = NULL;
    }
    if (edit_texture) {
        SDL_DestroyTexture(edit_texture);
        edit_texture = NULL;
    }
    if (trash_texture) {
        SDL_DestroyTexture(trash_texture);
        trash_texture = NULL;
    }

    fprintf(stderr, "Cleaning up Todos Page Textures\n");

    for (int i = 0; i < 7; i++) {
        if (day_symbols_textures[i]) {
            SDL_DestroyTexture(day_symbols_textures[i]);
            day_symbols_textures[i] = NULL;
        }
    }
    #endif

   
    if (todo_input) {
        DestroyTextInput(todo_input);
        todo_input = NULL;
    }
    if (todo_collection.todo_edit_input) {
        DestroyTextInput(todo_collection.todo_edit_input);
        todo_collection.todo_edit_input = NULL;
    }
}