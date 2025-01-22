#ifndef COLOR_PICKER_H
#define COLOR_PICKER_H
#include "../clay_extensions.h"
#include "clay.h"
#include <stdio.h>
#include "../config.h"
#include "modal.h"

// Predefined color palette with 255.0f scale
static const Clay_Color COLOR_PALETTE[] = {
    {139.0f, 0.0f, 0.0f, 255.0f},      // Maroon
    {70.0f, 130.0f, 180.0f, 255.0f},   // Steel Blue
    {188.0f, 143.0f, 143.0f, 255.0f},  // Rosy Brown
    {218.0f, 112.0f, 214.0f, 255.0f},  // Orchid
    {102.0f, 205.0f, 170.0f, 255.0f},  // Medium Aquamarine
    {205.0f, 92.0f, 92.0f, 255.0f},    // Indian Red
    {255.0f, 140.0f, 0.0f, 255.0f},    // Dark Orange
    {123.0f, 104.0f, 238.0f, 255.0f},  // Medium Slate Blue
    {46.0f, 139.0f, 87.0f, 255.0f},    // Sea Green
    {255.0f, 20.0f, 147.0f, 255.0f},   // Deep Pink
    {160.0f, 82.0f, 45.0f, 255.0f},    // Sienna
    {0.0f, 191.0f, 255.0f, 255.0f}     // Deep Sky Blue
};

#define COLOR_PALETTE_SIZE (sizeof(COLOR_PALETTE) / sizeof(Clay_Color))
#define COLORS_PER_ROW 4

// Function prototypes
void RenderColorPicker(Clay_Color current_color, void (*on_color_change)(Clay_Color), Modal* modal);

#endif // COLOR_PICKER_H