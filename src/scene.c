#include "scene.h"
#include "frustum.h"
#include "noise.h"
#include "logger.h"
#include <stdlib.h>

#define MAX_REFLECTION_DEPTH 3

static Vec3 getCheckeredColor(Vec3 hitPoint, Material mat)
{
    Float32 scale = 1.0f;
    Int32 ix = (Int32)floorf(hitPoint.x * scale);
    Int32 iz = (Int32)floorf(hitPoint.z * scale);
    if((ix + iz) & 1)
        return (Vec3){mat.color.x * 0.3f, mat.color.y * 0.3f, mat.color.z * 0.3f};
    return mat.color;
}

typedef struct {
    Float32 t;
    Material material;
    Vec3 normal;
    Vec3 hitPoint;
    Vec2 uv;
    Bool hit;
} HitResult;

static Bool intersectAABB(Vec3 origin, Vec3 invDir, AABB b, Float32 tMax)
{
    Float32 tx1 = (b.min.x - origin.x) * invDir.x;
    Float32 tx2 = (b.max.x - origin.x) * invDir.x;
    Float32 tmin = fminf(tx1, tx2);
    Float32 tmax = fmaxf(tx1, tx2);
    Float32 ty1 = (b.min.y - origin.y) * invDir.y;
    Float32 ty2 = (b.max.y - origin.y) * invDir.y;
    tmin = fmaxf(tmin, fminf(ty1, ty2));
    tmax = fminf(tmax, fmaxf(ty1, ty2));
    Float32 tz1 = (b.min.z - origin.z) * invDir.z;
    Float32 tz2 = (b.max.z - origin.z) * invDir.z;
    tmin = fmaxf(tmin, fminf(tz1, tz2));
    tmax = fminf(tmax, fmaxf(tz1, tz2));
    return tmax >= tmin && tmin < tMax && tmax > 0;
}

static Void bvhIntersect(Scene* scene, Int32 nodeIdx, Vec3 origin, Vec3 dir, Vec3 invDir, HitResult* result)
{
    BVHNode* node = &scene->bvhNodes[nodeIdx];
    if(!intersectAABB(origin, invDir, node->bounds, result->t)) return;

    if(node->triCount > 0)
    {
        for(Int32 i = 0; i < node->triCount; ++i)
        {
            Float32 t;
            Vec3 normal;
            Vec2 triUv;
            if(geomTriangleIntersect(origin, dir, scene->triangles[node->left + i], &t, &normal, &triUv) && t < result->t)
            {
                result->t = t;
                result->material = scene->triangles[node->left + i].material;
                result->normal = normal;
                result->uv = triUv;
                result->hit = true;
            }
        }
    }
    else
    {
        bvhIntersect(scene, node->left, origin, dir, invDir, result);
        bvhIntersect(scene, node->left + 1, origin, dir, invDir, result);
    }
}

static HitResult traceRayInternal(Scene* scene, Vec3 origin, Vec3 dir)
{
    HitResult result = {.t = INFINITY, .hit = false};
    for(Int32 i = 0; i < scene->sphereCount; ++i)
    {
        Float32 t;
        Vec3 normal;
        Vec2 sphereUv;
        if(geomSphereIntersect(origin, dir, scene->spheres[i], &t, &normal, &sphereUv) && t < result.t)
        {
            result.t = t;
            result.material = scene->spheres[i].material;
            result.normal = normal;
            result.uv = sphereUv;
            result.hit = true;
        }
    }

    for(Int32 i = 0; i < scene->boxCount; ++i)
    {
        Float32 t;
        Vec3 normal;
        Vec2 boxUv;
        if(geomBoxIntersect(origin, dir, scene->boxes[i], &t, &normal, &boxUv) && t < result.t)
        {
            result.t = t;
            result.material = scene->boxes[i].material;
            result.normal = normal;
            result.uv = boxUv;
            result.hit = true;
        }
    }

    for(Int32 i = 0; i < scene->planeCount; ++i)
    {
        Float32 t;
        Vec3 normal;
        Vec2 planeUv;
        if(geomPlaneIntersect(origin, dir, scene->planes[i], &t, &normal, &planeUv) && t < result.t)
        {
            result.t = t;
            result.material = scene->planes[i].material;
            result.normal = normal;
            result.uv = planeUv;
            result.hit = true;
        }
    }

    if(scene->bvhRoot != -1)
    {
        Vec3 invDir = {1.0f / dir.x, 1.0f / dir.y, 1.0f / dir.z};
        bvhIntersect(scene, scene->bvhRoot, origin, dir, invDir, &result);
    }
    else
    {
        for(Int32 i = 0; i < scene->triangleCount; ++i)
        {
            Float32 t;
            Vec3 normal;
            Vec2 triUv;
            if(geomTriangleIntersect(origin, dir, scene->triangles[i], &t, &normal, &triUv) && t < result.t)
            {
                result.t = t;
                result.material = scene->triangles[i].material;
                result.normal = normal;
                result.uv = triUv;
                result.hit = true;
            }
        }
    }

    if(result.hit)
    {
        result.hitPoint = vec3Add(origin, vec3Mul(dir, result.t));
    }
    return result;
}

static Bool bvhIntersectShadow(Scene* scene, Int32 nodeIdx, Vec3 origin, Vec3 dir, Vec3 invDir, Float32 maxT)
{
    BVHNode* node = &scene->bvhNodes[nodeIdx];
    if(!intersectAABB(origin, invDir, node->bounds, maxT)) return false;
    if(node->triCount > 0)
    {
        for(Int32 i = 0; i < node->triCount; ++i)
        {
            Float32 t;
            Vec3 n;
            Vec2 uv;
            if(geomTriangleIntersect(origin, dir, scene->triangles[node->left + i], &t, &n, &uv) && t < maxT) return true;
        }
    }
    else
    {
        if(bvhIntersectShadow(scene, node->left, origin, dir, invDir, maxT)) return true;
        if(bvhIntersectShadow(scene, node->left + 1, origin, dir, invDir, maxT)) return true;
    }
    return false;
}

static Bool traceRayShadow(Scene* scene, Vec3 origin, Vec3 dir, Float32 maxT)
{
    for(Int32 i = 0; i < scene->sphereCount; ++i)
    {
        Float32 t;
        Vec3 n;
        Vec2 uv;
        if(geomSphereIntersect(origin, dir, scene->spheres[i], &t, &n, &uv) && t < maxT) return true;
    }
    for(Int32 i = 0; i < scene->boxCount; ++i)
    {
        Float32 t;
        Vec3 n;
        Vec2 uv;
        if(geomBoxIntersect(origin, dir, scene->boxes[i], &t, &n, &uv) && t < maxT) return true;
    }
    for(Int32 i = 0; i < scene->planeCount; ++i)
    {
        Float32 t;
        Vec3 n;
        Vec2 uv;
        if(geomPlaneIntersect(origin, dir, scene->planes[i], &t, &n, &uv) && t < maxT) return true;
    }
    if(scene->bvhRoot != -1)
    {
        Vec3 invDir = {1.0f/dir.x, 1.0f/dir.y, 1.0f/dir.z};
        return bvhIntersectShadow(scene, scene->bvhRoot, origin, dir, invDir, maxT);
    }
    for(Int32 i = 0; i < scene->triangleCount; ++i)
    {
        Float32 t;
        Vec3 n;
        Vec2 uv;
        if(geomTriangleIntersect(origin, dir, scene->triangles[i], &t, &n, &uv) && t < maxT) return true;
    }
    return false;
}

static Vec3 shadeHit(Scene* scene, Vec3 hitPoint, Vec3 normal, Vec3 viewDir, Material material, Vec3 rayDir, Vec2 uv, Int32 depth);

static Vec3 getSkyColor(Vec3 dir)
{
    Float32 t = 0.5f * (dir.y + 1.0f);
    // Lerp between {1,1,1} and {0.5, 0.7, 1}
    // {1-0.5t, 1-0.3t, 1}
    return (Vec3){1.0f - 0.5f * t, 1.0f - 0.3f * t, 1.0f};
}

static Vec3 traceRay(Scene* scene, Vec3 origin, Vec3 dir, Int32 depth)
{
    if(depth > MAX_REFLECTION_DEPTH)
        return getSkyColor(dir);

    HitResult hit = traceRayInternal(scene, origin, dir);
    if(!hit.hit)
        return getSkyColor(dir);

    Vec3 viewDir = {-dir.x, -dir.y, -dir.z}; // dir is assumed normalized
    return shadeHit(scene, hit.hitPoint, hit.normal, viewDir, hit.material, dir, hit.uv, depth);
}

static Vec3 shadeHit(Scene* scene, Vec3 hitPoint, Vec3 normal, Vec3 viewDir, Material material, Vec3 rayDir, Vec2 uv, Int32 depth)
{
    Vec3 baseColor = material.color;
    if(material.textureType == TEXTURE_CHECKERED)
        baseColor = getCheckeredColor(hitPoint, material);
    else if(material.textureType == TEXTURE_NOISE)
    {
        Float32 n = noiseFractal3D(hitPoint.x * 2.0f, hitPoint.y * 2.0f, hitPoint.z * 2.0f, 6);
        n = (n + 1.0f) * 0.5f; // [0, 1]
        baseColor = vec3Mul(material.color, n);
    }
    else if(material.textureType == TEXTURE_IMAGE && material.texture)
    {
        Int32 tx = (Int32)(uv.x * material.texture->w) % material.texture->w;
        Int32 ty = (Int32)(uv.y * material.texture->h) % material.texture->h;
        if(tx < 0) tx += material.texture->w;
        if(ty < 0) ty += material.texture->h;
        TPixel p = material.texture->pix[ty * material.texture->w + tx];
        baseColor = (Vec3){p.r / 255.0f, p.g / 255.0f, p.b / 255.0f};
        baseColor = (Vec3){baseColor.x * material.color.x, baseColor.y * material.color.y, baseColor.z * material.color.z};
    }
    Vec3 color = {0, 0, 0};

    for(Int32 i = 0; i < scene->lightCount; ++i)
    {
        Light light = scene->lights[i];
        Vec3 lightDir = vec3Norm(vec3Sub(light.pos, hitPoint));
        Float32 atten = 1.0f;

        if(light.type == LIGHT_SPOT)
        {
            Float32 theta = vec3Dot(lightDir, vec3Norm(vec3Mul(light.dir, -1.0f)));
            Float32 eps = light.innerCutoff - light.outerCutoff;
            atten = clamp((theta - light.outerCutoff) / eps, 0.0f, 1.0f);
        }

        Float32 diff = fmaxf(vec3Dot(normal, lightDir), 0.0f) * material.diffuse;
        Float32 shadow = 0.0f;
        Int32 samples = (light.type == LIGHT_AREA) ? 4 : 1;
        Vec3 shadowOrigin = vec3Add(hitPoint, vec3Mul(normal, 0.001f));

        for(Int32 s = 0; s < samples; ++s)
        {
            Vec3 lpos = light.pos;
            if(light.type == LIGHT_AREA)
            {
                Float32 r = light.radius;
                if(s == 1) lpos.x += r; else if(s == 2) lpos.x -= r;
                else if(s == 3) lpos.z += r; else if(s == 4) lpos.z -= r;
            }
            Vec3 ldir = vec3Sub(lpos, shadowOrigin);
            Float32 d = vec3Len(ldir);
            ldir = vec3Mul(ldir, 1.0f / d);
            if(!traceRayShadow(scene, shadowOrigin, ldir, d)) shadow += 1.0f;
            else shadow += 0.3f;
        }
        shadow /= samples;

        Vec3 colorProduct = {baseColor.x * light.color.x, baseColor.y * light.color.y, baseColor.z * light.color.z};
        Vec3 diffuse = vec3Mul(colorProduct, diff * shadow * atten);

        Vec3 reflectDir = vec3Reflect(vec3Mul(lightDir, -1.0f), normal);
        Float32 specDot = fmaxf(vec3Dot(viewDir, reflectDir), 0.0f);
        Float32 spec = powf(fmaxf(vec3Dot(viewDir, reflectDir), 0.0f), material.shininess);
        Vec3 specular = vec3Mul(light.color, spec * 0.8f * shadow * atten);
        Vec3 ambient = vec3Mul(baseColor, 0.15f / (scene->lightCount > 0 ? scene->lightCount : 1));
        color = vec3Add(color, vec3Add(ambient, vec3Add(diffuse, specular)));
    }

    if(material.reflectivity > 0.0f && depth < MAX_REFLECTION_DEPTH)
    {
        Vec3 reflectDir = vec3Reflect(rayDir, normal);
        Vec3 reflectOrigin = vec3Add(hitPoint, vec3Mul(normal, 0.001f));
        Vec3 reflectColor = traceRay(scene, reflectOrigin, reflectDir, depth + 1);
        color = vec3Add(vec3Mul(color, 1.0f - material.reflectivity), vec3Mul(reflectColor, material.reflectivity));
    }

    if(material.transparency > 0.0f && depth < MAX_REFLECTION_DEPTH)
    {
        Float32 ior = material.refractiveIndex > 0 ? material.refractiveIndex : 1.0f;
        Float32 eta = vec3Dot(rayDir, normal) > 0 ? ior : 1.0f / ior;
        Vec3 n = vec3Dot(rayDir, normal) > 0 ? vec3Mul(normal, -1.0f) : normal;
        Vec3 refractDir = vec3Refract(rayDir, n, eta);

        if(vec3Len(refractDir) > 0.1f)
        {
            Vec3 refractOrigin = vec3Add(hitPoint, vec3Mul(n, -0.001f));
            Vec3 refractColor = traceRay(scene, refractOrigin, refractDir, depth + 1);
            color = vec3Add(vec3Mul(color, 1.0f - material.transparency), vec3Mul(refractColor, material.transparency));
        }
    }

    return color;
}

Scene* createScene(Camera camera, Vec3 backgroundColor)
{
    Scene* scene = (Scene*) malloc(sizeOf(Scene));
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
    scene->modelCount = 0;
    scene->modelCapacity = 0;
    scene->models = null;
    scene->labelCount = 0;
    scene->labelCapacity = 0;
    scene->labels = null;
    scene->bvhNodes = null;
    scene->bvhNodeCount = 0;
    scene->bvhNodeCapacity = 0;
    scene->bvhRoot = -1;
    scene->textures = null;
    scene->textureCount = 0;
    scene->textureCapacity = 0;
    scene->backgroundColor = backgroundColor;
    LOG_I("Scene created.");
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
        free(scene->models);
        free(scene->labels);
        free(scene->bvhNodes);
        if(scene->textures)
        {
            for(Int32 i = 0; i < scene->textureCount; ++i)
                tigrFree(scene->textures[i]);
            free(scene->textures);
        }
        free(scene);
    }
}

Tigr* sceneLoadTexture(Scene* scene, CharSeq path)
{
    Tigr* tex = tigrLoadImage(path);
    if(!tex) return null;

    if(scene->textureCount == scene->textureCapacity)
    {
        scene->textureCapacity = scene->textureCapacity ? scene->textureCapacity * 2 : 4;
        scene->textures = (Tigr**)realloc(scene->textures, sizeof(Tigr*) * scene->textureCapacity);
    }
    scene->textures[scene->textureCount++] = tex;
    return tex;
}

Tigr* sceneCreateDebugTexture(Scene* scene, Int32 width, Int32 height)
{
    Tigr* tex = tigrBitmap(width, height);
    if(!tex) return null;

    for(Int32 y = 0; y < height; ++y)
    {
        for(Int32 x = 0; x < width; ++x)
        {
            // Create a 16x16 valve-style checker pattern
            Int32 cx = x / 16;
            Int32 cy = y / 16;
            if((cx + cy) % 2 == 0)
                tex->pix[y * width + x] = tigrRGB(255, 0, 255); // Magenta
            else
                tex->pix[y * width + x] = tigrRGB(0, 0, 0); // Black
        }
    }

    if(scene->textureCount == scene->textureCapacity)
    {
        scene->textureCapacity = scene->textureCapacity ? scene->textureCapacity * 2 : 4;
        scene->textures = (Tigr**)realloc(scene->textures, sizeof(Tigr*) * scene->textureCapacity);
    }
    scene->textures[scene->textureCount++] = tex;
    return tex;
}

Void sceneAddModel(Scene* scene, ObjModel model)
{
    if(scene->modelCount >= scene->modelCapacity)
    {
        scene->modelCapacity = scene->modelCapacity == 0 ? 4 : scene->modelCapacity * 2;
        scene->models = (ObjModel*)realloc(scene->models, sizeOf(ObjModel) * scene->modelCapacity);
    }
    scene->models[scene->modelCount++] = model;
}

Void sceneAddDebugLabel(Scene* scene, Vec3 pos, CharSeq text, TPixel color)
{
    if(scene->labelCount >= scene->labelCapacity)
    {
        scene->labelCapacity = scene->labelCapacity == 0 ? 4 : scene->labelCapacity * 2;
        scene->labels = (DebugLabel*)realloc(scene->labels, sizeOf(DebugLabel) * scene->labelCapacity);
    }
    scene->labels[scene->labelCount++] = (DebugLabel){pos, text, color};
}

Void sceneAddSphere(Scene* scene, GeomSphere sphere)
{
    LOG_D("Adding sphere at (%.1f, %.1f, %.1f)", sphere.center.x, sphere.center.y, sphere.center.z);
    sphere.radiusSq = sphere.radius * sphere.radius;
    sphere.invRadius = 1.0f / sphere.radius;
    if(scene->sphereCount >= scene->sphereCapacity)
    {
        scene->sphereCapacity = scene->sphereCapacity == 0 ? 4 : scene->sphereCapacity * 2;
        scene->spheres = (GeomSphere*) realloc(scene->spheres, sizeOf(GeomSphere) * scene->sphereCapacity);
    }
    scene->spheres[scene->sphereCount++] = sphere;
}

Void sceneAddBox(Scene* scene, GeomBox box)
{
    LOG_D("Adding box at (%.1f, %.1f, %.1f)", box.center.x, box.center.y, box.center.z);
    box.min = vec3Sub(box.center, box.extents);
    box.max = vec3Add(box.center, box.extents);
    if(scene->boxCount >= scene->boxCapacity)
    {
        scene->boxCapacity = scene->boxCapacity == 0 ? 4 : scene->boxCapacity * 2;
        scene->boxes = (GeomBox*) realloc(scene->boxes, sizeOf(GeomBox) * scene->boxCapacity);
    }
    scene->boxes[scene->boxCount++] = box;
}

Void sceneAddPlane(Scene* scene, GeomPlane plane)
{
    LOG_D("Adding plane with normal (%.1f, %.1f, %.1f)", plane.normal.x, plane.normal.y, plane.normal.z);
    if(scene->planeCount >= scene->planeCapacity)
    {
        scene->planeCapacity = scene->planeCapacity == 0 ? 4 : scene->planeCapacity * 2;
        scene->planes = (GeomPlane*) realloc(scene->planes, sizeOf(GeomPlane) * scene->planeCapacity);
    }
    scene->planes[scene->planeCount++] = plane;
}

Void sceneAddTriangle(Scene* scene, GeomTriangle triangle)
{
    LOG_D("Adding triangle.");
    triangle.edge1 = vec3Sub(triangle.p1, triangle.p0);
    triangle.edge2 = vec3Sub(triangle.p2, triangle.p0);
    triangle.normal = vec3Norm(vec3Cross(triangle.edge1, triangle.edge2));
    if(scene->triangleCount >= scene->triangleCapacity)
    {
        scene->triangleCapacity = scene->triangleCapacity == 0 ? 4 : scene->triangleCapacity * 2;
        scene->triangles = (GeomTriangle*) realloc(scene->triangles, sizeOf(GeomTriangle) * scene->triangleCapacity);
    }
    scene->triangles[scene->triangleCount++] = triangle;
}

Void sceneAddLight(Scene* scene, Light light)
{
    LOG_I("Adding light at (%.1f, %.1f, %.1f) Type: %d", light.pos.x, light.pos.y, light.pos.z, light.type);
    if(scene->lightCount >= scene->lightCapacity)
    {
        scene->lightCapacity = scene->lightCapacity == 0 ? 4 : scene->lightCapacity * 2;
        scene->lights = (Light*) realloc(scene->lights, sizeOf(Light) * scene->lightCapacity);
    }
    scene->lights[scene->lightCount++] = light;
}

Void sceneRender(Scene* scene, Tigr* buffer)
{
    Float32 aspect = (Float32) buffer->w / buffer->h;
    Vec3 camDir = scene->camera.dir;
    Vec3 camUp = scene->camera.up;
    Vec3 right = vec3Norm(vec3Cross(camDir, camUp));
    Vec3 localUp = vec3Norm(vec3Cross(right, camDir));
    Float32 fovScale = tanf(scene->camera.fov / 2.0f);
    Float32 invW = 1.0f / buffer->w;
    Float32 invH = 1.0f / buffer->h;
    #pragma omp parallel for collapse(2) schedule(dynamic)
    for(Int32 y = 0; y < buffer->h; ++y)
    {
        for(Int32 x = 0; x < buffer->w; ++x)
        {
            Vec3 finalColor = {0, 0, 0};
            const Int32 samples = 2;
            const Float32 invSamples = 1.0f / samples;
            for(Int32 sy = 0; sy < samples; ++sy)
            {
                for(Int32 sx = 0; sx < samples; ++sx)
                {
                    Float32 fx = x + (sx + 0.5f) * invSamples;
                    Float32 fy = y + (sy + 0.5f) * invSamples;
                    Float32 u = (2.0f * fx * invW - 1.0f) * fovScale * aspect;
                    Float32 v = (1.0f - 2.0f * fy * invH) * fovScale;
                    Vec3 rayDir = vec3Norm(vec3Add(vec3Add(camDir, vec3Mul(right, u)), vec3Mul(localUp, v)));
                    finalColor = vec3Add(finalColor, traceRay(scene, scene->camera.pos, rayDir, 0));
                }
            }
            finalColor = vec3Mul(finalColor, 1.0f / (samples * samples));
            finalColor = (Vec3) { fminf(finalColor.x, 1.0f), fminf(finalColor.y, 1.0f), fminf(finalColor.z, 1.0f) };
            buffer->pix[y * buffer->w + x].r = (UInt8)(finalColor.x * 255);
            buffer->pix[y * buffer->w + x].g = (UInt8)(finalColor.y * 255);
            buffer->pix[y * buffer->w + x].b = (UInt8)(finalColor.z * 255);
            buffer->pix[y * buffer->w + x].a = 255;
        }
    }
    #if DEBUG_LIGHT_BOUNDS
    extern Float32 RENDER_SCALE;
    for(Int32 i = 0; i < scene->lightCount; ++i)
    {
        geomDrawDebugBounds(buffer, scene->lights[i].pos, 5.0f, scene->camera, 0.4f);
    }
    #endif
}

Void sceneAddSphereSimple(Scene* scene, Vec3 center, Float32 radius, Vec3 color)
{
    GeomSphere sphere = {.center = center, .radius = radius, .material = {{color.x, color.y, color.z}, 32.0f, 0.8f, 0.0f, 0.0f, 1.0f, TEXTURE_NONE, null}};
    sceneAddSphere(scene, sphere);
}

Void sceneAddBoxSimple(Scene* scene, Vec3 center, Vec3 extents, Vec3 color)
{
    GeomBox box = {.center = center, .extents = extents, .material = {{color.x, color.y, color.z}, 24.0f, 0.75f, 0.0f, 0.0f, 1.0f, TEXTURE_NONE, null}};
    sceneAddBox(scene, box);
}

Void sceneAddPlaneSimple(Scene* scene, Vec3 normal, Float32 distance, Vec3 color)
{
    GeomPlane plane = {.normal = normal, .distance = distance, .material = {{color.x, color.y, color.z}, 50.0f, 0.9f, 0.1f, 0.0f, 1.0f, TEXTURE_CHECKERED, null}};
    sceneAddPlane(scene, plane);
}

Void sceneAddLightSimple(Scene* scene, Vec3 pos, Vec3 color, Float32 intensity)
{
    Light light = {LIGHT_POINT, pos, {color.x * intensity, color.y * intensity, color.z * intensity}, {0, -1, 0}, 0.9f, 0.8f, 0};
    sceneAddLight(scene, light);
}

static AABB getTriangleBounds(GeomTriangle tri)
{
    AABB b;
    b.min.x = fminf(tri.p0.x, fminf(tri.p1.x, tri.p2.x));
    b.min.y = fminf(tri.p0.y, fminf(tri.p1.y, tri.p2.y));
    b.min.z = fminf(tri.p0.z, fminf(tri.p1.z, tri.p2.z));
    b.max.x = fmaxf(tri.p0.x, fmaxf(tri.p1.x, tri.p2.x));
    b.max.y = fmaxf(tri.p0.y, fmaxf(tri.p1.y, tri.p2.y));
    b.max.z = fmaxf(tri.p0.z, fmaxf(tri.p1.z, tri.p2.z));
    return b;
}

static Void updateNodeBounds(Scene* scene, Int32 nodeIdx)
{
    BVHNode* node = &scene->bvhNodes[nodeIdx];
    node->bounds.min = (Vec3){INFINITY, INFINITY, INFINITY};
    node->bounds.max = (Vec3){-INFINITY, -INFINITY, -INFINITY};
    for(Int32 i = 0; i < node->triCount; ++i)
    {
        AABB b = getTriangleBounds(scene->triangles[node->left + i]);
        node->bounds.min.x = fminf(node->bounds.min.x, b.min.x);
        node->bounds.min.y = fminf(node->bounds.min.y, b.min.y);
        node->bounds.min.z = fminf(node->bounds.min.z, b.min.z);
        node->bounds.max.x = fmaxf(node->bounds.max.x, b.max.x);
        node->bounds.max.y = fmaxf(node->bounds.max.y, b.max.y);
        node->bounds.max.z = fmaxf(node->bounds.max.z, b.max.z);
    }
}

static Void subdivide(Scene* scene, Int32 nodeIdx)
{
    BVHNode* node = &scene->bvhNodes[nodeIdx];
    if(node->triCount <= 2)
    {
        return;
    }
    Vec3 extent = vec3Sub(node->bounds.max, node->bounds.min);
    Int32 axis = 0;
    if(extent.y > extent.x)
    {
        axis = 1;
    }
    if(extent.z > (axis == 1 ? extent.y : extent.x))
    {
        axis = 2;
    }
    Float32 splitPos = ((Float32*) &node->bounds.min)[axis] + ((Float32*)&extent)[axis] * 0.5f;
    Int32 i = node->left;
    Int32 j = i + node->triCount - 1;
    while(i <= j)
    {
        Vec3 cent = vec3Mul(vec3Add(scene->triangles[i].p0, vec3Add(scene->triangles[i].p1, scene->triangles[i].p2)), 0.3333f);
        if(((Float32*)&cent)[axis] < splitPos)
        {
            i++;
        }
        else
        {
            GeomTriangle tmp = scene->triangles[i];
            scene->triangles[i] = scene->triangles[j];
            scene->triangles[j] = tmp;
            j--;
        }
    }
    Int32 leftCount = i - node->left;
    if(leftCount == 0 || leftCount == node->triCount)
    {
        return;
    }
    Int32 leftChildIdx = scene->bvhNodeCount++;
    Int32 rightChildIdx = scene->bvhNodeCount++;
    Int32 triStart = node->left;
    Int32 triCount = node->triCount;
    scene->bvhNodes[nodeIdx].left = leftChildIdx;
    scene->bvhNodes[nodeIdx].triCount = 0;
    scene->bvhNodes[leftChildIdx].left = triStart;
    scene->bvhNodes[leftChildIdx].triCount = leftCount;
    updateNodeBounds(scene, leftChildIdx);
    scene->bvhNodes[rightChildIdx].left = i;
    scene->bvhNodes[rightChildIdx].triCount = triCount - leftCount;
    updateNodeBounds(scene, rightChildIdx);
    subdivide(scene, leftChildIdx);
    subdivide(scene, rightChildIdx);
}

Void sceneBuildBVH(Scene* scene)
{
    if(scene->triangleCount == 0)
    {
        return;
    }
    LOG_I("Building BVH for %d triangles...", scene->triangleCount);
    scene->bvhNodeCapacity = scene->triangleCount * 2;
    scene->bvhNodes = (BVHNode*)malloc(sizeOf(BVHNode) * scene->bvhNodeCapacity);
    scene->bvhNodeCount = 1;
    scene->bvhNodes[0].left = 0;
    scene->bvhNodes[0].triCount = scene->triangleCount;
    updateNodeBounds(scene, 0);
    scene->bvhRoot = 0;
    subdivide(scene, 0);
    LOG_I("BVH built with %d nodes.", scene->bvhNodeCount);
}

Void sceneAddSpotLight(Scene* scene, Vec3 pos, Vec3 dir, Float32 inner, Float32 outer, Vec3 color, Float32 intensity)
{
    Light l = {LIGHT_SPOT, pos, {color.x * intensity, color.y * intensity, color.z * intensity}, dir, cosf(inner * PI / 180.0f), cosf(outer * PI / 180.0f), 0};
    sceneAddLight(scene, l);
}

Void sceneAddAreaLight(Scene* scene, Vec3 pos, Float32 radius, Vec3 color, Float32 intensity)
{
    Light l = {LIGHT_AREA, pos, {color.x * intensity, color.y * intensity, color.z * intensity}, {0}, 0, 0, radius};
    sceneAddLight(scene, l);
}

Void sceneAddDefaultLights(Scene* scene)
{
    sceneAddLightSimple(scene, (Vec3){2.0f, 4.0f, -3.0f}, (Vec3){1.0f, 1.0f, 1.0f}, 1.2f);
    sceneAddSpotLight(scene, (Vec3){-5.0f, 5.0f, 5.0f}, (Vec3){0.5f, -1.0f, 0.0f}, 15.0f, 25.0f, (Vec3){0.6f, 0.6f, 1.0f}, 2.0f);
    sceneAddAreaLight(scene, (Vec3){8.0f, 6.0f, 8.0f}, 1.5f, (Vec3){1.0f, 0.5f, 0.5f}, 1.5f);
}

Void sceneAddDefaultGround(Scene* scene)
{
    sceneAddPlaneSimple(scene, (Vec3){0.0f, 1.0f, 0.0f}, -2.0f, (Vec3){0.8f, 0.8f, 0.8f});
}