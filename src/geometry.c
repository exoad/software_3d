#include "geometry.h"
#include "tigr.h"
#include <stdlib.h>

Bool geomSphereIntersect(Vec3 rayOrigin, Vec3 rayDir, GeomSphere sphere, Float32* t, Vec3* normal, Vec2* uv)
{
    Vec3 oc = vec3Sub(rayOrigin, sphere.center);
    Float32 b = vec3Dot(oc, rayDir);
    Float32 c = vec3Dot(oc, oc) - sphere.radiusSq;
    Float32 h = b * b - c;
    if(h < 0.0f)
    {
        return false;
    }
    h = sqrtf(h);
    Float32 tval = -b - h;
    if(tval < 0.001f)
    {
        tval = -b + h;
    }
    if(tval < 0.001f)
    {
        return false;
    }
    *t = tval;
    Vec3 hit = vec3Add(rayOrigin, vec3Mul(rayDir, tval));
    Vec3 n = vec3Mul(vec3Sub(hit, sphere.center), sphere.invRadius);
    *normal = n;
    if(uv)
    {
        uv->x = 0.5f + atan2f(n.z, n.x) / (2.0f * PI);
        uv->y = 0.5f - asinf(n.y) / PI;
    }
    return true;
}

Bool geomTriangleIntersect(Vec3 rayOrigin, Vec3 rayDir, GeomTriangle tri, Float32* t, Vec3* normal, Vec2* uv)
{
    Vec3 h = vec3Cross(rayDir, tri.edge2);
    Float32 a = vec3Dot(tri.edge1, h);
    if(a > -1e-6f && a < 1e-6f)
    {
        return false;
    }
    Float32 f = 1.0f / a;
    Vec3 s = vec3Sub(rayOrigin, tri.p0);
    Float32 u = f * vec3Dot(s, h);
    if(u < 0.0f || u > 1.0f)
    {
        return false;
    }
    Vec3 q = vec3Cross(s, tri.edge1);
    Float32 v = f * vec3Dot(rayDir, q);
    if(v < 0.0f || u + v > 1.0f)
    {
        return false;
    }
    Float32 tval = f * vec3Dot(tri.edge2, q);
    if(tval > 1e-6f)
    {
        *t = tval;
        *normal = tri.normal;
        if(vec3Dot(rayDir, *normal) > 0.0f)
        {
            *normal = vec3Mul(*normal, -1.0f);
        }
        if(uv)
        {
            Float32 w = 1.0f - u - v;
            uv->x = w * tri.uv0.x + u * tri.uv1.x + v * tri.uv2.x;
            uv->y = w * tri.uv0.y + u * tri.uv1.y + v * tri.uv2.y;
        }
        return true;
    }
    return false;
}

Bool geomPlaneIntersect(Vec3 rayOrigin, Vec3 rayDir, GeomPlane plane, Float32* t, Vec3* normal, Vec2* uv)
{
    if(uv)
    {
        uv->x = 0;
        uv->y = 0;
    } // Default
    Float32 denom = vec3Dot(plane.normal, rayDir);
    if(fabsf(denom) < 1e-6f)
    {
        return false;
    }
    Float32 tval = (plane.distance - vec3Dot(plane.normal, rayOrigin)) / denom;
    if(tval > 1e-6f)
    {
        *t = tval;
        *normal = plane.normal;
        if(denom > 0.0f)
        {
            *normal = vec3Mul(plane.normal, -1.0f);
        }
        return true;
    }
    return false;
}

Bool geomBoxIntersect(Vec3 rayOrigin, Vec3 rayDir, GeomBox box, Float32* t, Vec3* normal, Vec2* uv)
{
    if(uv) { uv->x = 0; uv->y = 0; }
    Float32 eps = 1e-6f;
    Vec3 invDir = {1.0f / rayDir.x, 1.0f / rayDir.y, 1.0f / rayDir.z};
    Float32 tx1 = (box.min.x - rayOrigin.x) * invDir.x;
    Float32 tx2 = (box.max.x - rayOrigin.x) * invDir.x;
    Float32 txMin = fminf(tx1, tx2);
    Float32 txMax = fmaxf(tx1, tx2);
    Float32 ty1 = (box.min.y - rayOrigin.y) * invDir.y;
    Float32 ty2 = (box.max.y - rayOrigin.y) * invDir.y;
    Float32 tyMin = fminf(ty1, ty2);
    Float32 tyMax = fmaxf(ty1, ty2);
    Float32 tz1 = (box.min.z - rayOrigin.z) * invDir.z;
    Float32 tz2 = (box.max.z - rayOrigin.z) * invDir.z;
    Float32 tzMin = fminf(tz1, tz2);
    Float32 tzMax = fmaxf(tz1, tz2);
    Float32 tEnter = fmaxf(fmaxf(txMin, tyMin), tzMin);
    Float32 tExit = fminf(fminf(txMax, tyMax), tzMax);
    if(tEnter > tExit || tExit < 1e-6f)
    {
        return false;
    }
    if(tEnter > 1e-6f)
    {
        *t = tEnter;
        // determine normal based on face was hit
        if(fabsf(tEnter - txMin) < eps)
        {
            *normal = vec3Mul((Vec3){-1.0f, 0.0f, 0.0f}, rayDir.x > 0 ? 1.0f : -1.0f);
        }
        else if(fabsf(tEnter - txMax) < eps)
        {
            *normal = vec3Mul((Vec3){1.0f, 0.0f, 0.0f}, rayDir.x > 0 ? 1.0f : -1.0f);
        }
        else if(fabsf(tEnter - tyMin) < eps)
        {
            *normal = vec3Mul((Vec3){0.0f, -1.0f, 0.0f}, rayDir.y > 0 ? 1.0f : -1.0f);
        }
        else if(fabsf(tEnter - tyMax) < eps)
        {
            *normal = vec3Mul((Vec3){0.0f, 1.0f, 0.0f}, rayDir.y > 0 ? 1.0f : -1.0f);
        }
        else if(fabsf(tEnter - tzMin) < eps)
        {
            *normal = vec3Mul((Vec3){0.0f, 0.0f, -1.0f}, rayDir.z > 0 ? 1.0f : -1.0f);
        }
        else
        {
            *normal = vec3Mul((Vec3){0.0f, 0.0f, 1.0f}, rayDir.z > 0 ? 1.0f : -1.0f);
        }
        return true;
    }
    return false;
}

Bool geomTriangularPrismIntersect(Vec3 rayOrigin, Vec3 rayDir, GeomTriangularPrism prism, Float32* t, Vec3* normal, Vec2* uv, Material* material)
{
    Float32 t1 = INFINITY;
    Vec3 n1;
    Vec2 uv1;
    if(geomTriangleIntersect(rayOrigin, rayDir, prism.tri1, &t1, &n1, &uv1))
    {
        *t = t1;
        *normal = n1;
        if(uv)
        {
            *uv = uv1;
        }
        *material = prism.tri1.material;
        return true;
    }
    Float32 t2 = INFINITY;
    Vec3 n2;
    Vec2 uv2;
    if(geomTriangleIntersect(rayOrigin, rayDir, prism.tri2, &t2, &n2, &uv2))
    {
        if(t2 < t1)
        {
            *t = t2;
            *normal = n2;
            *material = prism.tri2.material;
            return true;
        }
    }
    return false;
}

Void geomDrawDebugBounds(Tigr* buffer, Vec3 lightPos, Float32 radius, Camera camera, Float32 renderScale)
{
    #if DEBUG_LIGHT_BOUNDS
    Vec3 right = vec3Norm(vec3Cross(camera.dir, camera.up));
    Float32 screenRadius = radius * 50.0f; // scale for visibility
    for(Int32 angle = 0; angle < 360; angle += 10)
    {
        Float32 rad = (Float32)angle * PI / 180.0f;
        Vec3 circlePoint = vec3Add(lightPos, vec3Add(vec3Mul(right, cosf(rad) * radius), vec3Mul(camera.up, sinf(rad) * radius)));
        Vec3 relPos = vec3Sub(circlePoint, camera.pos);
        Float32 depth = vec3Dot(relPos, camera.dir);
        if(depth > 0.1f)
        {
            Float32 x = vec3Dot(relPos, right) / depth;
            Float32 y = vec3Dot(relPos, camera.up) / depth;
            Int32 screenX = (Int32)(buffer->w * 0.5f + x * buffer->w * 0.5f / renderScale);
            Int32 screenY = (Int32)(buffer->h * 0.5f - y * buffer->h * 0.5f / renderScale);
            if(screenX >= 0 && screenX < buffer->w && screenY >= 0 && screenY < buffer->h)
            {
                buffer->pix[screenY * buffer->w + screenX] = tigrRGB(255, 255, 0);
            }
        }
    }
    #endif
    use(buffer);
    use(lightPos);
    use(radius);
    use(camera);
    use(renderScale);
}
