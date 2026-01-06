#include "rendercontext.h"
#include "frustum.h"
#include "logger.h"
#include <stdlib.h>

static CharSeq renderModeNames[] = {
    "Rasterizer",
    "SDF Raymarcher",
    "Grid",
    "Normals",
    "Depth",
    "UV Debug",
    "Ambient Occlusion",
    "Edge Detect",
    "Fog",
    "Cel Shading"
};

RenderContext* createRenderContext(Int32 width, Int32 height, Float32 renderScale)
{
    LOG_I("Creating render context %dx%d (Scale: %.2f)", width, height, renderScale);
    RenderContext* ctx = (RenderContext*) malloc(sizeOf(RenderContext));
    Int32 renderW = (Int32) (width * renderScale);
    Int32 renderH = (Int32) (height * renderScale);
    ctx->renderBuffer = tigrBitmap(renderW, renderH);
    ctx->renderScale = renderScale;
    ctx->camera = (CameraController*) malloc(sizeOf(CameraController));
    cameraInit(ctx->camera, (Vec3){0.0f, 2.0f, -20.0f}, (Vec3){0.0f, 0.0f, 1.0f}, (Vec3){0.0f, 1.0f, 0.0f}, 60.0f * PI / 180.0f);
    Camera sceneCamera = {ctx->camera->pos, ctx->camera->dir, ctx->camera->up, ctx->camera->fov};
    ctx->scene = createScene(sceneCamera, (Vec3)COLOR_BLACK);
    ctx->passes[RENDER_MODE_SDF] = createShaderPass(builtinSdfRaymarchShader);
    ctx->passes[RENDER_MODE_GRID] = createShaderPass(builtinGridShader);
    ctx->passes[RENDER_MODE_NORMALS] = createShaderPass(builtinNormalVisShader);
    ctx->passes[RENDER_MODE_DEPTH] = createShaderPass(builtinDepthShader);
    ctx->passes[RENDER_MODE_UV] = createShaderPass(builtinUvShader);
    ctx->passes[RENDER_MODE_AO] = createShaderPass(builtinAmbientOcclusionShader);
    ctx->passes[RENDER_MODE_EDGE] = createShaderPass(builtinEdgeDetectShader);
    ctx->passes[RENDER_MODE_FOG] = createShaderPass(builtinFogShader);
    ctx->passes[RENDER_MODE_CEL] = createShaderPass(builtinCelShadingShader);
    ctx->passes[RENDER_MODE_RASTER] = null;
    ctx->currentMode = RENDER_MODE_RASTER;
    ctx->visibleObjects = 0;
    ctx->totalObjects = 0;
    return ctx;
}

Void destroyRenderContext(RenderContext* ctx)
{
    LOG_I("Destroying render context.");
    if(!ctx)
    {
        return;
    }
    for(Int32 i = 0; i < RENDER_MODE_COUNT; ++i)
    {
        if(ctx->passes[i])
        {
            destroyShaderPass(ctx->passes[i]);
        }
    }
    destroyScene(ctx->scene);
    tigrFree(ctx->renderBuffer);
    free(ctx->camera);
    free(ctx);
}

Void renderContextSetMode(RenderContext* ctx, RenderMode mode)
{
    if(mode >= 0 && mode < RENDER_MODE_COUNT)
    {
        LOG_I("Render mode changed to: %s", renderModeNames[mode]);
        ctx->currentMode = mode;
    }
}

Void renderContextNextMode(RenderContext* ctx)
{
    ctx->currentMode = (ctx->currentMode + 1) % RENDER_MODE_COUNT;
}

static Void updateCameraInPasses(RenderContext* ctx)
{
    Camera cam = {ctx->camera->pos, ctx->camera->dir, ctx->camera->up, ctx->camera->fov};
    for(Int32 i = 0; i < RENDER_MODE_COUNT; ++i)
    {
        if(ctx->passes[i])
        {
            shaderPassSetCamera(ctx->passes[i], cam.pos, cam.dir, cam.up, cam.fov);
        }
    }
}

static Int32 countVisibleObjects(RenderContext* ctx, Frustum* frustum)
{
    Int32 visible = 0;
    Int32 total = ctx->scene->sphereCount + ctx->scene->boxCount + ctx->scene->triangleCount;
    for(Int32 i = 0; i < ctx->scene->sphereCount; ++i)
    {
        if(frustumSphereIntersect(frustum, ctx->scene->spheres[i].center, ctx->scene->spheres[i].radius))
        {
            visible++;
        }
    }
    for(Int32 i = 0; i < ctx->scene->boxCount; ++i)
    {
        Float32 radius = vec3Len(ctx->scene->boxes[i].extents);
        if(frustumSphereIntersect(frustum, ctx->scene->boxes[i].center, radius))
        {
            visible++;
        }
    }
    for(Int32 i = 0; i < ctx->scene->triangleCount; ++i)
    {
        Vec3 center = vec3Mul(vec3Add(vec3Add(ctx->scene->triangles[i].p0, ctx->scene->triangles[i].p1), ctx->scene->triangles[i].p2), 1.0f / 3.0f);
        if(frustumSphereIntersect(frustum, center, 2.0f))
            visible++;
    }
    ctx->totalObjects = total;
    ctx->visibleObjects = visible;
    return visible;
}

Void renderContextUpdate(RenderContext* ctx, Float32 deltaTime)
{
    ctx->scene->camera.pos = ctx->camera->pos;
    ctx->scene->camera.dir = ctx->camera->dir;
    ctx->scene->camera.up = ctx->camera->up;
    ctx->scene->camera.fov = ctx->camera->fov;
    updateCameraInPasses(ctx);
    Frustum frustum;
    Float32 aspect = (Float32)ctx->renderBuffer->w / ctx->renderBuffer->h;
    frustumExtract(&frustum, &ctx->scene->camera, 0.1f, 100.0f, aspect);
    countVisibleObjects(ctx, &frustum);
    use(deltaTime);
}

Void renderContextRender(RenderContext* ctx, Tigr* screen)
{
    if(ctx->currentMode == RENDER_MODE_RASTER)
    {
        sceneRender(ctx->scene, ctx->renderBuffer);
    }
    else if(ctx->passes[ctx->currentMode])
    {
        shaderPassExecute(ctx->passes[ctx->currentMode], ctx->renderBuffer, 0.016f);
    }
    for(Int32 y = 0; y < screen->h; ++y)
    {
        for(Int32 x = 0; x < screen->w; ++x)
        {
            Int32 srcX = (Int32)(x * ctx->renderScale);
            Int32 srcY = (Int32)(y * ctx->renderScale);
            screen->pix[y * screen->w + x] = ctx->renderBuffer->pix[srcY * ctx->renderBuffer->w + srcX];
        }
    }
}

Scene* renderContextGetScene(RenderContext* ctx)
{
    return ctx->scene;
}

CameraController* renderContextGetCamera(RenderContext* ctx)
{
    return ctx->camera;
}

CharSeq renderContextGetModeName(RenderContext* ctx)
{
    return renderModeNames[ctx->currentMode];
}

Int32 renderContextGetCulledCount(RenderContext* ctx)
{
    return ctx->totalObjects - ctx->visibleObjects;
}

Int32 renderContextGetTotalCount(RenderContext* ctx)
{
    return ctx->totalObjects;
}
