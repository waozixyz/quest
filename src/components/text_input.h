#ifndef TEXT_INPUT_H
#define TEXT_INPUT_H

#include "../../vendor/clay/clay.h"
#include "../styles.h"

#define MAX_TEXT_INPUT_LENGTH 256

typedef struct {
    char text[MAX_TEXT_INPUT_LENGTH];
    size_t text_length;
    size_t cursor_position;
    bool is_focused;
    void (*on_change)(const char* text);
    float blink_timer;
    bool cursor_visible;
} TextInput;

// Create a new text input with optional onChange callback
TextInput* CreateTextInput(void (*on_change)(const char* text));

// Free memory allocated for text input
void DestroyTextInput(TextInput* input);

// Update text input state based on keyboard input and update cursor blink
void UpdateTextInput(TextInput* input, int key, float delta_time);

// Render the text input using Clay
void RenderTextInput(TextInput* input);

// Set the text programmatically
void SetTextInputText(TextInput* input, const char* text);

// Clear the text input
void ClearTextInput(TextInput* input);

// Get the current text
const char* GetTextInputText(const TextInput* input);

#endif