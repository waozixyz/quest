#ifndef PAGES_H
#define PAGES_H

#include "home.h"      // Include the Home page header
#include "habits.h"    // Include the Habits page header
#include "todos.h"     // Include the Todos page header
#include "timeline.h"  // Include the Timeline page header
#include "routine.h"   // Include the Routine page header

// Define the PageID enum
typedef enum {
    PAGE_HOME,     // Home page
    PAGE_HABITS,   // Habits page
    PAGE_TODOS,    // Todos page
    PAGE_TIMELINE, // Timeline page
    PAGE_ROUTINE,  // Routine page
    NUM_PAGES      // Total number of pages
} PageID;

// Declare the active page variable
extern PageID ACTIVE_PAGE;

#endif // PAGES_H
