#ifndef SCENE_H
#define SCENE_H

#include "shared.h"
#include "gfx.h"
#include "renderer.h"
#include "geometry.h"
#include "tigr.h"

typedef struct {
    Vec3 min;
    Vec3 max;
} AABB;

typedef struct {
    AABB bounds;
    Int32 left;      // child index or triangle start
    Int32 triCount;  // 0 if internal node
} BVHNode;

typedef struct {
    Vec3 min;
    Vec3 max;
} ObjModel;

typedef struct {
    Vec3 pos;
    CharSeq text;
    TPixel color;
} DebugLabel;

typedef struct {
    Camera camera;
    GeomSphere* spheres;
    Int32 sphereCount;
    Int32 sphereCapacity;
    GeomBox* boxes;
    Int32 boxCount;
    Int32 boxCapacity;
    GeomPlane* planes;
    Int32 planeCount;
    Int32 planeCapacity;
    GeomTriangle* triangles;
    Int32 triangleCount;
    Int32 triangleCapacity;
    Light* lights;
    Int32 lightCount;
    Int32 lightCapacity;
    ObjModel* models;
    Int32 modelCount;
    Int32 modelCapacity;
    DebugLabel* labels;
    Int32 labelCount;
    Int32 labelCapacity;
    BVHNode* bvhNodes;
    Int32 bvhNodeCount;
    Int32 bvhNodeCapacity;
    Int32 bvhRoot;
    Tigr** textures;
    Int32 textureCount;
    Int32 textureCapacity;
    Vec3 backgroundColor;
} Scene;

Void sceneAddModel(Scene* scene, ObjModel model);
Void sceneAddDebugLabel(Scene* scene, Vec3 pos, CharSeq text, TPixel color);
Void sceneBuildBVH(Scene* scene);
Tigr* sceneLoadTexture(Scene* scene, CharSeq path);
Tigr* sceneCreateDebugTexture(Scene* scene, Int32 width, Int32 height);

Scene* createScene(Camera camera, Vec3 backgroundColor);
Void destroyScene(Scene* scene);
Void sceneAddSphere(Scene* scene, GeomSphere sphere);
Void sceneAddBox(Scene* scene, GeomBox box);
Void sceneAddPlane(Scene* scene, GeomPlane plane);
Void sceneAddTriangle(Scene* scene, GeomTriangle triangle);
Void sceneAddLight(Scene* scene, Light light);
Void sceneRender(Scene* scene, Tigr* buffer);

Void sceneAddSphereSimple(Scene* scene, Vec3 center, Float32 radius, Vec3 color);
Void sceneAddBoxSimple(Scene* scene, Vec3 center, Vec3 extents, Vec3 color);
Void sceneAddPlaneSimple(Scene* scene, Vec3 normal, Float32 distance, Vec3 color);
Void sceneAddLightSimple(Scene* scene, Vec3 pos, Vec3 color, Float32 intensity);
Void sceneAddSpotLight(Scene* scene, Vec3 pos, Vec3 dir, Float32 inner, Float32 outer, Vec3 color, Float32 intensity);
Void sceneAddAreaLight(Scene* scene, Vec3 pos, Float32 radius, Vec3 color, Float32 intensity);
Void sceneAddDefaultLights(Scene* scene);
Void sceneAddDefaultGround(Scene* scene);

#endif