#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "ascii_image.h"

#define VERSION "1.0"

// --- Get the folder of a file path ---
char* get_input_dir_path(const char* input) {
    if (!input) return NULL;

    char drive[_MAX_DRIVE], dir[_MAX_DIR];
    _splitpath_s(input, drive, sizeof(drive), dir, sizeof(dir), NULL, 0, NULL, 0);

    char* result = (char*)malloc(_MAX_PATH);
    if (!result) return NULL;

    _makepath_s(result, _MAX_PATH, drive, dir, NULL, NULL);
    return result;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s [-p <path>] [-o <output>] [-s <width>] [--help] [--version]\n", argv[0]);
        return 1;
    }

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0) {
            printf("ASCII Image Generator %s\n", VERSION);
            printf("Usage: %s [-p <path>] [-o <output>] [-s <width>] [--help] [--version]\n", argv[0]);
            printf("Options:\n");
            printf("  -p <path>      Input image path (required)\n");
            printf("  -o <output>    Output text file (optional, relative to input image if relative path)\n");
            printf("  -s <width>     Output width in characters (optional, default 120)\n");
            printf("  --help         Show this help message\n");
            printf("  --version      Show program version\n");
            return 0;
        } else if (strcmp(argv[i], "--version") == 0) {
            printf("%s\n", VERSION);
            return 0;
        }
    }

    char *path = NULL;
    char *out = NULL;
    int out_width = 0; // 0 = auto (default 120)

    // --- Parse arguments ---
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-p") == 0 && i + 1 < argc) path = argv[++i];
        else if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) out = argv[++i];
        else if (strcmp(argv[i], "-s") == 0 && i + 1 < argc) {
            out_width = atoi(argv[++i]);
            if (out_width < 1) out_width = 1;
        }
    }

    if (!path) {
        printf("Missing -p <path>\nUse --help for usage.\n");
        return 1;
    }

    // --- Load image ---
    int width, height, channels;
    unsigned char* data = stbi_load(path, &width, &height, &channels, 0);
    if (!data) {
        printf("Failed to load image: %s\n", path);
        return 1;
    }

    printf("Loaded image: %s (%dx%d, %d channels)\n", path, width, height, channels);
    if (out_width == 0) out_width = 120;

    // --- Convert image to ASCII ---
    char* ascii = image_to_ascii(data, width, height, channels, out_width);
    if (!ascii) {
        printf("Failed to convert image to ASCII.\n");
        stbi_image_free(data);
        return 1;
    }

    // --- Handle output ---
    if (out) {
        char full_out[_MAX_PATH];

        if (PathIsRelativeA(out)) {
            char* input_dir = get_input_dir_path(path);
            if (!PathCombineA(full_out, input_dir, out)) {
                printf("Failed to combine paths.\n");
                free(input_dir);
                free(ascii);
                stbi_image_free(data);
                return 1;
            }
            free(input_dir);
        } else {
            strcpy_s(full_out, sizeof(full_out), out);
        }

        // Create parent directories if necessary
        char drive[_MAX_DRIVE], dir[_MAX_DIR], parent[_MAX_PATH];
        _splitpath_s(full_out, drive, sizeof(drive), dir, sizeof(dir), NULL, 0, NULL, 0);
        _makepath_s(parent, sizeof(parent), drive, dir, NULL, NULL);

        // Recursively create directories
        char current[_MAX_PATH] = {0};
        char temp[_MAX_PATH];
        strcpy_s(temp, sizeof(temp), parent);
        char* token;
        char* context = NULL;

        if (strlen(temp) > 1 && temp[1] == ':') {
            current[0] = temp[0]; current[1] = ':'; current[2] = '\0';
            token = &temp[2];
        } else {
            token = temp;
        }

        token = strtok_s(token, "\\/", &context);
        while (token) {
            if (strlen(current) > 0) strcat_s(current, sizeof(current), "\\");
            strcat_s(current, sizeof(current), token);

            DWORD attr = GetFileAttributesA(current);
            if (attr == INVALID_FILE_ATTRIBUTES) {
                if (!CreateDirectoryA(current, NULL)) {
                    DWORD error = GetLastError();
                    if (error != ERROR_ALREADY_EXISTS) {
                        printf("Failed to create directory: %s\n", current);
                        free(ascii);
                        stbi_image_free(data);
                        return 1;
                    }
                }
            }
            token = strtok_s(NULL, "\\/", &context);
        }

        // Write ASCII to file
        FILE* f;
        if (fopen_s(&f, full_out, "w") != 0 || !f) {
            printf("Failed to open output file: %s\n", full_out);
            free(ascii);
            stbi_image_free(data);
            return 1;
        }

        fprintf(f, "%s", ascii);
        fclose(f);
        printf("Saved ASCII art to: %s\n", full_out);
    } else {
        printf("\n%s\n", ascii);
    }

    free(ascii);
    stbi_image_free(data);
    return 0;
}
