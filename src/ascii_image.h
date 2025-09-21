#ifndef ASCII_IMAGE_H
#define ASCII_IMAGE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Different ramps for ASCII art
typedef enum {
    RAMP_LOW = 0,    // low-brightness gradient
    RAMP_SIMPLE,     // simple gradient
    RAMP_HIGH        // high-brightness gradient
} ASCII_RAMP;

// ASCII ramps
static const char *ASCII_CHARS_BIT    = " -*@";
static const char *ASCII_CHARS_SIMPLE = " .:-=+*%%#@";
static const char *ASCII_CHARS_HIGH   = " .'`^\",:;IIl!i<>~+_-?][}{1)(|\\/tfjrxnruvczYXUOJUCQL0Owmqpdbkhdah*#MW8%%B@$";

// Map brightness to ASCII character based on selected ramp, with optional inversion
static inline char brightness_to_ascii(unsigned char r, unsigned char g, unsigned char b, ASCII_RAMP ramp, int invert) {
    float brightness = 0.2126f * r + 0.7152f * g + 0.0722f * b; // perceptual brightness

    const char *chars;
    switch (ramp) {
        case RAMP_LOW:    chars = ASCII_CHARS_BIT; break;
        case RAMP_HIGH:   chars = ASCII_CHARS_HIGH; break;
        case RAMP_SIMPLE:
        default:          chars = ASCII_CHARS_SIMPLE; break;
    }

    int idx = (int)((brightness / 255.0f) * (strlen(chars) - 1));
    
    // Invert mapping if requested
    if (invert) idx = ((int)(strlen(chars)) - 1 - idx);

    return chars[idx];
}

// Generate ASCII art with given output width, ramp type, and inversion
static char* image_to_ascii(unsigned char *data, int width, int height, int channels, float scale, ASCII_RAMP ramp, int invert) {
    if (scale < 1) scale = 1;  // minimum scale

    int out_w = (int)(width / scale);
    int out_h = (int)(height / scale);

    if (out_w < 1) out_w = 1;
    if (out_h < 1) out_h = 1;

    int buf_size = (out_w * 2 + 1) * out_h + 1;
    char *output = (char*)malloc(buf_size);
    if (!output) return NULL;

    char *p = output;

    for (int y = 0; y < out_h; y++) {
        int src_y = (int)(y * scale);  // sample top-left of the block
        if (src_y >= height) src_y = height - 1;

        for (int x = 0; x < out_w; x++) {
            int src_x = (int)(x * scale);
            if (src_x >= width) src_x = width - 1;

            int idx = (src_y * width + src_x) * channels;

            unsigned char r = data[idx];
            unsigned char g = (channels >= 3) ? data[idx + 1] : r;
            unsigned char b = (channels >= 3) ? data[idx + 2] : r;

            *p++ = brightness_to_ascii(r, g, b, ramp, invert);
            *p++ = ' ';
        }
        *p++ = '\n';
    }
    *p = '\0';

    return output;
}

#endif
