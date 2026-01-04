#include "public.h"

Int32 main(Int32 argc, CharSeq argv[])
{
    use(argc);
    use(argv);
    GfxBuffer* buffer = newGfxBuffer(100, 100);
    for(Int32 y = 0; y < buffer->height; ++y)
    {
        for(Int32 x = 0; x < buffer->width; ++x)
        {
            GfxPixel* p = &buffer->pixels[y * buffer->width + x];
            Float32 v = sinf(x * 0.1f) * sinf(y * 0.1f) + sinf((x + y) * 0.1f);
            p->r = (UInt8) (128 + 127 * sinf(v * 1.0f));
            p->g = (UInt8) (128 + 127 * sinf(v * 3.0f));
            p->b = (UInt8) (128 + 127 * sinf(v * 5.0f));
        }
    }
    bmpWrite("output.bmp", buffer);
    freeGfxBuffer(buffer);
    return 0;
}