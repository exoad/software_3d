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
    scene->boxes = null;
    scene->boxCount = 0;
    scene->boxCapacity = 0;
    scene->planes = null;
    scene->planeCount = 0;
    scene->planeCapacity = 0;
    scene->triangles = null;
    scene->triangleCount = 0;
    scene->triangleCapacity = 0;
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
        free(scene->boxes);
        free(scene->planes);
        free(scene->triangles);
        free(scene->lights);
        free(scene);
    }
}

Void sceneAddSphere(Scene* scene, GeomSphere sphere)
{
    if(scene->sphereCount >= scene->sphereCapacity)
    {
        scene->sphereCapacity = scene->sphereCapacity == 0 ? 4 : scene->sphereCapacity * 2;
        scene->spheres = (GeomSphere*) realloc(scene->spheres, sizeof(GeomSphere) * scene->sphereCapacity);
    }
    scene->spheres[scene->sphereCount++] = sphere;
}

Void sceneAddBox(Scene* scene, GeomBox box)
{
    if(scene->boxCount >= scene->boxCapacity)
    {
        scene->boxCapacity = scene->boxCapacity == 0 ? 4 : scene->boxCapacity * 2;
        scene->boxes = (GeomBox*) realloc(scene->boxes, sizeof(GeomBox) * scene->boxCapacity);
    }
    scene->boxes[scene->boxCount++] = box;
}

Void sceneAddPlane(Scene* scene, GeomPlane plane)
{
    if(scene->planeCount >= scene->planeCapacity)
    {
        scene->planeCapacity = scene->planeCapacity == 0 ? 4 : scene->planeCapacity * 2;
        scene->planes = (GeomPlane*) realloc(scene->planes, sizeof(GeomPlane) * scene->planeCapacity);
    }
    scene->planes[scene->planeCount++] = plane;
}

Void sceneAddTriangle(Scene* scene, GeomTriangle triangle)
{
    if(scene->triangleCount >= scene->triangleCapacity)
    {
        scene->triangleCapacity = scene->triangleCapacity == 0 ? 4 : scene->triangleCapacity * 2;
        scene->triangles = (GeomTriangle*) realloc(scene->triangles, sizeof(GeomTriangle) * scene->triangleCapacity);
    }
    scene->triangles[scene->triangleCount++] = triangle;
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
    #pragma omp parallel for collapse(2) schedule(dynamic)
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
            GeomSphere* hitSphere = null;
            Material hitMaterial = {0};
            Vec3 hitNormal = {0};

            // Check spheres
            for(Int32 i = 0; i < scene->sphereCount; ++i)
            {
                Float32 t;
                Vec3 normal;
                if(geomSphereIntersect(scene->camera.pos, rayDir, scene->spheres[i], &t, &normal) && t < closestT)
                {
                    closestT = t;
                    hitSphere = &scene->spheres[i];
                    hitMaterial = scene->spheres[i].material;
                    hitNormal = normal;
                }
            }

            // Check boxes
            for(Int32 i = 0; i < scene->boxCount; ++i)
            {
                Float32 t;
                Vec3 normal;
                if(geomBoxIntersect(scene->camera.pos, rayDir, scene->boxes[i], &t, &normal) && t < closestT)
                {
                    closestT = t;
                    hitSphere = null;
                    hitMaterial = scene->boxes[i].material;
                    hitNormal = normal;
                }
            }

            // Check planes
            for(Int32 i = 0; i < scene->planeCount; ++i)
            {
                Float32 t;
                Vec3 normal;
                if(geomPlaneIntersect(scene->camera.pos, rayDir, scene->planes[i], &t, &normal) && t < closestT)
                {
                    closestT = t;
                    hitSphere = null;
                    hitMaterial = scene->planes[i].material;
                    hitNormal = normal;
                }
            }

            // Check triangles
            for(Int32 i = 0; i < scene->triangleCount; ++i)
            {
                Float32 t;
                Vec3 normal;
                if(geomTriangleIntersect(scene->camera.pos, rayDir, scene->triangles[i], &t, &normal) && t < closestT)
                {
                    closestT = t;
                    hitSphere = null;
                    hitMaterial = scene->triangles[i].material;
                    hitNormal = normal;
                }
            }

            if(hitSphere != null || closestT < INFINITY)
            {
                Vec3 hit = vec3Add(scene->camera.pos, vec3Mul(rayDir, closestT));
                Vec3 normal = hitNormal;
                Vec3 viewDir = vec3Norm(vec3Sub(scene->camera.pos, hit));
                Vec3 color = {0, 0, 0};
                for(Int32 i = 0; i < scene->lightCount; ++i)
                {
                    Vec3 lightDir = vec3Norm(vec3Sub(scene->lights[i].pos, hit));
                    Float32 diff = fmaxf(vec3Dot(normal, lightDir), 0.0f) * hitMaterial.diffuse;

                    // Element-wise multiply for colors
                    Vec3 colorProduct = {hitMaterial.color.x * scene->lights[i].color.x, hitMaterial.color.y * scene->lights[i].color.y, hitMaterial.color.z * scene->lights[i].color.z};
                    Vec3 diffuse = vec3Mul(colorProduct, diff);

                    Vec3 reflectDir = vec3Reflect(vec3Mul(lightDir, -1.0f), normal);
                    Float32 spec = powf(fmaxf(vec3Dot(viewDir, reflectDir), 0.0f), hitMaterial.shininess);
                    Vec3 specular = vec3Mul(scene->lights[i].color, spec * 0.5f);
                    Vec3 ambient = vec3Mul(hitMaterial.color, 0.1f);
                    color = vec3Add(color, vec3Add(ambient, vec3Add(diffuse, specular)));
                }
                color = (Vec3) { fminf(color.x, 1.0f), fminf(color.y, 1.0f), fminf(color.z, 1.0f) };
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

    #if DEBUG_LIGHT_BOUNDS
    // Draw debug bounds for each light
    extern Float32 RENDER_SCALE;
    for(Int32 i = 0; i < scene->lightCount; ++i)
    {
        geomDrawDebugBounds(buffer, scene->lights[i].pos, 5.0f, scene->camera, 0.4f);
    }
    #endif
}