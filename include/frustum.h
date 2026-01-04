#ifndef FRUSTUM_H
#define FRUSTUM_H

#include "shared.h"
#include "renderer.h"

typedef struct {
    // 0: left, 1: right, 2: bottom, 3: top, 4: near, 5: far
    Plane planes[6];
} Frustum;

Void frustumExtract(Frustum* frustum, Camera* camera, Float32 nearPlane, Float32 farPlane, Float32 aspect);
Bool frustumSphereIntersect(const Frustum* frustum, Vec3 center, Float32 radius);

#endif