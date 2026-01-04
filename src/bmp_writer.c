#include "public.h"
#include <stdlib.h>

Bool bmpWrite(const CharSeq filename, const GfxBuffer* buffer) {
    if(!buffer || !buffer->pixels || buffer->width <= 0 || buffer->height <= 0) {
        return false;
    }

    CFile* file = fopen(filename, "wb");
    if(!file) {
        return false;
    }

    Int32 width = buffer->width;
    Int32 height = buffer->height;
    Int32 row_size = width * 3;
    Int32 padding = (4 - (row_size % 4)) % 4;
    Int32 padded_row_size = row_size + padding;
    UInt32 image_size = padded_row_size * height;
    UInt32 file_size = 54 + image_size;
    UInt8 fileHeader[14] = {
        'B',
        'M',
        (UInt8) (file_size & 0xFF),
        (UInt8) ((file_size >> 8) & 0xFF),
        (UInt8)((file_size >> 16) & 0xFF),
        (UInt8)((file_size >> 24) & 0xFF),
        0,
        0,
        0,
        0,
        54,
        0,
        0,
        0
    };
    UInt8 infoHeader[40] = {
        40,
        0,
        0,
        0,
        (UInt8)(width & 0xFF),
        (UInt8)((width >> 8) & 0xFF),
        (UInt8)((width >> 16) & 0xFF),
        (UInt8)((width >> 24) & 0xFF),
        (UInt8)(height & 0xFF),
        (UInt8)((height >> 8) & 0xFF),
        (UInt8)((height >> 16) & 0xFF),
        (UInt8)((height >> 24) & 0xFF),
        1,
        0,
        24,
        0,
        0,
        0,
        0,
        0,
        (UInt8)(image_size & 0xFF),
        (UInt8)((image_size >> 8) & 0xFF),
        (UInt8)((image_size >> 16) & 0xFF),
        (UInt8)((image_size >> 24) & 0xFF),
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0
    };
    if(fwrite(fileHeader, 1, 14, file) != 14)
    {
        fclose(file);
        return false;
    }
    if(fwrite(infoHeader, 1, 40, file) != 40)
    {
        fclose(file);
        return false;
    }
    for(Int32 y = height - 1; y >= 0; --y)
    {
        for(Int32 x = 0; x < width; ++x)
        {
            Color p = buffer->pixels[y * width + x];
            UInt8 pixel_data[3] = { p.b, p.g, p.r };
            if(fwrite(pixel_data, 1, 3, file) != 3)
            {
                fclose(file);
                return false;
            }
        }
        UInt8 pad[3] = {0};
        if(fwrite(pad, 1, padding, file) != (Size)padding)
        {
            fclose(file);
            return false;
        }
    }
    fclose(file);
    return true;
}