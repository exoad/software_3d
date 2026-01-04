#ifndef SCENE_H
#define SCENE_H

#include "shared.h"
#include "gfx.h"
#include "renderer.h"
#include "tigr.h"

typedef struct {
    Camera camera;
    Sphere* spheres;
    Int32 sphereCount;
    Int32 sphereCapacity;
    Light* lights;
    Int32 lightCount;
    Int32 lightCapacity;
    Vec3 backgroundColor;
} Scene;

Scene* createScene(Camera camera, Vec3 backgroundColor);
Void destroyScene(Scene* scene);
Void sceneAddSphere(Scene* scene, Sphere sphere);
Void sceneAddLight(Scene* scene, Light light);
Void sceneRender(Scene* scene, Tigr* buffer);

#endif