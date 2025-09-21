#ifndef ASCII_IMAGE_H
#define ASCII_IMAGE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ASCII ramp from dark → light
static const char *ASCII_CHARS =
    " .:-=+*%#@";

// brightness → ASCII character
static inline char brightness_to_ascii(unsigned char r, unsigned char g, unsigned char b) {
    float brightness = 0.2126f * r + 0.7152f * g + 0.0722f * b; // perceptual
    int idx = (int)((brightness / 255.0f) * (strlen(ASCII_CHARS) - 1));
    return ASCII_CHARS[idx];
}

// Generate ASCII art with given output width
static char* image_to_ascii(unsigned char *data, int width, int height, int channels, int out_w) {
    // maintain aspect ratio (console chars ~2:1 height/width)
    float aspect = 2.0f;
    int out_h = (int)((float)height * ((float)out_w / width) / aspect);

    // buffer = (chars per row + newline) * rows + 1 (null terminator)
    int buf_size = (out_w + 1) * out_h + 1;
    char *output = (char*)malloc(buf_size);
    if (!output) return NULL;

    char *p = output;

    for (int y = 0; y < out_h; y++) {
        for (int x = 0; x < out_w; x++) {
            // Map output pixel to source pixel
            int src_x = (x * width) / out_w;
            int src_y = (y * height) / out_h;
            int idx = (src_y * width + src_x) * channels;

            unsigned char r = data[idx];
            unsigned char g = (channels >= 3) ? data[idx+1] : r;
            unsigned char b = (channels >= 3) ? data[idx+2] : r;

            *p++ = brightness_to_ascii(r, g, b);
        }
        *p++ = '\n';
    }
    *p = '\0';

    return output;
}

#endif
