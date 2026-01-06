#ifndef CAMERA_H
#define CAMERA_H

#include "shared.h"

typedef struct {
    Vec3 pos;
    Vec3 dir;
    Vec3 up;
    Float32 fov;
    Float32 pitch;
    Float32 yaw;
} CameraController;

Void cameraInit(CameraController* cam, Vec3 pos, Vec3 dir, Vec3 up, Float32 fov);
Void cameraUpdateFromPitchYaw(CameraController* cam);
Void cameraRotate(CameraController* cam, Float32 pitchDelta, Float32 yawDelta);
Void cameraMove(CameraController* cam, Vec3 forward, Vec3 right, Float32 forwardAmount, Float32 rightAmount, Float32 upAmount);
Vec3 cameraGetForward(CameraController* cam);
Vec3 cameraGetRight(CameraController* cam);

#endif
