#include <stdlib.h>
#include "gfx.h"

GfxBuffer* newGfxBuffer(Int32 width, Int32 height) {
    if(width <= 0 || height <= 0)
    {
        return null;
    }
    GfxBuffer* buffer = (GfxBuffer*) malloc(sizeof(GfxBuffer));
    if(!buffer)
    {
        return null;
    }
    buffer->width = width;
    buffer->height = height;
    buffer->pixels = (Color*) malloc(sizeof(Color) * width * height);
    if(!buffer->pixels)
    {
        free(buffer);
        return null;
    }
    return buffer;
}


Void freeGfxBuffer(GfxBuffer* buffer)
{
    if(buffer)
    {
        free(buffer->pixels);
        free(buffer);
    }
}