#include "frustum.h"

Void frustumExtract(Frustum* frustum, Camera* camera, Float32 nearPlane, Float32 farPlane, Float32 aspect) {
    Vec3 right = vec3Norm(vec3Cross(camera->dir, camera->up));
    Float32 tanHalfFov = tanf(camera->fov / 2.0f);

    // near
    frustum->planes[4].normal = camera->dir;
    frustum->planes[4].distance = vec3Dot(camera->dir, vec3Add(camera->pos, vec3Mul(camera->dir, nearPlane)));

    // far
    frustum->planes[5].normal = vec3Mul(camera->dir, -1.0f);
    frustum->planes[5].distance = vec3Dot(frustum->planes[5].normal, vec3Add(camera->pos, vec3Mul(camera->dir, farPlane)));

    // left
    Vec3 leftDir = vec3Add(vec3Mul(camera->dir, nearPlane), vec3Mul(right, -nearPlane * tanHalfFov * aspect));
    Vec3 leftNormal = vec3Norm(vec3Cross(leftDir, camera->up));
    frustum->planes[0].normal = leftNormal;
    frustum->planes[0].distance = vec3Dot(leftNormal, camera->pos);

    // right
    Vec3 rightDir = vec3Add(vec3Mul(camera->dir, nearPlane), vec3Mul(right, nearPlane * tanHalfFov * aspect));
    Vec3 rightNormal = vec3Norm(vec3Cross(camera->up, rightDir));
    frustum->planes[1].normal = rightNormal;
    frustum->planes[1].distance = vec3Dot(rightNormal, camera->pos);

    // bottom
    Vec3 bottomDir = vec3Add(vec3Mul(camera->dir, nearPlane), vec3Mul(camera->up, -nearPlane * tanHalfFov));
    Vec3 bottomNormal = vec3Norm(vec3Cross(bottomDir, right));
    frustum->planes[2].normal = bottomNormal;
    frustum->planes[2].distance = vec3Dot(bottomNormal, camera->pos);

    // top
    Vec3 topDir = vec3Add(vec3Mul(camera->dir, nearPlane), vec3Mul(camera->up, nearPlane * tanHalfFov));
    Vec3 topNormal = vec3Norm(vec3Cross(right, topDir));
    frustum->planes[3].normal = topNormal;
    frustum->planes[3].distance = vec3Dot(topNormal, camera->pos);
}

Bool frustumSphereIntersect(const Frustum* frustum, Vec3 center, Float32 radius) {
    for(Int32 i = 0; i < 6; ++i)
    {
        Float32 dist = vec3Dot(frustum->planes[i].normal, center) - frustum->planes[i].distance;
        if(dist + radius < 0.0f)
        {
            return false; // outside
        }
    }
    return true; // inside or intersecting
}