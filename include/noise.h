#ifndef NOISE_H
#define NOISE_H

#include "shared.h"

Float32 noiseValue2D(Float32 x, Float32 y);
Float32 noiseValue3D(Float32 x, Float32 y, Float32 z);
Float32 noiseFractal3D(Float32 x, Float32 y, Float32 z, Int32 octaves);

#endif
