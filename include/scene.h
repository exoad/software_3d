#ifndef SCENE_H
#define SCENE_H

#include "shared.h"
#include "gfx.h"
#include "renderer.h"
#include "geometry.h"
#include "tigr.h"

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
    Vec3 backgroundColor;
} Scene;

Scene* createScene(Camera camera, Vec3 backgroundColor);
Void destroyScene(Scene* scene);
Void sceneAddSphere(Scene* scene, GeomSphere sphere);
Void sceneAddBox(Scene* scene, GeomBox box);
Void sceneAddPlane(Scene* scene, GeomPlane plane);
Void sceneAddTriangle(Scene* scene, GeomTriangle triangle);
Void sceneAddLight(Scene* scene, Light light);
Void sceneRender(Scene* scene, Tigr* buffer);

#endif