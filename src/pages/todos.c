#include "todos.h"
// Global state for our text inputs
static TextInput* name_input;
static TextInput* description_input;

static void OnNameChanged(const char* text) {
    printf("Name changed: %s\n", text);
}

static void OnDescriptionChanged(const char* text) {
    printf("Description changed: %s\n", text);
}

// Call this when initializing your application
void InitializeTodosPage() {
    name_input = CreateTextInput(OnNameChanged);
    description_input = CreateTextInput(OnDescriptionChanged);
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
        // Title
        CLAY(CLAY_ID("TodosPageTitle"),
            CLAY_LAYOUT({
                .sizing = { CLAY_SIZING_GROW(), CLAY_SIZING_FIT(0) },
                .childAlignment = { .x = CLAY_ALIGN_X_CENTER },
                .padding = { 0, 32 }
            })
        ) {
            CLAY_TEXT(CLAY_STRING("Text Input Demo"), 
                CLAY_TEXT_CONFIG({ 
                    .fontSize = 48,
                    .fontId = FONT_ID_TITLE_56,
                    .textColor = COLOR_TEXT
                })
            );
        }

        // Test card for inputs
        CLAY(CLAY_ID("InputTestCard"),
            CLAY_LAYOUT({
                .sizing = { CLAY_SIZING_GROW({.max = 600}), CLAY_SIZING_FIT(0) },
                .childAlignment = { .x = CLAY_ALIGN_X_CENTER },
                .padding = { 24, 24 },
                .childGap = 16,
                .layoutDirection = CLAY_TOP_TO_BOTTOM
            }),
            CLAY_RECTANGLE({ 
                .color = COLOR_PANEL,
                .cornerRadius = CLAY_CORNER_RADIUS(8)
            })
        ) {
            // Name field label
            CLAY_TEXT(CLAY_STRING("Name:"),
                CLAY_TEXT_CONFIG({
                    .fontSize = 16,
                    .fontId = FONT_ID_BODY_16,
                    .textColor = COLOR_TEXT
                })
            );

            // Name input
            // RenderTextInput(name_input);

            // Description field label
            CLAY(CLAY_LAYOUT({ .padding = { 0, 16 } })) {
                CLAY_TEXT(CLAY_STRING("Description:"),
                    CLAY_TEXT_CONFIG({
                        .fontSize = 16,
                        .fontId = FONT_ID_BODY_16,
                        .textColor = COLOR_TEXT
                    })
                );
            }

            // Description input
            // RenderTextInput(description_input);

            // Help text
            CLAY(CLAY_LAYOUT({ .padding = { 0, 24 } })) {
                CLAY_TEXT(CLAY_STRING("Click an input to focus it, then type to enter text."),
                    CLAY_TEXT_CONFIG({
                        .fontSize = 14,
                        .fontId = FONT_ID_BODY_16,
                        .textColor = COLOR_TEXT_SECONDARY
                    })
                );
            }

            // Current values display
            CLAY(CLAY_LAYOUT({ 
                .padding = { 16, 24 },
                .childGap = 8,
                .layoutDirection = CLAY_TOP_TO_BOTTOM
            }),
            CLAY_RECTANGLE({
                .color = COLOR_SECONDARY,
                .cornerRadius = CLAY_CORNER_RADIUS(4)
            })) {
                CLAY_TEXT(CLAY_STRING("Current Values:"),
                    CLAY_TEXT_CONFIG({
                        .fontSize = 14,
                        .fontId = FONT_ID_BODY_16,
                        .textColor = COLOR_TEXT
                    })
                );

                // Display name value
                
                CLAY(CLAY_LAYOUT({ .childGap = 8 })) {
                    CLAY_TEXT(CLAY_STRING("Name: "),
                        CLAY_TEXT_CONFIG({
                            .fontSize = 14,
                            .fontId = FONT_ID_BODY_16,
                            .textColor = COLOR_TEXT_SECONDARY
                        })
                    );
                    CLAY_TEXT(CLAY_STRING(name_input->text),
                        CLAY_TEXT_CONFIG({
                            .fontSize = 14,
                            .fontId = FONT_ID_BODY_16,
                            .textColor = COLOR_TEXT
                        })
                    );
                }

                // Display description value
                
                CLAY(CLAY_LAYOUT({ .childGap = 8 })) {
                    CLAY_TEXT(CLAY_STRING("Description: "),
                        CLAY_TEXT_CONFIG({
                            .fontSize = 14,
                            .fontId = FONT_ID_BODY_16,
                            .textColor = COLOR_TEXT_SECONDARY
                        })
                    );
                    CLAY_TEXT(CLAY_STRING(description_input->text),
                        CLAY_TEXT_CONFIG({
                            .fontSize = 14,
                            .fontId = FONT_ID_BODY_16,
                            .textColor = COLOR_TEXT
                        })
                    );
                }
            }
        }
    }
}

// Add this cleanup function
void CleanupTodosPage() {
    if (name_input) {
        free(name_input);
        name_input = NULL;
    }
    if (description_input) {
        free(description_input);
        description_input = NULL;
    }
}