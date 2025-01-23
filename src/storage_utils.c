#include "storage_utils.h"
#include <utils.h>
#include <assert.h>

#ifndef __EMSCRIPTEN__

// Constants
#define DIRECTORY_PERMISSIONS 0755

// Helper function to construct file paths safely
static int construct_file_path(char* dest, size_t dest_size, const char* root_dir, const char* filename) {
    int result = snprintf(dest, dest_size, "%s/%s", root_dir, filename);
    if (result < 0 || result >= dest_size) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, 
            "Path too long: %s/%s", root_dir, filename);
        return -1;
    }
    return 0;
}

int ensure_directory_exists(const char* path) {
    assert(path != NULL);

    struct stat st = {0};
    char tmp[MAX_PATH_LENGTH];
    char *p = NULL;
    size_t len;

    // Copy the path to a temporary buffer
    snprintf(tmp, sizeof(tmp), "%s", path);
    len = strlen(tmp);

    // Remove trailing slash if present
    if (tmp[len - 1] == '/')
        tmp[len - 1] = 0;

    // Attempt to create the full directory path
    for (p = tmp + 1; *p; p++) {
        if (*p == '/') {
            *p = 0;  // Temporarily truncate the path
            
            // Try to create the parent directory
            if (stat(tmp, &st) == -1) {
                int mkdir_result = mkdir(tmp, DIRECTORY_PERMISSIONS);
                if (mkdir_result != 0 && errno != EEXIST) {
                    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, 
                        "Failed to create directory: %s. Error: %s", 
                        tmp, strerror(errno));
                    return -1;
                }
            }

            *p = '/';  // Restore the path separator
        }
    }

    // Create the final directory
    if (stat(tmp, &st) == -1) {
        int mkdir_result = mkdir(tmp, DIRECTORY_PERMISSIONS);
        if (mkdir_result != 0 && errno != EEXIST) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, 
                "Failed to create final directory: %s. Error: %s", 
                tmp, strerror(errno));
            return -1;
        }
    }

    return 0;
}

void determine_storage_directory(StorageConfig* config) {
    assert(config != NULL);
    memset(config, 0, sizeof(StorageConfig));

#if defined(CLAY_MOBILE)
    // Android-specific storage paths
    const char* potential_paths[] = {
        "/data/data/" APP_PACKAGE "/files",
        "/data/user/0/" APP_PACKAGE "/files",
        "/storage/emulated/0/Android/data/" APP_PACKAGE "/files"
    };

    for (int i = 0; i < 3; i++) {
        if (ensure_directory_exists(potential_paths[i]) == 0) {
            strlcpy(config->root_dir, potential_paths[i], MAX_PATH_LENGTH);
            break;
        }
    }

    if (strlen(config->root_dir) == 0) {
        // Fallback
        strcpy(config->root_dir, "/data/data/" APP_PACKAGE "/files");
        ensure_directory_exists(config->root_dir);
    }

#else
    // Desktop platforms with XDG compliance
    const char* home_dir = getenv("HOME");
    const char* xdg_data_home = getenv("XDG_DATA_HOME");

    char full_path[MAX_PATH_LENGTH];

    if (xdg_data_home) {
        // Use XDG_DATA_HOME if set
        snprintf(full_path, sizeof(full_path), "%s/myquest", xdg_data_home);
    } else if (home_dir) {
        // Fallback to ~/.local/share/myquest if XDG_DATA_HOME is not set
        snprintf(full_path, sizeof(full_path), "%s/.local/share/myquest", home_dir);
    } else {
        // Absolute last resort: current directory
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, 
            "Could not determine home or XDG_DATA_HOME directory. Using current directory.");
        strcpy(config->root_dir, ".");
        return;
    }

    // Ensure the directory exists
    if (ensure_directory_exists(full_path) == 0) {
        strlcpy(config->root_dir, full_path, MAX_PATH_LENGTH);
    } else {
        // Fallback to current directory if directory creation fails
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, 
            "Failed to create myquest directory. Falling back to current directory.");
        strcpy(config->root_dir, ".");
    }
#endif

    // Construct specific file paths with error handling
    if (construct_file_path(config->habits_path, MAX_PATH_LENGTH, config->root_dir, "habits.json") != 0 ||
        construct_file_path(config->todos_path, MAX_PATH_LENGTH, config->root_dir, "todos.json") != 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, 
            "Error: Storage path too long. Using default filenames.");
        strcpy(config->habits_path, "habits.json");
        strcpy(config->todos_path, "todos.json");
    }
}

int write_file_contents(const char* path, const char* contents, size_t length) {
    assert(path != NULL && contents != NULL);

    FILE* file = fopen(path, "wb");
    if (!file) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, 
            "Could not open file for writing: %s", path);
        return -1;
    }

    size_t written = fwrite(contents, 1, length, file);
    fclose(file);

    if (written != length) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, 
            "Failed to write entire file: %s", path);
        return -1;
    }

    return 0;
}

char* read_file_contents(const char* path, long* file_size) {
    assert(path != NULL && file_size != NULL);

    FILE* file = fopen(path, "rb");
    if (!file) {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, 
            "Could not open file for reading: %s", path);
        return NULL;
    }

    // Get file size
    fseek(file, 0, SEEK_END);
    *file_size = ftell(file);
    rewind(file);

    // Allocate memory
    char* buffer = malloc(*file_size + 1);
    if (!buffer) {
        fclose(file);
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, 
            "Failed to allocate memory for file reading");
        return NULL;
    }

    // Read file contents
    size_t read_size = fread(buffer, 1, *file_size, file);
    fclose(file);

    if (read_size != *file_size) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, 
            "Failed to read entire file: %s", path);
        free(buffer);
        return NULL;
    }

    buffer[*file_size] = '\0';  // Null-terminate
    return buffer;
}
#endif