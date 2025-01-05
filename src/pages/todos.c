#include "todos.h"

// Global state for text inputs
static TextInput* name_input = NULL;
static TextInput* description_input = NULL;

static void OnNameChanged(const char* text) {
    printf("Name changed: %s\n", text);
}

static void OnDescriptionChanged(const char* text) {
    printf("Description changed: %s\n", text);
}

void InitializeTodosPage() {
    name_input = CreateTextInput(OnNameChanged);
    description_input = CreateTextInput(OnDescriptionChanged);
    
    // Safety check
    if (!name_input || !description_input) {
        printf("Failed to initialize text inputs!\n");
        return;
    }
}

void HandleTodosPageInput(InputEvent event) {
    if (!name_input || !description_input) return;

    // Update blink timer for both inputs
    if (event.delta_time > 0) {
        UpdateTextInput(name_input, 0, event.delta_time);
        UpdateTextInput(description_input, 0, event.delta_time);
    }

    // Handle actual input
    if (event.isTextInput) {
        if (name_input->is_focused) {
            UpdateTextInput(name_input, event.text[0], event.delta_time);
        }
        if (description_input->is_focused) {
            UpdateTextInput(description_input, event.text[0], event.delta_time);
        }
    } else {
        // Handle special keys
        if (name_input->is_focused) {
            UpdateTextInput(name_input, event.key, event.delta_time);
        }
        if (description_input->is_focused) {
            UpdateTextInput(description_input, event.key, event.delta_time);
        }
    }
}

void RenderTodosPage() {
    // Add safety check at the start
    if (!name_input || !description_input) {
        // Render error message or return
        return;
    }

    CLAY(CLAY_ID("TodosContainer"), 
        CLAY_LAYOUT({ 
            .sizing = { CLAY_SIZING_GROW(), CLAY_SIZING_GROW() },
            .padding = { 32, 32 },
            .childGap = 24,
            .layoutDirection = CLAY_TOP_TO_BOTTOM
        })
    ) {
        // Title
        CLAY_TEXT(CLAY_STRING("Text Input Demo"), 
            CLAY_TEXT_CONFIG({ 
                .fontSize = 48,
                .textColor = COLOR_TEXT
            })
        );

        // Input card
        CLAY(CLAY_ID("InputCard"),
            CLAY_LAYOUT({
                .sizing = { CLAY_SIZING_GROW({.max = 600}), CLAY_SIZING_FIT(0) },
                .padding = { 24, 24 },
                .childGap = 16,
                .layoutDirection = CLAY_TOP_TO_BOTTOM
            }),
            CLAY_RECTANGLE({ 
                .color = COLOR_CARD,
                .cornerRadius = CLAY_CORNER_RADIUS(8)
            })
        ) {
            // Name field
            CLAY_TEXT(CLAY_STRING("Name:"), 
                CLAY_TEXT_CONFIG({
                    .fontSize = 16,
                    .textColor = COLOR_TEXT
                })
            );
            
            if (name_input) {
                RenderTextInput(name_input);
            }

            // Description field
            CLAY_TEXT(CLAY_STRING("Description:"),
                CLAY_TEXT_CONFIG({
                    .fontSize = 16,
                    .textColor = COLOR_TEXT
                })
            );
            
            if (description_input) {
                RenderTextInput(description_input);
            }

            // Current values section
            if (name_input && description_input) {
                CLAY(CLAY_LAYOUT({ 
                    .padding = { 16, 16 },
                    .childGap = 8,
                    .layoutDirection = CLAY_TOP_TO_BOTTOM
                }),
                CLAY_RECTANGLE({
                    .color = COLOR_SECONDARY,
                    .cornerRadius = CLAY_CORNER_RADIUS(4)
                })) {
                    // Display name value
                    CLAY(CLAY_LAYOUT({ .childGap = 8 })) {
                        CLAY_TEXT(CLAY_STRING("Name: "), 
                            CLAY_TEXT_CONFIG({
                                .fontSize = 14,
                                .textColor = COLOR_TEXT_SECONDARY
                            })
                        );
                        
                        Clay_String name_str = {
                            .chars = name_input->text,
                            .length = name_input->text_length
                        };
                        CLAY_TEXT(name_str,
                            CLAY_TEXT_CONFIG({
                                .fontSize = 14,
                                .textColor = COLOR_TEXT
                            })
                        );
                    }

                    // Display description value
                    CLAY(CLAY_LAYOUT({ .childGap = 8 })) {
                        CLAY_TEXT(CLAY_STRING("Description: "),
                            CLAY_TEXT_CONFIG({
                                .fontSize = 14,
                                .textColor = COLOR_TEXT_SECONDARY
                            })
                        );
                        
                        Clay_String desc_str = {
                            .chars = description_input->text,
                            .length = description_input->text_length
                        };
                        CLAY_TEXT(desc_str,
                            CLAY_TEXT_CONFIG({
                                .fontSize = 14,
                                .textColor = COLOR_TEXT
                            })
                        );
                    }
                }
            }
        }
    }
}

void CleanupTodosPage() {
    if (name_input) {
        DestroyTextInput(name_input);
        name_input = NULL;
    }
    if (description_input) {
        DestroyTextInput(description_input);
        description_input = NULL;
    }
}