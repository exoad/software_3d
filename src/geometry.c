#include "geometry.h"
#include "tigr.h"
#include <stdlib.h>

Bool geomSphereIntersect(Vec3 rayOrigin, Vec3 rayDir, GeomSphere sphere, Float32* t, Vec3* normal)
{
    Vec3 oc = vec3Sub(rayOrigin, sphere.center);
    Float32 a = vec3Dot(rayDir, rayDir);
    Float32 b = 2.0f * vec3Dot(oc, rayDir);
    Float32 c = vec3Dot(oc, oc) - sphere.radius * sphere.radius;
    Float32 discriminant = b * b - 4.0f * a * c;

    if(discriminant < 0.0f)
    {
        return false;
    }

    Float32 sqrtDisc = sqrtf(discriminant);
    Float32 t1 = (-b - sqrtDisc) / (2.0f * a);
    Float32 t2 = (-b + sqrtDisc) / (2.0f * a);

    if(t1 > 1e-6f)
    {
        *t = t1;
        Vec3 hitPoint = vec3Add(rayOrigin, vec3Mul(rayDir, t1));
        *normal = vec3Norm(vec3Sub(hitPoint, sphere.center));
        return true;
    }
    else if(t2 > 1e-6f)
    {
        *t = t2;
        Vec3 hitPoint = vec3Add(rayOrigin, vec3Mul(rayDir, t2));
        *normal = vec3Norm(vec3Sub(hitPoint, sphere.center));
        return true;
    }

    return false;
}

Bool geomTriangleIntersect(Vec3 rayOrigin, Vec3 rayDir, GeomTriangle tri, Float32* t, Vec3* normal)
{
    Vec3 edge1 = vec3Sub(tri.p1, tri.p0);
    Vec3 edge2 = vec3Sub(tri.p2, tri.p0);
    Vec3 h = vec3Cross(rayDir, edge2);
    Float32 a = vec3Dot(edge1, h);
    if(fabsf(a) < 1e-6f)
    {
        return false; // ray paralle
    }
    Float32 f = 1.0f / a;
    Vec3 s = vec3Sub(rayOrigin, tri.p0);
    Float32 u = f * vec3Dot(s, h);
    if(u < 0.0f || u > 1.0f)
    {
        return false;
    }
    Vec3 q = vec3Cross(s, edge1);
    Float32 v = f * vec3Dot(rayDir, q);
    if(v < 0.0f || u + v > 1.0f)
    {
        return false;
    }
    Float32 tval = f * vec3Dot(edge2, q);
    if(tval > 1e-6f)
    {
        *t = tval;
        *normal = vec3Norm(vec3Cross(edge1, edge2));
        return true;
    }
    return false;
}

Bool geomPlaneIntersect(Vec3 rayOrigin, Vec3 rayDir, GeomPlane plane, Float32* t, Vec3* normal)
{
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

Bool geomBoxIntersect(Vec3 rayOrigin, Vec3 rayDir, GeomBox box, Float32* t, Vec3* normal)
{
    Float32 eps = 1e-6f;
    Float32 txMin = (rayDir.x > eps) ? ((box.center.x - box.extents.x - rayOrigin.x) / rayDir.x) : ((box.center.x + box.extents.x - rayOrigin.x) / rayDir.x);
    Float32 txMax = (rayDir.x > eps) ? ((box.center.x + box.extents.x - rayOrigin.x) / rayDir.x) : ((box.center.x - box.extents.x - rayOrigin.x) / rayDir.x);
    Float32 tyMin = (rayDir.y > eps) ? ((box.center.y - box.extents.y - rayOrigin.y) / rayDir.y) : ((box.center.y + box.extents.y - rayOrigin.y) / rayDir.y);
    Float32 tyMax = (rayDir.y > eps) ? ((box.center.y + box.extents.y - rayOrigin.y) / rayDir.y) : ((box.center.y - box.extents.y - rayOrigin.y) / rayDir.y);
    Float32 tzMin = (rayDir.z > eps) ? ((box.center.z - box.extents.z - rayOrigin.z) / rayDir.z) : ((box.center.z + box.extents.z - rayOrigin.z) / rayDir.z);
    Float32 tzMax = (rayDir.z > eps) ? ((box.center.z + box.extents.z - rayOrigin.z) / rayDir.z) : ((box.center.z - box.extents.z - rayOrigin.z) / rayDir.z);
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

Bool geomTriangularPrismIntersect(Vec3 rayOrigin, Vec3 rayDir, GeomTriangularPrism prism, Float32* t, Vec3* normal, Material* material)
{
    Float32 t1 = INFINITY;
    Vec3 n1;
    if(geomTriangleIntersect(rayOrigin, rayDir, prism.tri1, &t1, &n1))
    {
        *t = t1;
        *normal = n1;
        *material = prism.tri1.material;
        return true;
    }
    Float32 t2 = INFINITY;
    Vec3 n2;
    if(geomTriangleIntersect(rayOrigin, rayDir, prism.tri2, &t2, &n2))
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
    // Draw a sphere representing the light's influence radius
    Vec3 right = vec3Norm(vec3Cross(camera.dir, camera.up));
    Float32 screenRadius = radius * 50.0f; // Scale for visibility

    // Draw circle on screen by projecting sphere edges
    for(Int32 angle = 0; angle < 360; angle += 10)
    {
        Float32 rad = (Float32)angle * PI / 180.0f;
        Vec3 circlePoint = vec3Add(lightPos, vec3Add(vec3Mul(right, cosf(rad) * radius), vec3Mul(camera.up, sinf(rad) * radius)));

        // Project to screen space
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
