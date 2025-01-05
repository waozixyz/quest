#ifndef TEXT_INPUT_H
#define TEXT_INPUT_H

#include "../../vendor/clay/clay.h"
#include "../styles.h"
#include <string.h>
#include <stdlib.h>

#define MAX_TEXT_INPUT_LENGTH 256

typedef struct {
    char text[MAX_TEXT_INPUT_LENGTH];
    size_t text_length;
    size_t cursor_position;
    bool is_focused;
    void (*on_change)(const char* text);
    void (*on_submit)(const char* text); 
    float blink_timer;
    bool cursor_visible;
} TextInput;

TextInput* CreateTextInput(void (*on_change)(const char* text), void (*on_submit)(const char* text));
void DestroyTextInput(TextInput* input);
void UpdateTextInput(TextInput* input, int key, float delta_time);
void RenderTextInput(TextInput* input, uint32_t id);
void SetTextInputText(TextInput* input, const char* text);
void ClearTextInput(TextInput* input);
const char* GetTextInputText(const TextInput* input);
void UnfocusTextInput(TextInput* input);
void HandleGlobalClick(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData);
#endif