// #define LOGGER_DISABLE_GLOBAL
#include "public.h"
#include "tigr.h"
#include "rendercontext.h"
#include "input.h"
#include "obj_loader.h"
#include "debug_renderer.h"
#include "logger.h"

#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1200
#define RENDER_SCALE 0.2f

static Int32 screenshotCount = 0;

static Void setupDemoScene(Scene* scene)
{
    LOG_I("Setting up demo scene...");
    sceneAddDefaultGround(scene);

    for(Int32 i = 0; i < 5; ++i)
    {
        Float32 x = (Float32)i * 6.5f - 13.0f;
        sceneAddSphereSimple(scene, (Vec3){x, 0.0f, 6.0f}, 1.2f, (Vec3){0.2f + i * 0.15f, 0.8f - i * 0.1f, 0.5f});
        sceneAddBoxSimple(scene, (Vec3){x, 0.0f, 15.0f}, (Vec3){0.8f, 0.8f, 0.8f}, (Vec3){0.8f - i * 0.1f, 0.2f + i * 0.15f, 0.7f});
    }
    sceneAddDebugLabel(scene, (Vec3){-13.0f, 2.5f, 6.0f}, "Sphere Row", tigrRGB(200, 200, 255));
    sceneAddDebugLabel(scene, (Vec3){-13.0f, 2.5f, 15.0f}, "Box Row", tigrRGB(200, 255, 200));

    sceneAddLightSimple(scene, (Vec3){-8.0f, 6.0f, 20.0f}, (Vec3){1.0f, 1.0f, 1.0f}, 1.0f);
    sceneAddDebugLabel(scene, (Vec3){-8.0f, 6.5f, 20.0f}, "Point Light", tigrRGB(255, 255, 200));

    sceneAddSpotLight(scene, (Vec3){0.0f, 8.0f, 20.0f}, (Vec3){0.0f, -1.0f, -0.5f}, 15.0f, 25.0f, (Vec3){0.5f, 0.5f, 1.0f}, 2.0f);
    sceneAddDebugLabel(scene, (Vec3){0.0f, 8.5f, 20.0f}, "Spot Light", tigrRGB(200, 200, 255));

    sceneAddAreaLight(scene, (Vec3){8.0f, 7.0f, 20.0f}, 2.0f, (Vec3){1.0f, 0.5f, 0.5f}, 1.5f);
    sceneAddDebugLabel(scene, (Vec3){8.0f, 7.5f, 20.0f}, "Area Light", tigrRGB(255, 200, 200));

    Material glass = {(Vec3){1.0f, 1.0f, 1.0f}, 64.0f, 0.1f, 0.1f, 0.9f, 1.5f, TEXTURE_NONE, null};
    GeomSphere glassSphere = {.center = (Vec3){0.0f, 0.0f, -6.0f}, .radius = 1.5f, .material = glass};
    sceneAddSphere(scene, glassSphere);
    sceneAddDebugLabel(scene, (Vec3){0.0f, 2.5f, -6.0f}, "Glass (Refraction)", tigrRGB(150, 255, 255));

    Material noiseMat = {(Vec3){1.0f, 0.8f, 0.4f}, 32.0f, 0.8f, 0.1f, 0.0f, 1.0f, TEXTURE_NOISE, null};
    GeomSphere noiseSphere = {.center = (Vec3){-7.5f, 0.0f, -6.0f}, .radius = 1.5f, .material = noiseMat};
    sceneAddSphere(scene, noiseSphere);
    sceneAddDebugLabel(scene, (Vec3){-7.5f, 2.5f, -6.0f}, "Procedural Noise", tigrRGB(255, 200, 150));

    Tigr* tex = sceneCreateDebugTexture(scene, 128, 128);
    Material texMat = {(Vec3){1.0f, 1.0f, 1.0f}, 32.0f, 0.8f, 0.1f, 0.0f, 1.0f, TEXTURE_IMAGE, tex};
    GeomSphere texSphere = {.center = (Vec3){7.5f, 0.0f, -6.0f}, .radius = 1.5f, .material = texMat};
    sceneAddSphere(scene, texSphere);
    sceneAddDebugLabel(scene, (Vec3){7.5f, 2.5f, -6.0f}, "Debug Texture", tigrRGB(200, 255, 200));

    GeomTriangle triangles[] = {
        {.p0 = {-4.0f, -1.5f, 25.0f}, .p1 = {0.0f, -1.5f, 25.0f}, .p2 = {-2.0f, 2.0f, 26.0f}, .uv0 = {0,0}, .uv1 = {1,0}, .uv2 = {0.5f,1}, .material = {{0.8f, 0.2f, 0.2f}, 32.0f, 0.8f, 0.0f, 0.0f, 1.0f, TEXTURE_NONE, null}},
        {.p0 = {2.0f, -1.5f, 25.0f}, .p1 = {6.0f, -1.5f, 25.0f}, .p2 = {4.0f, 2.0f, 26.0f}, .uv0 = {0,0}, .uv1 = {1,0}, .uv2 = {0.5f,1}, .material = {{0.2f, 0.8f, 0.2f}, 32.0f, 0.8f, 0.0f, 0.0f, 1.0f, TEXTURE_NONE, null}},
    };
    for(Int32 i = 0; i < 2; ++i)
    {
        sceneAddTriangle(scene, triangles[i]);
    }
    sceneAddDebugLabel(scene, (Vec3){1.0f, 3.5f, 25.0f}, "Triangle Mesh", tigrRGB(255, 200, 255));
}

static Void invertPixel(Tigr* screen, Int32 x, Int32 y)
{
    if(x < 0 || x >= screen->w || y < 0 || y >= screen->h)
    {
        return;
    }
    TPixel p = screen->pix[y * screen->w + x];
    screen->pix[y * screen->w + x] = tigrRGB(255 - p.r, 255 - p.g, 255 - p.b);
}

static Void drawCrosshair(Tigr* screen)
{
    Int32 cx = screen->w / 2;
    Int32 cy = screen->h / 2;
    Int32 size = 6;
    Int32 gap = 2;
    Int32 thick = 1;
    for(Int32 i = gap; i < size; ++i)
    {
        for(Int32 t = -thick; t <= thick; ++t)
        {
            invertPixel(screen, cx - i, cy + t);
            invertPixel(screen, cx + i, cy + t);
            invertPixel(screen, cx + t, cy - i);
            invertPixel(screen, cx + t, cy + i);
        }
    }
    for(Int32 ty = -thick; ty <= thick; ++ty)
    {
        for(Int32 tx = -thick; tx <= thick; ++tx)
        {
            invertPixel(screen, cx + tx, cy + ty);
        }
    }
}

Int32 main(Int32 argc, CharSeq argv[])
{
    use(argc);
    use(argv);
    LOG_I("Initializing Tigr window %dx%d...", SCREEN_WIDTH, SCREEN_HEIGHT);
    Tigr* screen = tigrWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Software Raytracer", 0);
    LOG_I("Creating render context (scale: %.2f)...", RENDER_SCALE);
    RenderContext* ctx = createRenderContext(SCREEN_WIDTH, SCREEN_HEIGHT, RENDER_SCALE);
    LOG_I("Loading initial OBJ model...");
    loadObj("assets/cube.obj", renderContextGetScene(ctx));
    setupDemoScene(renderContextGetScene(ctx));
    LOG_I("Building BVH...");
    sceneBuildBVH(renderContextGetScene(ctx));
    LOG_I("Startup complete. Entering main loop.");
    InputState inputState;
    initInputState(&inputState);
    Int32 frameCount = 0;
    Float32 timeAccumulator = 0.0f;
    Float32 fps = 0.0f;
    Float32 lastFrameTime = tigrTime();
    while(!tigrClosed(screen))
    {
        Float32 currentTime = tigrTime();
        Float32 deltaTime = currentTime - lastFrameTime;
        lastFrameTime = currentTime;
        if(deltaTime < 0.0f || deltaTime > 1.0f)
        {
            deltaTime = 1.0f / 60.0f;
        }
        else if(deltaTime > 0.1f)
        {
            deltaTime = 0.1f;
        }
        inputUpdate(ctx, screen, deltaTime, &inputState);
        renderContextUpdate(ctx, deltaTime);
        renderContextRender(ctx, screen);
#if DEBUG_MODEL_BOXES
        debugDrawModelBoxes(screen, renderContextGetScene(ctx));
        debugDrawLabels(screen, renderContextGetScene(ctx));
#endif
        if(tigrKeyDown(screen, 'X'))
        {
            Int8 fileName[64];
            sprintf(fileName, "screenshot_%d.png", ++screenshotCount);
            LOG_I("Capturing screenshot to '%s'...", fileName);
            tigrSaveImage(fileName, screen);
        }
        tigrPrint(screen, tfont, 10, 10, tigrRGB(255, 200, 50), "Mode: %s [TAB]", renderContextGetModeName(ctx));
        tigrPrint(screen, tfont, 10, 30, tigrRGB(255, 100, 100), "FPS: %.1f (%.2fms)", fps, fps > 0.0f ? (1000.0f / fps) : 0.0f);
        tigrPrint(screen, tfont, 10, 50, tigrRGB(100, 255, 100), "Culled: %d/%d", renderContextGetCulledCount(ctx), renderContextGetTotalCount(ctx));
        tigrPrint(screen, tfont, 10, 70, tigrRGB(150, 150, 255), "WASD: Move  QE: Up/Down  Arrows: Look  Shift: Sprint  S: Screenshot");
        tigrPrint(screen, tfont, 10, 90, tigrRGB(200, 200, 150), "Mouse: %s [SPACE toggle, LMB drag]", inputState.mouseCaptured ? "ON" : "OFF");
        tigrPrint(screen, tfont, 10, 110, tigrRGB(255, 255, 255), "Press [X] to take screenshot");
        drawCrosshair(screen);
        tigrUpdate(screen);
        frameCount++;
        timeAccumulator += deltaTime;
        if(timeAccumulator >= 1.0f)
        {
            fps = (Float32) frameCount / timeAccumulator;
            frameCount = 0;
            timeAccumulator = 0.0f;
        }
    }
    LOG_I("Shutting down...");
    destroyRenderContext(ctx);
    tigrFree(screen);
    return 0;
}