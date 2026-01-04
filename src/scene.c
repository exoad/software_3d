#include "scene.h"
#include "frustum.h"
#include <stdlib.h>

Scene* createScene(Camera camera, Vec3 backgroundColor)
{
    Scene* scene = (Scene*) malloc(sizeof(Scene));
    if(!scene)
    {
        return null;
    }
    scene->camera = camera;
    scene->spheres = null;
    scene->sphereCount = 0;
    scene->sphereCapacity = 0;
    scene->lights = null;
    scene->lightCount = 0;
    scene->lightCapacity = 0;
    scene->backgroundColor = backgroundColor;
    return scene;
}

Void destroyScene(Scene* scene)
{
    if(scene)
    {
        free(scene->spheres);
        free(scene->lights);
        free(scene);
    }
}

Void sceneAddSphere(Scene* scene, Sphere sphere)
{
    if(scene->sphereCount >= scene->sphereCapacity)
    {
        scene->sphereCapacity = scene->sphereCapacity == 0 ? 4 : scene->sphereCapacity * 2;
        scene->spheres = (Sphere*) realloc(scene->spheres, sizeof(Sphere) * scene->sphereCapacity);
    }
    scene->spheres[scene->sphereCount++] = sphere;
}

Void sceneAddLight(Scene* scene, Light light)
{
    if(scene->lightCount >= scene->lightCapacity)
    {
        scene->lightCapacity = scene->lightCapacity == 0 ? 4 : scene->lightCapacity * 2;
        scene->lights = (Light*) realloc(scene->lights, sizeof(Light) * scene->lightCapacity);
    }
    scene->lights[scene->lightCount++] = light;
}

Void sceneRender(Scene* scene, Tigr* buffer)
{
    Float32 aspect = (Float32) buffer->w / buffer->h;
    Frustum frustum;
    frustumExtract(&frustum, &scene->camera, 0.1f, 100.0f, aspect);
    for(Int32 y = 0; y < buffer->h; ++y)
    {
        for(Int32 x = 0; x < buffer->w; ++x)
        {
            Float32 u = (2.0f * (x + 0.5f) / buffer->w - 1.0f) * tanf(scene->camera.fov / 2.0f) * aspect;
            Float32 v = (1.0f - 2.0f * (y + 0.5f) / buffer->h) * tanf(scene->camera.fov / 2.0f);
            Vec3 right = vec3Norm(
                vec3Cross(scene->camera.dir, scene->camera.up)
            );
            Vec3 rayDir = vec3Norm(
                vec3Add(
                    vec3Add(
                        vec3Mul(scene->camera.dir, 1),
                        vec3Mul(right, u)
                    ),
                    vec3Mul(scene->camera.up, v)
                )
            );
            Float32 closestT = INFINITY;
            Sphere* hitSphere = null;
            for(Int32 i = 0; i < scene->sphereCount; ++i)
            {
                // if(!frustumSphereIntersect(&frustum, scene->spheres[i].center, scene->spheres[i].radius))
                // { // cull it!
                //     continue;
                // }
                Float32 t;
                if(intersectSphere(scene->camera.pos, rayDir, scene->spheres[i], &t) && t < closestT)
                {
                    closestT = t;
                    hitSphere = &scene->spheres[i];
                }
            }
            if(hitSphere)
            {
                Vec3 hit = vec3Add(
                    scene->camera.pos,
                    vec3Mul(rayDir, closestT)
                );
                Vec3 normal = vec3Norm(vec3Sub(hit, hitSphere->center));
                Vec3 viewDir = vec3Norm(vec3Sub(scene->camera.pos, hit));
                Vec3 color = {0, 0, 0};
                for(Int32 i = 0; i < scene->lightCount; ++i)
                {
                    Vec3 lightDir = vec3Norm(vec3Sub(scene->lights[i].pos, hit));
                    Float32 diff = fmaxf(vec3Dot(normal, lightDir), 0.0f);
                    Vec3 diffuse = vec3Mul(scene->lights[i].color, diff);
                    Vec3 reflectDir = vec3Reflect(
                        vec3Mul(lightDir, -1),
                        normal
                    );
                    Float32 spec = powf(
                        fmaxf(
                            vec3Dot(viewDir, reflectDir),
                            0.0f
                        ),
                        32.0f
                    );
                    Vec3 specular = vec3Mul(scene->lights[i].color, spec);
                    Vec3 ambient = vec3Mul((Vec3){0.5f, 0.5f, 1.0f}, 0.1f);
                    color = vec3Add(color, vec3Add(vec3Add(ambient, vec3Mul(diffuse, 0.8f)), vec3Mul(specular, 0.5f)));
                }
                color = (Vec3) { fminf(color.x, 1.0f), fminf(color.y, 1.0f), fminf(color.z, 1.0f)};
                buffer->pix[y * buffer->w + x].r = (UInt8)(color.x * 255);
                buffer->pix[y * buffer->w + x].g = (UInt8)(color.y * 255);
                buffer->pix[y * buffer->w + x].b = (UInt8)(color.z * 255);
                buffer->pix[y * buffer->w + x].a = 255;
            }
            else
            {
                buffer->pix[y * buffer->w + x].r = (UInt8)(scene->backgroundColor.x * 255);
                buffer->pix[y * buffer->w + x].g = (UInt8)(scene->backgroundColor.y * 255);
                buffer->pix[y * buffer->w + x].b = (UInt8)(scene->backgroundColor.z * 255);
                buffer->pix[y * buffer->w + x].a = 255;
            }
        }
    }
}