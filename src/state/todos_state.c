#include "state/todos_state.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>


EM_JS(void, JS_SaveTodos, (const TodoCollection* collection), {});
EM_JS(void, JS_LoadTodos, (TodoCollection* collection), {});
EM_JS(void, addTodoFunction, (TodoCollection* collection, const char* text));
EM_JS(void, deleteTodoFunction, (TodoCollection* collection, uint32_t id));
EM_JS(void, toggleTodoFunction, (TodoCollection* collection, uint32_t id));
EM_JS(void, setActiveDayFunction, (TodoCollection* collection, const char* day));

#else
#include "storage_utils.h"
#include "../../vendor/cJSON/cJSON.h"
#include <time.h>

static cJSON* TodoToJSON(const Todo* todo) {
    cJSON* todoObj = cJSON_CreateObject();
    cJSON_AddNumberToObject(todoObj, "id", todo->id);
    cJSON_AddStringToObject(todoObj, "text", todo->text);
    cJSON_AddNumberToObject(todoObj, "position", todo->position);
    cJSON_AddBoolToObject(todoObj, "completed", todo->completed);
    cJSON_AddNumberToObject(todoObj, "created_at", (double)todo->created_at);
    cJSON_AddStringToObject(todoObj, "day", todo->day);
    return todoObj;
}

static void SaveTodosJSON(const TodoCollection* collection) {
    if (!collection) return;

    StorageConfig storage_config;
    determine_storage_directory(&storage_config);

    cJSON* root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "active_day", collection->active_day);
    
    cJSON* todos = cJSON_CreateArray();
    for (size_t i = 0; i < collection->todos_count; i++) {
        cJSON* todo = TodoToJSON(&collection->todos[i]);
        cJSON_AddItemToArray(todos, todo);
    }
    cJSON_AddItemToObject(root, "todos", todos);

    char* jsonStr = cJSON_Print(root);
    write_file_contents(storage_config.todos_path, jsonStr, strlen(jsonStr));

    free(jsonStr);
    cJSON_Delete(root);
}

static void LoadTodoFromJSON(Todo* todo, cJSON* todoObj) {
    todo->id = cJSON_GetObjectItem(todoObj, "id")->valueint;
    strncpy(todo->text, cJSON_GetObjectItem(todoObj, "text")->valuestring, MAX_TODO_TEXT - 1);
    todo->position = cJSON_GetObjectItem(todoObj, "position")->valueint;
    todo->completed = cJSON_IsTrue(cJSON_GetObjectItem(todoObj, "completed"));
    todo->created_at = (time_t)cJSON_GetObjectItem(todoObj, "created_at")->valuedouble;
    strncpy(todo->day, cJSON_GetObjectItem(todoObj, "day")->valuestring, 9);
}

static void CreateDefaultTodosJSON(TodoCollection* collection) {
    memset(collection, 0, sizeof(TodoCollection));
    strcpy(collection->active_day, "Monday");
    SaveTodosJSON(collection);
}

static void LoadTodosJSON(TodoCollection* collection) {
    StorageConfig storage_config;
    determine_storage_directory(&storage_config);

    long file_size;
    char* jsonStr = read_file_contents(storage_config.todos_path, &file_size);
    if (!jsonStr) {
        CreateDefaultTodosJSON(collection);
        return;
    }

    cJSON* root = cJSON_Parse(jsonStr);
    free(jsonStr);

    if (!root) {
        CreateDefaultTodosJSON(collection);
        return;
    }

    memset(collection, 0, sizeof(TodoCollection));
    strncpy(collection->active_day, 
            cJSON_GetObjectItem(root, "active_day")->valuestring, 9);

    cJSON* todos = cJSON_GetObjectItem(root, "todos");
    collection->todos_count = 0;
    
    cJSON* todo;
    cJSON_ArrayForEach(todo, todos) {
        if (collection->todos_count >= MAX_TODOS) break;
        LoadTodoFromJSON(&collection->todos[collection->todos_count++], todo);
    }

    cJSON_Delete(root);
}
#endif

void LoadTodos(TodoCollection* collection) {
    if (!collection) return;

    #ifdef __EMSCRIPTEN__
        JS_LoadTodos(collection);
    #else
        LoadTodosJSON(collection);
    #endif

    if (collection->active_day[0] == '\0') {
        strcpy(collection->active_day, "Monday");
    }
}

void SaveTodos(TodoCollection* collection) {
    if (!collection) return;

    #ifdef __EMSCRIPTEN__
        JS_SaveTodos(collection);
    #else
        SaveTodosJSON(collection);
    #endif
}

void AddTodo(TodoCollection* collection, const char* text) {
    if (!collection || !text || collection->todos_count >= MAX_TODOS) return;
    #ifdef __EMSCRIPTEN__
        addTodoFunction(collection, text);
        JS_LoadTodos(collection);  // Reload the collection after adding
    #else
        Todo* new_todo = &collection->todos[collection->todos_count];
        new_todo->id = collection->todos_count;
        strncpy(new_todo->text, text, MAX_TODO_TEXT - 1);
        new_todo->position = collection->todos_count;
        new_todo->completed = false;
        new_todo->created_at = time(NULL);
        strcpy(new_todo->day, collection->active_day);

        collection->todos_count++;
        SaveTodos(collection);
    #endif
}

void DeleteTodo(TodoCollection* collection, uint32_t id) {
    if (!collection) return;
    #ifdef __EMSCRIPTEN__
        deleteTodoFunction(collection, id);
        JS_LoadTodos(collection);
    #else
        for (size_t i = 0; i < collection->todos_count; i++) {
            if (collection->todos[i].id == id) {
                // Shift remaining todos left
                memmove(&collection->todos[i], 
                        &collection->todos[i + 1],
                        (collection->todos_count - i - 1) * sizeof(Todo));
                collection->todos_count--;
                SaveTodos(collection);
                return;
            }
        }
    #endif
}

void ToggleTodo(TodoCollection* collection, uint32_t id) {
    if (!collection) return;

    #ifdef __EMSCRIPTEN__
        toggleTodoFunction(collection, id);
        JS_LoadTodos(collection); 
    #else
        for (size_t i = 0; i < collection->todos_count; i++) {
            if (collection->todos[i].id == id) {
                collection->todos[i].completed = !collection->todos[i].completed;
                SaveTodos(collection);
                return;
            }
        }
    #endif
}

void SetActiveDay(TodoCollection* collection, const char* day) {
    if (!collection || !day) return;
    
    #ifdef __EMSCRIPTEN__
        setActiveDayFunction(collection, day);
        JS_LoadTodos(collection);  // Reload the collection after changing the day
    #else
    strncpy(collection->active_day, day, 9);
    #endif
}

Todo* GetTodosByDay(TodoCollection* collection, const char* day, size_t* count) {
    static Todo filtered_todos[MAX_TODOS_PER_DAY];
    *count = 0;

    if (!collection || !day) return filtered_todos;

    for (size_t i = 0; i < collection->todos_count; i++) {
        if (strcmp(collection->todos[i].day, day) == 0) {
            filtered_todos[*count] = collection->todos[i];
            (*count)++;
        }
    }

    return filtered_todos;
}