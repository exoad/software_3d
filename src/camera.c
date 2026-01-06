#include "camera.h"
#include <math.h>

Void cameraInit(CameraController* cam, Vec3 pos, Vec3 dir, Vec3 up, Float32 fov)
{
    cam->pos = pos;
    cam->dir = vec3Norm(dir);
    cam->up = vec3Norm(up);
    cam->fov = fov;
    cam->pitch = asinf(cam->dir.y);
    cam->yaw = atan2f(cam->dir.x, cam->dir.z);
}

Void cameraUpdateFromPitchYaw(CameraController* cam)
{
    Float32 cosPitch = cosf(cam->pitch);
    cam->dir.x = cosPitch * sinf(cam->yaw);
    cam->dir.y = sinf(cam->pitch);
    cam->dir.z = cosPitch * cosf(cam->yaw);
    cam->dir = vec3Norm(cam->dir);
    cam->up = (Vec3){0, 1, 0};
}

Void cameraRotate(CameraController* cam, Float32 pitchDelta, Float32 yawDelta)
{
    cam->pitch += pitchDelta;
    cam->yaw += yawDelta;

    Float32 maxPitch = PI / 2.0f - 0.01f;
    if(cam->pitch > maxPitch)
    {
        cam->pitch = maxPitch;
    }
    if(cam->pitch < -maxPitch)
    {
        cam->pitch = -maxPitch;
    }

    cameraUpdateFromPitchYaw(cam);
}

Void cameraMove(CameraController* cam, Vec3 forward, Vec3 right, Float32 forwardAmount, Float32 rightAmount, Float32 upAmount)
{
    cam->pos = vec3Add(cam->pos, vec3Mul(forward, forwardAmount));
    cam->pos = vec3Add(cam->pos, vec3Mul(right, rightAmount));
    cam->pos.y += upAmount;
}

Vec3 cameraGetForward(CameraController* cam)
{
    return vec3Norm(cam->dir);
}

Vec3 cameraGetRight(CameraController* cam)
{
    return vec3Norm(vec3Cross(cam->dir, cam->up));
}
