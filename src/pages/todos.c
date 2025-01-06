#include "pages/todos.h"

static TodoCollection todo_collection = {0};
static TextInput* todo_input = NULL;
static char* DAYS[] = {"Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"};

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
    {.url = CLAY_STRING(ICON_PATH "moon.png"), .dimensions = {55, 55}},
    {.url = CLAY_STRING(ICON_PATH "mars.png"), .dimensions = {60, 60}},
    {.url = CLAY_STRING(ICON_PATH "mercury.png"), .dimensions = {55, 55}},
    {.url = CLAY_STRING(ICON_PATH "jupiter.png"), .dimensions = {55, 55}},
    {.url = CLAY_STRING(ICON_PATH "venus.png"), .dimensions = {55, 55}},
    {.url = CLAY_STRING(ICON_PATH "saturn.png"), .dimensions = {60, 60}},
    {.url = CLAY_STRING(ICON_PATH "sun.png"), .dimensions = {55, 55}}
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
}
#else
void InitializeTodosPage(SDL_Renderer* renderer) {
    LoadTodos(&todo_collection);
    todo_input = CreateTextInput(OnTodoInputChanged, OnTodoInputSubmit);

    for (int i = 0; i < 7; i++) {
        if (day_symbols_textures[i]) {
            SDL_DestroyTexture(day_symbols_textures[i]);
            day_symbols_textures[i] = NULL;
        }

        SDL_Surface* surface = IMG_Load(DAY_SYMBOLS[i].url.chars);
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

static void HandleTabInteraction(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData) {
    if (pointerInfo.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        SetActiveDay(&todo_collection, DAYS[(int)userData]);
        SaveTodos(&todo_collection);
    }
}

void RenderTodoTab(const char* day, const DaySymbol* symbol, bool active, int index) {
    char id_buffer[32];
    
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

        Clay_String day_str = { .chars = day, .length = strlen(day) };
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
                RenderTodoTab(DAYS[i], &DAY_SYMBOLS[i], 
                    strcmp(DAYS[i], todo_collection.active_day) == 0, i);
            }
        }

        // Todo input
        if (todo_input) {
            RenderTextInput(todo_input, 1);
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

void CleanupTodosPage() {
    #ifndef __EMSCRIPTEN__
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
}