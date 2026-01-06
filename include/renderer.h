#ifndef RENDERER_H
#define RENDERER_H

#include "shared.h"
#include "gfx.h"

#define AMBIENT_FACTOR 0.1f
#define DIFFUSE_FACTOR 0.8f
#define SPECULAR_FACTOR 0.5f
#define SHININESS 32.0f

typedef struct {
    Vec3 center;
    Float32 radius;
} Sphere;

typedef struct {
    Vec3 pos;
    Vec3 dir;
    Vec3 up;
    Float32 fov;
} Camera;

typedef enum {
    LIGHT_POINT,
    LIGHT_SPOT,
    LIGHT_AREA
} LightType;

typedef struct {
    LightType type;
    Vec3 pos;
    Vec3 color;
    Vec3 dir;
    Float32 innerCutoff;
    Float32 outerCutoff;
    Float32 radius; // for area lights
} Light;

Bool intersectSphere(Vec3 origin, Vec3 dir, Sphere sphere, Float32* t);
Vec3 phongShade(Vec3 hit, Vec3 normal, Vec3 viewDir, Vec3 lightPos, Vec3 lightColor, Vec3 objColor, Float32 shininess);
Void renderScene(GfxBuffer* buffer, Camera cam, Sphere sphere, Light light, Vec3 objColor, Float32 shininess);

#endif