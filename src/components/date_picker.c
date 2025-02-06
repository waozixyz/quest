#include <stdio.h>
#include <string.h>
#include "components/date_picker.h"

#include "rocks.h"
#include "quest_theme.h"


static void (*g_date_change_callback)(time_t) = NULL;
static Modal* g_modal = NULL;
static struct tm g_editing_date = {0};
    
static const char* month_names[] = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun", 
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

void InitializeDatePicker(time_t initial_date, void (*on_date_change)(time_t), Modal* modal) {
    g_date_change_callback = on_date_change;
    g_modal = modal;
    
    struct tm* temp = localtime(&initial_date);
    if (temp) {
        g_editing_date = *temp;
        mktime(&g_editing_date);
    }
}

static void HandleDateClick(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData) {
    if (pointerInfo.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        Modal* modal = (Modal*)userData;
        OpenModal(modal);
    }
}

static void HandleDayChange(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData) {
    if (pointerInfo.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        int change = (int)userData;
        g_editing_date.tm_mday += change;
        mktime(&g_editing_date);
    }
}

static void HandleMonthChange(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData) {
    if (pointerInfo.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        int change = (int)userData;
        g_editing_date.tm_mon += change;
        mktime(&g_editing_date);
    }
}

static void HandleYearChange(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData) {
    if (pointerInfo.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        int change = (int)userData;
        g_editing_date.tm_year += change;
        mktime(&g_editing_date);
    }
}

static void HandleSaveDate(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData) {
    if (pointerInfo.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        if (g_date_change_callback) {
            time_t new_date = mktime(&g_editing_date);
            g_date_change_callback(new_date);
        }
        g_modal->is_open = false;
    }
}

static void RenderDatePickerModal(void) {
    Rocks_Theme base_theme = Rocks_GetTheme(GRocks);
    QuestThemeExtension* theme = (QuestThemeExtension*)base_theme.extension;

    static char year_str[16];
    static char month_str[32];
    static char day_str[16];
    
    // Year string conversion
    {
        int year = 1900 + g_editing_date.tm_year;
        int index = 0;
        int temp_year = year;
        int digits[4] = {0};
        int digit_count = 0;
        
        while (temp_year > 0) {
            digits[digit_count++] = temp_year % 10;
            temp_year /= 10;
        }
        
        for (int i = digit_count - 1; i >= 0; i--) {
            year_str[index++] = digits[i] + '0';
        }
        year_str[index] = '\0';
    }
    
    // Month string
    {
        int month_index = g_editing_date.tm_mon;
        if (month_index < 0) month_index = 0;
        if (month_index > 11) month_index = 11;
        
        const char* month_name = month_names[month_index];
        size_t i = 0;
        while (month_name[i] && i < sizeof(month_str) - 1) {
            month_str[i] = month_name[i];
            i++;
        }
        month_str[i] = '\0';
    }
    
    // Day string conversion
    {
        int day = g_editing_date.tm_mday;
        int index = 0;
        int temp_day = day;
        int digits[2] = {0};
        int digit_count = 0;
        
        while (temp_day > 0) {
            digits[digit_count++] = temp_day % 10;
            temp_day /= 10;
        }
        
        for (int i = digit_count - 1; i >= 0; i--) {
            day_str[index++] = digits[i] + '0';
        }
        day_str[index] = '\0';
    }

    CLAY(CLAY_ID("DatePickerModal"),
        CLAY_LAYOUT({
            .sizing = { CLAY_SIZING_GROW(), CLAY_SIZING_GROW() },
            .childGap = 20,
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
            .padding = { 20, 20, 20, 20 }
        })
    ) {
        // Title
        CLAY_TEXT(CLAY_STRING("Select Date"), 
            CLAY_TEXT_CONFIG({
                .fontSize = 24,
                .textColor = base_theme.text
            })
        );

        // Year
        CLAY(CLAY_LAYOUT({
            .childGap = 10,
            .layoutDirection = CLAY_LEFT_TO_RIGHT
        })) {
            CLAY_TEXT(CLAY_STRING("Year:"), CLAY_TEXT_CONFIG({ .fontSize = 18, .textColor = base_theme.text }));
            
            CLAY(CLAY_LAYOUT({ .padding = { 10, 10, 5, 5 }}),
                CLAY_RECTANGLE({ .color = base_theme.secondary }),
                Clay_OnHover(HandleYearChange, -1)
            ) {
                CLAY_TEXT(CLAY_STRING("-"), CLAY_TEXT_CONFIG({ .fontSize = 18, .textColor = base_theme.text }));
            }

            Clay_String year_obj = { .chars = year_str, .length = strlen(year_str) };
            CLAY_TEXT(year_obj, CLAY_TEXT_CONFIG({ .fontSize = 18, .textColor = base_theme.text }));

            CLAY(CLAY_LAYOUT({ .padding = { 10, 10, 5, 5 }}),
                CLAY_RECTANGLE({ .color = base_theme.secondary }),
                Clay_OnHover(HandleYearChange, 1)
            ) {
                CLAY_TEXT(CLAY_STRING("+"), CLAY_TEXT_CONFIG({ .fontSize = 18, .textColor = base_theme.text }));
            }
        }

        // Month
        CLAY(CLAY_LAYOUT({
            .childGap = 10,
            .layoutDirection = CLAY_LEFT_TO_RIGHT
        })) {
            CLAY_TEXT(CLAY_STRING("Month:"), CLAY_TEXT_CONFIG({ .fontSize = 18, .textColor = base_theme.text }));
            
            CLAY(CLAY_LAYOUT({ .padding = { 10, 10, 5, 5 }}),
                CLAY_RECTANGLE({ .color = base_theme.secondary }),
                Clay_OnHover(HandleMonthChange, -1)
            ) {
                CLAY_TEXT(CLAY_STRING("-"), CLAY_TEXT_CONFIG({ .fontSize = 18, .textColor = base_theme.text }));
            }

            Clay_String month_obj = { .chars = month_str, .length = strlen(month_str) };
            CLAY_TEXT(month_obj, CLAY_TEXT_CONFIG({ .fontSize = 18, .textColor = base_theme.text }));

            CLAY(CLAY_LAYOUT({ .padding = { 10, 10, 5, 5 }}),
                CLAY_RECTANGLE({ .color = base_theme.secondary }),
                Clay_OnHover(HandleMonthChange, 1)
            ) {
                CLAY_TEXT(CLAY_STRING("+"), CLAY_TEXT_CONFIG({ .fontSize = 18, .textColor = base_theme.text }));
            }
        }

        // Day
        CLAY(CLAY_LAYOUT({
            .childGap = 10,
            .layoutDirection = CLAY_LEFT_TO_RIGHT
        })) {
            CLAY_TEXT(CLAY_STRING("Day:"), CLAY_TEXT_CONFIG({ .fontSize = 18, .textColor = base_theme.text }));
            
            CLAY(CLAY_LAYOUT({ .padding = { 10, 10, 5, 5 }}),
                CLAY_RECTANGLE({ .color = base_theme.secondary }),
                Clay_OnHover(HandleDayChange, -1)
            ) {
                CLAY_TEXT(CLAY_STRING("-"), CLAY_TEXT_CONFIG({ .fontSize = 18, .textColor = base_theme.text }));
            }

            Clay_String day_obj = { .chars = day_str, .length = strlen(day_str) };
            CLAY_TEXT(day_obj, CLAY_TEXT_CONFIG({ .fontSize = 18, .textColor = base_theme.text }));

            CLAY(CLAY_LAYOUT({ .padding = { 10, 10, 5, 5 }}),
                CLAY_RECTANGLE({ .color = base_theme.secondary }),
                Clay_OnHover(HandleDayChange, 1)
            ) {
                CLAY_TEXT(CLAY_STRING("+"), CLAY_TEXT_CONFIG({ .fontSize = 18, .textColor = base_theme.text }));
            }
        }

        // Save Button
        CLAY(CLAY_LAYOUT({ .padding = { 0, 0, 10, 10 }})) {
            CLAY(CLAY_LAYOUT({ .padding = { 20, 20, 10, 10 }}),
                CLAY_RECTANGLE({
                    .color = base_theme.primary,
                    .cursorPointer = true,
                    .cornerRadius = CLAY_CORNER_RADIUS(8)
                }),
                Clay_OnHover(HandleSaveDate, 0)
            ) {
                CLAY_TEXT(CLAY_STRING("Save"), CLAY_TEXT_CONFIG({ .fontSize = 18, .textColor = base_theme.text }));
            }
        }
    }
}

void RenderDatePicker(time_t current_date, void (*on_date_change)(time_t), Modal* modal) {
    Rocks_Theme base_theme = Rocks_GetTheme(GRocks);
    QuestThemeExtension* theme = (QuestThemeExtension*)base_theme.extension;

    static char date_str[32] = {0};
    struct tm* current_tm = localtime(&current_date);

    memset(date_str, 0, sizeof(date_str));

    if (current_tm) {
        snprintf(date_str, sizeof(date_str), "%d %s %d", 
            current_tm->tm_mday, 
            (current_tm->tm_mon < 12) ? month_names[current_tm->tm_mon] : "Invalid",
            1900 + current_tm->tm_year);
    } else {
        strcpy(date_str, "Invalid Date");
    }

    CLAY(CLAY_ID("DatePickerContainer"),
        CLAY_LAYOUT({
            .sizing = { CLAY_SIZING_FIT(), CLAY_SIZING_FIT() },
            .padding = { 16, 16, 16, 16 }
        })
    ) {
        CLAY(CLAY_ID("DateDisplay"),
            CLAY_LAYOUT({ .padding = { 10, 10, 5, 5 }}),
            CLAY_RECTANGLE({
                .color = base_theme.secondary,
                .cursorPointer = true,
                .cornerRadius = CLAY_CORNER_RADIUS(8)
            }),
            Clay_OnHover(HandleDateClick, (intptr_t)modal)
        ) {
            Clay_String date_obj = { .chars = date_str, .length = strlen(date_str) };
            CLAY_TEXT(date_obj, CLAY_TEXT_CONFIG({ .fontSize = 16, .textColor = base_theme.text }));
        }
    }

    if (modal->is_open) {
        RenderModal(modal, RenderDatePickerModal);
    }
}