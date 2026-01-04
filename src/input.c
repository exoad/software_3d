#include "input.h"

#define MOVE_SPEED 0.1f
#define ROT_SPEED 0.05f

Void handleInput(Scene* scene, Tigr* screen, Float32 deltaTime) {
    Vec3 forward = vec3Norm(scene->camera.dir);
    Vec3 right = vec3Norm(vec3Cross(scene->camera.dir, scene->camera.up));
    Float32 moveSpeed = MOVE_SPEED * deltaTime;
    Float32 rotSpeed = ROT_SPEED * deltaTime;
    if(tigrKeyHeld(screen, 'W'))
    {
        scene->camera.pos = vec3Add(scene->camera.pos, vec3Mul(forward, moveSpeed));
    }
    if(tigrKeyHeld(screen, 'S'))
    {
        scene->camera.pos = vec3Sub(scene->camera.pos, vec3Mul(forward, moveSpeed));
    }
    if(tigrKeyHeld(screen, 'A'))
    {
        scene->camera.pos = vec3Sub(scene->camera.pos, vec3Mul(right, moveSpeed));
    }
    if(tigrKeyHeld(screen, 'D'))
    {
        scene->camera.pos = vec3Add(scene->camera.pos, vec3Mul(right, moveSpeed));
    }

    if(tigrKeyHeld(screen, TK_LEFT))
    {
        scene->camera.dir = vec3RotateY(scene->camera.dir, rotSpeed);
    }
    if(tigrKeyHeld(screen, TK_RIGHT))
    {
        scene->camera.dir = vec3RotateY(scene->camera.dir, -rotSpeed);
    }
    if(tigrKeyHeld(screen, TK_UP))
    {
        scene->camera.dir = vec3RotateX(scene->camera.dir, rotSpeed);
        scene->camera.up = vec3RotateX(scene->camera.up, rotSpeed);
    }
    if(tigrKeyHeld(screen, TK_DOWN))
    {
        scene->camera.dir = vec3RotateX(scene->camera.dir, -rotSpeed);
        scene->camera.up = vec3RotateX(scene->camera.up, -rotSpeed);
    }
}