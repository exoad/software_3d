#include "public.h"
#include "tigr.h"
#include "input.h"

#define RENDER_SCALE 0.08f

static Camera cam = {
    .pos = {0, 0, -5},
    .dir = {0, 0, 1},
    .up = {0, 1, 0},
    .fov = 60.0f * PI / 180.0f
};
static Int32 frameCount = 0;
static Float32 timeAccumulator = 0.0f;
static Float32 fps = 60.0f;

Int32 main(Int32 argc, CharSeq argv[])
{
    use(argc);
    use(argv);
    Tigr* screen = tigrWindow(1920, 1080, "Software Raytracer", 0);
    Int32 renderW = (Int32) (screen->w * RENDER_SCALE);
    Int32 renderH = (Int32) (screen->h * RENDER_SCALE);
    Tigr* renderBuffer = tigrBitmap(renderW, renderH);
    Scene* scene = createScene(cam, (Vec3) COLOR_BLACK);
    Sphere sphere = {
        .center = VEC3_ZERO,
        .radius = 1.0f
    };
    sceneAddSphere(scene, sphere);
    Light light = {
        .pos = {2, 2, -3},
        .color = {0.75, 0.75, 0.75}
    };
    sceneAddLight(scene, light);
    Float32 lastTime = tigrTime();
    while(!tigrClosed(screen))
    {
        Float32 currentTime = tigrTime();
        Float32 deltaTime = currentTime - lastTime;
        if(deltaTime < 0.0f || deltaTime > 0.1f)
        {
            deltaTime = 1.0f / 60.0f;
        }
        lastTime = currentTime;
        handleInput(scene, screen, deltaTime);
        sceneRender(scene, renderBuffer);
        for(Int32 y = 0; y < screen->h; ++y)
        {
            for(Int32 x = 0; x < screen->w; ++x)
            {
                Int32 srcX = (Int32) (x * RENDER_SCALE);
                Int32 srcY = (Int32) (y * RENDER_SCALE);
                screen->pix[y * screen->w + x] = renderBuffer->pix[srcY * renderBuffer->w + srcX];
            }
        }
        timeAccumulator += deltaTime;
        frameCount++;
        if(timeAccumulator >= 1.0f)
        {
            fps = (Float32)frameCount / timeAccumulator;
            frameCount = 0;
            timeAccumulator = 0.0f;
        }
        tigrPrint(screen, tfont, 10, 10, tigrRGB(0xff, 0, 0), "FPS: %.1f", fps);
        tigrUpdate(screen);
    }
    destroyScene(scene);
    tigrFree(renderBuffer);
    tigrFree(screen);
    return 0;
}