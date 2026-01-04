#ifndef GFX_H
#define GFX_H

#include "shared.h"

#define COLOR_MAGENTA {1, 0, 1}
#define COLOR_BLACK   {0, 0, 0}
#define COLOR_GREEN   {0, 1, 0}
#define COLOR_WHITE   {1, 1, 1}
#define COLOR_RED     {1, 0, 0}
#define COLOR_BLUE    {0, 0, 1}
#define COLOR_YELLOW  {1, 1, 0}
#define COLOR_CYAN    {0, 1, 1}

typedef struct {
    UInt8 r, g, b;
} Color;

typedef struct {
    Int32 width, height;
    Color* pixels;
} GfxBuffer;

GfxBuffer* newGfxBuffer(Int32 width, Int32 height);

Void freeGfxBuffer(GfxBuffer* buffer);

#endif