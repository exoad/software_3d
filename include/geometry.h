#ifndef GEOMETRY_H
#define GEOMETRY_H

#include "shared.h"

// Forward declarations
typedef struct Tigr Tigr;

#define DEBUG_LIGHT_BOUNDS 0

typedef enum {
    TEXTURE_NONE,
    TEXTURE_CHECKERED,
    TEXTURE_NOISE,
    TEXTURE_IMAGE
} TextureType;

typedef struct {
    Vec3 color;
    Float32 shininess;
    Float32 diffuse;
    Float32 reflectivity;
    Float32 transparency;
    Float32 refractiveIndex;
    TextureType textureType;
    Tigr* texture;
} Material;

typedef struct {
    Vec3 center;
    Float32 radius;
    Float32 radiusSq;
    Float32 invRadius;
    Material material;
} GeomSphere;

typedef struct {
    Vec3 center;
    Vec3 extents; // halfwidths in each direction
    Vec3 min;
    Vec3 max;
    Material material;
} GeomBox;

typedef struct {
    Vec3 p0, p1, p2;
    Vec2 uv0, uv1, uv2;
    Vec3 edge1, edge2;
    Vec3 normal;
    Material material;
} GeomTriangle;

typedef struct {
    Vec3 normal;
    Float32 distance;
    Material material;
} GeomPlane;

typedef struct {
    GeomTriangle tri1;
    GeomTriangle tri2;
} GeomTriangularPrism;

Bool geomSphereIntersect(Vec3 rayOrigin, Vec3 rayDir, GeomSphere sphere, Float32* t, Vec3* normal, Vec2* uv);
Bool geomBoxIntersect(Vec3 rayOrigin, Vec3 rayDir, GeomBox box, Float32* t, Vec3* normal, Vec2* uv);
Bool geomTriangleIntersect(Vec3 rayOrigin, Vec3 rayDir, GeomTriangle tri, Float32* t, Vec3* normal, Vec2* uv);
Bool geomPlaneIntersect(Vec3 rayOrigin, Vec3 rayDir, GeomPlane plane, Float32* t, Vec3* normal, Vec2* uv);
Bool geomTriangularPrismIntersect(Vec3 rayOrigin, Vec3 rayDir, GeomTriangularPrism prism, Float32* t, Vec3* normal, Vec2* uv, Material* material);

#include "renderer.h"

Void geomDrawDebugBounds(Tigr* buffer, Vec3 lightPos, Float32 radius, Camera camera, Float32 renderScale);

#endif
