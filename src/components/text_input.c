#include "text_input.h"
#include <string.h>
#include <stdlib.h>

static void HandleTextInputClick(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData) {
    TextInput* input = (TextInput*)userData;
    
    if (pointerInfo.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        input->is_focused = true;
    }
}

TextInput* CreateTextInput(void (*on_change)(const char* text)) {
    TextInput* input = (TextInput*)malloc(sizeof(TextInput));
    memset(input, 0, sizeof(TextInput));
    input->on_change = on_change;
    return input;
}

void UpdateTextInput(TextInput* input, int key) {
    if (!input->is_focused) return;

    // Handle backspace
    if (key == '\b') {
        if (input->text_length > 0 && input->cursor_position > 0) {
            memmove(&input->text[input->cursor_position - 1], 
                    &input->text[input->cursor_position], 
                    input->text_length - input->cursor_position);
            input->text_length--;
            input->cursor_position--;
            input->text[input->text_length] = '\0';
            if (input->on_change) {
                input->on_change(input->text);
            }
        }
        return;
    }

    // Handle printable characters
    if (key >= 32 && key <= 126 && input->text_length < MAX_TEXT_INPUT_LENGTH - 1) {
        memmove(&input->text[input->cursor_position + 1], 
                &input->text[input->cursor_position], 
                input->text_length - input->cursor_position);
        input->text[input->cursor_position] = (char)key;
        input->text_length++;
        input->cursor_position++;
        input->text[input->text_length] = '\0';
        if (input->on_change) {
            input->on_change(input->text);
        }
    }
}

void RenderTextInput(TextInput* input) {
    CLAY(CLAY_ID("TextInput"), 
        CLAY_LAYOUT({ 
            .sizing = { CLAY_SIZING_GROW(), CLAY_SIZING_FIXED(40) },
            .padding = { 8, 8 }
        }),
        CLAY_RECTANGLE({ 
            .color = input->is_focused ? COLOR_BACKGROUND_FOCUSED : COLOR_BACKGROUND,
            .cornerRadius = CLAY_CORNER_RADIUS(4)
        }),
        CLAY_BORDER({
            .left = { 1, input->is_focused ? COLOR_BORDER_FOCUSED : COLOR_BORDER },
            .right = { 1, input->is_focused ? COLOR_BORDER_FOCUSED : COLOR_BORDER },
            .top = { 1, input->is_focused ? COLOR_BORDER_FOCUSED : COLOR_BORDER },
            .bottom = { 1, input->is_focused ? COLOR_BORDER_FOCUSED : COLOR_BORDER },
            .cornerRadius = CLAY_CORNER_RADIUS(4)
        }),
        Clay_OnHover(HandleTextInputClick, (intptr_t)input)
    ) {
        // Render the text content
        if (input->text_length > 0) {
            CLAY_TEXT(CLAY_STRING(input->text), CLAY_TEXT_CONFIG({
                .textColor = COLOR_TEXT,
                .fontSize = 16,
                .wrapMode = CLAY_TEXT_WRAP_NONE
            }));
        }

        // Render cursor when focused
        if (input->is_focused) {
            CLAY(CLAY_ID("TextCursor"),
                CLAY_LAYOUT({
                    .sizing = { CLAY_SIZING_FIXED(2), CLAY_SIZING_FIXED(20) }
                }),
                CLAY_RECTANGLE({ .color = COLOR_TEXT })
            ) {}
        }
    }
}
