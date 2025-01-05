#ifndef TODOS_STATE_H
#define TODOS_STATE_H

#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include <stdlib.h>
#include "../../vendor/clay/clay.h"


#define MAX_TODO_TEXT 256
#define MAX_TODOS_PER_DAY 100
#define MAX_TODOS (MAX_TODOS_PER_DAY * 7)

typedef struct {
    uint32_t id;
    char text[MAX_TODO_TEXT];
    int32_t position;
    bool completed;
    time_t created_at;
    char day[10]; // Monday, Tuesday, etc.
} Todo;

typedef struct {
    Todo todos[MAX_TODOS];
    size_t todos_count;
    char active_day[10];
} TodoCollection;

// State management functions
void LoadTodos(TodoCollection* collection);
void SaveTodos(TodoCollection* collection);
void AddTodo(TodoCollection* collection, const char* text);
void DeleteTodo(TodoCollection* collection, uint32_t id);
void ToggleTodo(TodoCollection* collection, uint32_t id);
void UpdateTodoPositions(TodoCollection* collection, const char* day);
void SetActiveDay(TodoCollection* collection, const char* day);
Todo* GetTodosByDay(TodoCollection* collection, const char* day, size_t* count);

#endif
