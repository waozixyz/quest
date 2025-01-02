#include "habits.h"
#include "../styles.h"
#include <time.h>
#include <emscripten.h>
#include <stdio.h>

static const Clay_String DAY_STRINGS[32] = {
    {1, "0"},
    {1, "1"},
    {1, "2"},
    {1, "3"},
    {1, "4"},
    {1, "5"},
    {1, "6"},
    {1, "7"},
    {1, "8"},
    {1, "9"},
    {2, "10"},
    {2, "11"},
    {2, "12"},
    {2, "13"},
    {2, "14"},
    {2, "15"},
    {2, "16"},
    {2, "17"},
    {2, "18"},
    {2, "19"},
    {2, "20"},
    {2, "21"},
    {2, "22"},
    {2, "23"},
    {2, "24"},
    {2, "25"},
    {2, "26"},
    {2, "27"},
    {2, "28"},
    {2, "29"},
    {2, "30"},
    {2, "31"}
};

static const Clay_String MONTH_STRINGS[12] = {
    {7, "January"},
    {8, "February"},
    {5, "March"},
    {5, "April"},
    {3, "May"},
    {4, "June"},
    {4, "July"},
    {6, "August"},
    {9, "September"},
    {7, "October"},
    {8, "November"},
    {8, "December"}
};


void RenderDayBox(int day_number, bool is_today, bool is_past, int unique_index) {

    char day_id[20] = "DayBox_";
    int id_len = 7;  
    
    int temp_index = unique_index;
    int digit_count = 0;
    do {
        digit_count++;
        temp_index /= 10;
    } while (temp_index > 0);
    
    temp_index = unique_index;
    for (int i = digit_count - 1; i >= 0; i--) {
        day_id[id_len + i] = (temp_index % 10) + '0';
        temp_index /= 10;
    }
    day_id[id_len + digit_count] = '\0';

    // Create slightly dimmed color for past days
    Clay_Color box_color = COLOR_SECONDARY;
    if (is_past) {
        box_color.r *= 0.7f;
        box_color.g *= 0.7f;
        box_color.b *= 0.7f;
    }


    CLAY(CLAY_ID(day_id),
        CLAY_LAYOUT({
            .sizing = { 
                CLAY_SIZING_FIXED(70),
                CLAY_SIZING_FIXED(70)  
            },
            .childAlignment = { 
                .x = CLAY_ALIGN_X_CENTER,
                .y = CLAY_ALIGN_Y_CENTER 
            }
        }),
        CLAY_RECTANGLE({ 
            .color = box_color,
            .cornerRadius = CLAY_CORNER_RADIUS(8)
        }),
        is_today ? CLAY_BORDER_OUTSIDE_RADIUS(2, COLOR_PRIMARY, 8) : 0,  // Rounded border
        CLAY_TEXT(DAY_STRINGS[day_number], 
            CLAY_TEXT_CONFIG({ 
                .fontSize = 16,
                .fontId = FONT_ID_BODY_16,
                .textColor = COLOR_TEXT
            })
        )
    );
}
void RenderHabitsPage() {

    time_t now;
    time(&now);



    // Get current time
    struct tm *local_time = localtime(&now);
    
    // Create timestamp for today at midnight
    struct tm today_midnight = *local_time;
    today_midnight.tm_hour = 0;
    today_midnight.tm_min = 0;
    today_midnight.tm_sec = 0;
    time_t today_timestamp = mktime(&today_midnight);

    struct tm *today = localtime(&now);

    // Calculate start date (Monday of current week)
    struct tm start_date = today_midnight;  // Start with today at midnight
    int days_to_monday;
    if (start_date.tm_wday == 0) {  // If Sunday (0)
        days_to_monday = 6;
    } else {
        days_to_monday = start_date.tm_wday - 1;  // Otherwise subtract from current day (1-6) to get to Monday
    }
    start_date.tm_mday -= days_to_monday;  // Go back to Monday
    mktime(&start_date);  // Normalize the date

    CLAY(CLAY_ID("HabitsContainer"), 
        CLAY_LAYOUT({ 
            .sizing = { CLAY_SIZING_GROW(), CLAY_SIZING_GROW() },
            .padding = { 32, 32 },
            .childGap = 32,
            .layoutDirection = CLAY_TOP_TO_BOTTOM
        })
    ) {
        // Title
        CLAY(CLAY_ID("HabitsPageTitle"),
            CLAY_LAYOUT({
                .sizing = { CLAY_SIZING_GROW(), CLAY_SIZING_FIT(0) },
                .childAlignment = { .x = CLAY_ALIGN_X_CENTER },
                .padding = { 0, 32 }
            })
        ) {
            CLAY_TEXT(CLAY_STRING("Habits Calendar"), 
                CLAY_TEXT_CONFIG({ 
                    .fontSize = 48,
                    .fontId = FONT_ID_TITLE_56,
                    .textColor = COLOR_TEXT
                })
            );
        }

    // Date display
        CLAY(CLAY_ID("CurrentDateDisplay"),
            CLAY_LAYOUT({
                .sizing = { CLAY_SIZING_GROW(), CLAY_SIZING_FIT(0) },
                .childAlignment = { .x = CLAY_ALIGN_X_CENTER },
                .padding = { 0, 16 }
            })
        ) {
            CLAY_TEXT(MONTH_STRINGS[today->tm_mon],
                CLAY_TEXT_CONFIG({
                    .fontSize = 24,
                    .fontId = FONT_ID_BODY_24,
                    .textColor = COLOR_TEXT
                })
            );
            CLAY_TEXT(DAY_STRINGS[today->tm_mday],
                CLAY_TEXT_CONFIG({
                    .fontSize = 24,
                    .fontId = FONT_ID_BODY_24,
                    .textColor = COLOR_TEXT
                })
            );
        }

        // Scrollable Calendar Container
        CLAY(CLAY_ID("CalendarScrollContainer"),
            CLAY_LAYOUT({
                .sizing = { CLAY_SIZING_GROW(), CLAY_SIZING_GROW() }
            }),
            CLAY_SCROLL({ .vertical = true })
        ) {
            // Calendar Grid
            CLAY(CLAY_ID("CalendarGrid"), 
                CLAY_LAYOUT({
                    .sizing = { CLAY_SIZING_GROW(), CLAY_SIZING_GROW() },
                    .childGap = 32,
                    .layoutDirection = CLAY_TOP_TO_BOTTOM,
                    .padding = { 16, 16 }
                })
            ) {
                // Calculate end date (66 days + remainder of week)
                struct tm end_date = start_date;
                end_date.tm_mday += 66;
                mktime(&end_date);
                // Complete the week
                int days_to_add = (7 - end_date.tm_wday) % 7;
                end_date.tm_mday += days_to_add;
                mktime(&end_date);

                // Calculate total weeks needed
                time_t start_time = mktime(&start_date);
                time_t end_time = mktime(&end_date);
                int total_days = (int)((end_time - start_time) / (60 * 60 * 24)) + 1;
                int total_weeks = (total_days + 6) / 7;

                struct tm current = start_date;
                int unique_index = 0;

                for (int row = 0; row < total_weeks; row++) {
                    CLAY(CLAY_LAYOUT({
                        .sizing = { CLAY_SIZING_GROW(), CLAY_SIZING_FIT(0) },
                        .childGap = 10,
                        .layoutDirection = CLAY_LEFT_TO_RIGHT
                    })) {

                        for (int col = 0; col < 7; col++) {
                            time_t current_timestamp = mktime(&current);

                            bool is_today = (current_timestamp == today_timestamp);
                            bool is_past = (current_timestamp < today_timestamp);

                            RenderDayBox(current.tm_mday, is_today, is_past, unique_index++);
                            current.tm_mday++;
                        }
                    }
                }
            }
        }
    }
}
