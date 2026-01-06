#ifndef INPUT_H
#define INPUT_H

#include "camera.h"
#include "tigr.h"
#include "rendercontext.h"

typedef struct {
    Int32 lastMouseX;
    Int32 lastMouseY;
    Bool mouseCaptured;
} InputState;

Void initInputState(InputState* state);
Void handleInput(CameraController* cam, Tigr* screen, Float32 deltaTime, InputState* inputState);
Void inputUpdate(RenderContext* ctx, Tigr* screen, Float32 deltaTime, InputState* state);

#endif