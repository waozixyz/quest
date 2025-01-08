// events.h
#ifndef EVENTS_H
#define EVENTS_H

typedef struct {
    float delta_time;
    bool isTextInput;
    char text[32];
    int key;
} InputEvent;

#endif
