#include "components/text_input.h"
#include "rocks.h"
#include "quest_theme.h"

#define CURSOR_BLINK_RATE 0.53f
#define PADDING 8

static TextInput* focused_input = NULL;

static void HandleTextInputClick(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData) {
    TextInput* input = (TextInput*)userData;
    if (!input) return;
    
    if (pointerInfo.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        if (focused_input && focused_input != input) {
            UnfocusTextInput(focused_input);
        }
        
        input->is_focused = true;
        input->cursor_visible = true;
        input->blink_timer = 0;
        input->cursor_position = input->text_length;
        focused_input = input;

        #if defined(CLAY_MOBILE)
        SDL_StartTextInput();
        #endif
    }
}

void HandleGlobalClick(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData) {
    if (pointerInfo.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME && focused_input) {
        UnfocusTextInput(focused_input);
        focused_input = NULL;
    }
}

TextInput* CreateTextInput(void (*on_change)(const char* text), void (*on_submit)(const char* text)) {
    TextInput* input = (TextInput*)malloc(sizeof(TextInput));
    if (!input) return NULL;
    
    input->text[0] = '\0';
    input->text_length = 0;
    input->cursor_position = 0;
    input->is_focused = false;
    input->on_change = on_change;
    input->on_submit = on_submit;
    input->blink_timer = 0;
    input->cursor_visible = true;
    
    return input;
}

void DestroyTextInput(TextInput* input) {
    free(input);
}

void UpdateTextInput(TextInput* input, int key, float delta_time) {
    if (!input || !input->is_focused) return;

    input->blink_timer += delta_time;
    if (input->blink_timer >= CURSOR_BLINK_RATE) {
        input->blink_timer -= CURSOR_BLINK_RATE;
        input->cursor_visible = !input->cursor_visible;
    }

    if (key == '\r' || key == '\n') {
        if (input->on_submit) {
            input->on_submit(input->text);
        }
        return;
    }
    
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

    if (key == 0x25) { // Left arrow
        if (input->cursor_position > 0) {
            input->cursor_position--;
            input->cursor_visible = true;
            input->blink_timer = 0;
        }
        return;
    }
    if (key == 0x27) { // Right arrow
        if (input->cursor_position < input->text_length) {
            input->cursor_position++;
            input->cursor_visible = true;
            input->blink_timer = 0;
        }
        return;
    }

    if (key >= 32 && key <= 126 && input->text_length < MAX_TEXT_INPUT_LENGTH - 1) {
        memmove(&input->text[input->cursor_position + 1], 
                &input->text[input->cursor_position], 
                input->text_length - input->cursor_position);
        input->text[input->cursor_position] = (char)key;
        input->text_length++;
        input->cursor_position++;
        input->text[input->text_length] = '\0';
        input->cursor_visible = true;
        input->blink_timer = 0;
        if (input->on_change) {
            input->on_change(input->text);
        }
    }
}

void UnfocusTextInput(TextInput* input) {
    if (!input) return;
    input->is_focused = false;
    input->cursor_visible = false;
}

void RenderTextInput(TextInput* input, uint32_t id) { 
    if (!input) return;

    RocksTheme base_theme = rocks_get_theme(g_rocks);
    QuestThemeExtension* theme = (QuestThemeExtension*)base_theme.extension;

    CLAY(CLAY_IDI("TextInput", id), 
        CLAY_LAYOUT({ 
            .sizing = { CLAY_SIZING_GROW(), CLAY_SIZING_FIXED(40) },
            .padding = { PADDING, PADDING }
        }),
        CLAY_RECTANGLE({ 
            .color = input->is_focused ? base_theme.background_focused : base_theme.background,
            .cornerRadius = CLAY_CORNER_RADIUS(4)
        }),
        CLAY_BORDER({
            .left = { .width = 1, .color = input->is_focused ? theme->border_focused : theme->border },
            .right = { .width = 1, .color = input->is_focused ? theme->border_focused : theme->border },
            .top = { .width = 1, .color = input->is_focused ? theme->border_focused : theme->border },
            .bottom = { .width = 1, .color = input->is_focused ? theme->border_focused : theme->border },
            .cornerRadius = CLAY_CORNER_RADIUS(4)
        }),
        Clay_OnHover(HandleTextInputClick, (intptr_t)(void*)input)
    ) {
        if (input->text_length > 0) {
            if (input->cursor_position > 0) {
                Clay_String before_cursor = {
                    .chars = input->text,
                    .length = input->cursor_position
                };
                CLAY_TEXT(before_cursor, CLAY_TEXT_CONFIG({
                    .textColor = base_theme.text,
                    .fontSize = 16,
                    .wrapMode = CLAY_TEXT_WRAP_NONE
                }));
            }

            if (input->is_focused && input->cursor_visible) {
                CLAY(CLAY_ID("TextCursor"),
                    CLAY_LAYOUT({
                        .sizing = { CLAY_SIZING_FIXED(2), CLAY_SIZING_FIXED(20) }
                    }),
                    CLAY_RECTANGLE({ .color = theme->cursor })
                ) {}
            }

            if (input->cursor_position < input->text_length) {
                Clay_String after_cursor = {
                    .chars = &input->text[input->cursor_position],
                    .length = input->text_length - input->cursor_position
                };
                CLAY_TEXT(after_cursor, CLAY_TEXT_CONFIG({
                    .textColor = base_theme.text,
                    .fontSize = 16,
                    .wrapMode = CLAY_TEXT_WRAP_NONE
                }));
            }
        } else if (input->is_focused && input->cursor_visible) {
            CLAY(CLAY_ID("TextCursor"),
                CLAY_LAYOUT({
                    .sizing = { CLAY_SIZING_FIXED(2), CLAY_SIZING_FIXED(20) }
                }),
                CLAY_RECTANGLE({ .color = theme->cursor })
            ) {}
        }
    }
}

void SetTextInputText(TextInput* input, const char* text) {
    if (!input || !text) return;
    
    size_t len = strlen(text);
    if (len >= MAX_TEXT_INPUT_LENGTH) {
        len = MAX_TEXT_INPUT_LENGTH - 1;
    }
    
    memcpy(input->text, text, len);
    input->text[len] = '\0';
    input->text_length = len;
    input->cursor_position = len;
    
    if (input->on_change) {
        input->on_change(input->text);
    }
}

void ClearTextInput(TextInput* input) {
    if (!input) return;
    
    input->text[0] = '\0';
    input->text_length = 0;
    input->cursor_position = 0;
    
    if (input->on_change) {
        input->on_change(input->text);
    }
}

const char* GetTextInputText(const TextInput* input) {
    if (!input) return "";
    return input->text;
}