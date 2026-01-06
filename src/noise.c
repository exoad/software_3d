#include "noise.h"
#include <math.h>

static Float32 hash(Int32 n)
{
    n = (n << 13) ^ n;
    const Float32 inv_2pow30 = 1.0f / 1073741824.0f;
    return (1.0f - ((n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff) * inv_2pow30);
}

static Float32 lerp(Float32 a, Float32 b, Float32 t)
{
    return a + t * (b - a);
}

static Float32 noise1D(Int32 x)
{
    return hash(x);
}

Float32 noiseValue3D(Float32 x, Float32 y, Float32 z)
{
    Int32 ix = (Int32)floorf(x);
    Int32 iy = (Int32)floorf(y);
    Int32 iz = (Int32)floorf(z);
    Float32 fx = x - ix;
    Float32 fy = y - iy;
    Float32 fz = z - iz;
    Int32 base = ix + iy * 57 + iz * 113;
    Float32 v000 = hash(base);
    Float32 v100 = hash(base + 1);
    Float32 v010 = hash(base + 57);
    Float32 v110 = hash(base + 58);
    Float32 v001 = hash(base + 113);
    Float32 v101 = hash(base + 114);
    Float32 v011 = hash(base + 170);
    Float32 v111 = hash(base + 171);
    Float32 i1 = lerp(v000, v100, fx);
    Float32 i2 = lerp(v010, v110, fx);
    Float32 i3 = lerp(v001, v101, fx);
    Float32 i4 = lerp(v011, v111, fx);
    return lerp(lerp(i1, i2, fy), lerp(i3, i4, fy), fz);
}

Float32 noiseFractal3D(Float32 x, Float32 y, Float32 z, Int32 octaves)
{
    Float32 total = 0.0f;
    Float32 frequency = 1.0f;
    Float32 amplitude = 1.0f;
    Float32 maxValue = 0.0f;
    for(Int32 i = 0; i < octaves; ++i)
    {
        total += noiseValue3D(x * frequency, y * frequency, z * frequency) * amplitude;
        maxValue += amplitude;
        amplitude *= 0.5f;
        frequency *= 2.0f;
    }
    return total / maxValue;
}
