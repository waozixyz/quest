#ifndef EVENTS_H
#define EVENTS_H

typedef struct {
    int key;              // Key code
    char text[32];        // Text input
    bool isTextInput;     // Whether this is a text input event
    float delta_time;     // Time since last frame
} InputEvent;

#endif
