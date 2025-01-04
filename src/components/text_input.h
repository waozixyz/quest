#ifndef TEXT_INPUT_H
#define TEXT_INPUT_H

#include "../../vendor/clay/clay.h"
#include "../styles.h"

// Max length for input text
#define MAX_TEXT_INPUT_LENGTH 256

typedef struct {
    char text[MAX_TEXT_INPUT_LENGTH];
    size_t cursor_position;
    size_t text_length;
    bool is_focused;
    Clay_ElementId element_id;
    void (*on_change)(const char* text); // Callback for when text changes
} TextInput;

// Initialize a new text input
TextInput* CreateTextInput(void (*on_change)(const char* text));

// Update text input state
void UpdateTextInput(TextInput* input, int key);

// Render the text input component
void RenderTextInput(TextInput* input);

#endif
