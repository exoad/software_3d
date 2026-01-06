#include "input.h"
#include "logger.h"

#define MOVE_SPEED 6.0f
#define SPRINT_MULT 4.0f
#define ROT_SPEED 2.5f
#define MOUSE_SENS 0.003f

Void initInputState(InputState* state)
{
    state->lastMouseX = 0;
    state->lastMouseY = 0;
    state->mouseCaptured = false;
}

Void handleInput(CameraController* cam, Tigr* screen, Float32 deltaTime, InputState* inputState)
{
    Vec3 forward = cameraGetForward(cam);
    Vec3 right = cameraGetRight(cam);
    Float32 sprintMult = tigrKeyHeld(screen, TK_SHIFT) ? SPRINT_MULT : 1.0f;
    Float32 moveSpeed = MOVE_SPEED * deltaTime * sprintMult;
    Float32 rotSpeed = ROT_SPEED * deltaTime;
    Float32 fwd = 0.0f, rgt = 0.0f, upd = 0.0f;
    if(tigrKeyHeld(screen, 'W'))
    {
        fwd += moveSpeed;
    }
    if(tigrKeyHeld(screen, 'S'))
    {
        fwd -= moveSpeed;
    }
    if(tigrKeyHeld(screen, 'A'))
    {
        rgt -= moveSpeed;
    }
    if(tigrKeyHeld(screen, 'D'))
    {
        rgt += moveSpeed;
    }
    if(tigrKeyHeld(screen, 'Q'))
    {
        upd -= moveSpeed;
    }
    if(tigrKeyHeld(screen, 'E'))
    {
        upd += moveSpeed;
    }
    cameraMove(cam, forward, right, fwd, rgt, upd);
    Float32 pitchDelta = 0.0f, yawDelta = 0.0f;
    if(tigrKeyHeld(screen, TK_LEFT))
    {
        yawDelta += rotSpeed;
    }
    if(tigrKeyHeld(screen, TK_RIGHT))
    {
        yawDelta -= rotSpeed;
    }
    if(tigrKeyHeld(screen, TK_UP))
    {
        pitchDelta += rotSpeed;
    }
    if(tigrKeyHeld(screen, TK_DOWN))
    {
        pitchDelta -= rotSpeed;
    }
    if(pitchDelta != 0.0f || yawDelta != 0.0f)
    {
        cameraRotate(cam, pitchDelta, yawDelta);
    }
    Int32 mx, my, buttons;
    tigrMouse(screen, &mx, &my, &buttons);
    if(tigrKeyDown(screen, TK_SPACE))
    {
        inputState->mouseCaptured = !inputState->mouseCaptured;
        LOG_I("Mouse capture %s.", inputState->mouseCaptured ? "enabled" : "disabled");
    }
    if(inputState->mouseCaptured && (buttons & 1))
    {
        Int32 dx = mx - inputState->lastMouseX;
        Int32 dy = my - inputState->lastMouseY;
        if(dx != 0 || dy != 0)
        {
            cameraRotate(cam, -dy * MOUSE_SENS, -dx * MOUSE_SENS);
        }
    }
    inputState->lastMouseX = mx;
    inputState->lastMouseY = my;
}

Void inputUpdate(RenderContext* ctx, Tigr* screen, Float32 deltaTime, InputState* state)
{
    if(tigrKeyDown(screen, TK_TAB))
    {
        renderContextNextMode(ctx);
    }
    handleInput(renderContextGetCamera(ctx), screen, deltaTime, state);
}