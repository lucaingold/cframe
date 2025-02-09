#ifndef __FONTS_H
#define __FONTS_H

#include <stdint.h>

// Font structure definition
typedef struct {
    const uint8_t *table;
    uint16_t width;
    uint16_t height;
} sFONT2;

// Font declarations
extern sFONT2 Font12;

#endif /* __FONTS_H */
