#ifndef GEOMETRY_H
#define GEOMETRY_H

#include "shared.h"

// Forward declarations
typedef struct Tigr Tigr;

#define DEBUG_LIGHT_BOUNDS 0

typedef struct {
    Vec3 color;
    Float32 shininess;
    Float32 diffuse;
} Material;

typedef struct {
    Vec3 center;
    Float32 radius;
    Material material;
} GeomSphere;

typedef struct {
    Vec3 center;
    Vec3 extents; // Half-widths in each direction
    Material material;
} GeomBox;

typedef struct {
    Vec3 p0;
    Vec3 p1;
    Vec3 p2;
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

// Intersection functions
Bool geomSphereIntersect(Vec3 rayOrigin, Vec3 rayDir, GeomSphere sphere, Float32* t, Vec3* normal);
Bool geomBoxIntersect(Vec3 rayOrigin, Vec3 rayDir, GeomBox box, Float32* t, Vec3* normal);
Bool geomTriangleIntersect(Vec3 rayOrigin, Vec3 rayDir, GeomTriangle tri, Float32* t, Vec3* normal);
Bool geomPlaneIntersect(Vec3 rayOrigin, Vec3 rayDir, GeomPlane plane, Float32* t, Vec3* normal);
Bool geomTriangularPrismIntersect(Vec3 rayOrigin, Vec3 rayDir, GeomTriangularPrism prism, Float32* t, Vec3* normal, Material* material);

// Need renderer.h for Camera type in debug visualization
#include "renderer.h"

// Debug visualization
Void geomDrawDebugBounds(Tigr* buffer, Vec3 lightPos, Float32 radius, Camera camera, Float32 renderScale);

#endif
