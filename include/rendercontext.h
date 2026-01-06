#ifndef RENDERCONTEXT_H
#define RENDERCONTEXT_H

#include "shared.h"
#include "scene.h"
#include "camera.h"
#include "shaderpass.h"
#include "tigr.h"

typedef enum {
    RENDER_MODE_RASTER,
    RENDER_MODE_SDF,
    RENDER_MODE_GRID,
    RENDER_MODE_NORMALS,
    RENDER_MODE_DEPTH,
    RENDER_MODE_UV,
    RENDER_MODE_AO,
    RENDER_MODE_EDGE,
    RENDER_MODE_FOG,
    RENDER_MODE_CEL,
    RENDER_MODE_COUNT
} RenderMode;

typedef struct {
    Scene* scene;
    CameraController* camera;
    Tigr* renderBuffer;
    ShaderPass* passes[RENDER_MODE_COUNT];
    RenderMode currentMode;
    Float32 renderScale;
    Int32 visibleObjects;
    Int32 totalObjects;
} RenderContext;

RenderContext* createRenderContext(Int32 width, Int32 height, Float32 renderScale);
Void destroyRenderContext(RenderContext* ctx);
Void renderContextSetMode(RenderContext* ctx, RenderMode mode);
Void renderContextNextMode(RenderContext* ctx);
Void renderContextUpdate(RenderContext* ctx, Float32 deltaTime);
Void renderContextRender(RenderContext* ctx, Tigr* screen);
Scene* renderContextGetScene(RenderContext* ctx);
CameraController* renderContextGetCamera(RenderContext* ctx);
CharSeq renderContextGetModeName(RenderContext* ctx);
Int32 renderContextGetCulledCount(RenderContext* ctx);
Int32 renderContextGetTotalCount(RenderContext* ctx);

#endif
