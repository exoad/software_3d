#define DEBUG_LIGHT_BOUNDS 0

#include "public.h"
#include "tigr.h"
#include "input.h"

#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1200

#define RENDER_SCALE 0.4f

static Camera cam = {
    .pos = {0, 0, -5},
    .dir = {0, 0, 1},
    .up = {0, 1, 0},
    .fov = 60.0f * PI / 180.0f
};

static Int32 frameCount = 0;
static Float32 timeAccumulator = 0.0f;
static Float32 fps = 0.0f;

Int32 main(Int32 argc, CharSeq argv[])
{
    use(argc);
    use(argv);
    Tigr* screen = tigrWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Software Raytracer", 0);
    Int32 renderW = (Int32) (screen->w * RENDER_SCALE);
    Int32 renderH = (Int32) (screen->h * RENDER_SCALE);
    Tigr* renderBuffer = tigrBitmap(renderW, renderH);
    Scene* scene = createScene(cam, (Vec3) COLOR_BLACK);

    GeomSphere sphere = {
        .center = VEC3_ZERO,
        .radius = 1.0f,
        .material = {.color = {1.0f, 0.8f, 0.5f}, .shininess = 32.0f, .diffuse = 0.8f}
    };
    sceneAddSphere(scene, sphere);

    GeomBox box = {
        .center = {-3.0f, 0.0f, 2.0f},
        .extents = {0.5f, 0.5f, 0.5f},
        .material = {.color = {0.2f, 0.7f, 1.0f}, .shininess = 16.0f, .diffuse = 0.7f}
    };
    sceneAddBox(scene, box);

    GeomPlane groundPlane = {
        .normal = {0.0f, 1.0f, 0.0f},
        .distance = 2.0f,
        .material = {.color = {0.5f, 0.5f, 0.5f}, .shininess = 50.0f, .diffuse = 0.9f}
    };
    sceneAddPlane(scene, groundPlane);

    GeomTriangle triangle = {
        .p0 = {3.0f, 0.0f, 1.0f},
        .p1 = {4.0f, 0.0f, 1.0f},
        .p2 = {3.5f, 1.0f, 1.0f},
        .material = {.color = {1.0f, 0.2f, 0.2f}, .shininess = 24.0f, .diffuse = 0.75f}
    };
    sceneAddTriangle(scene, triangle);

    Light light = {
        .pos = {2, 2, -3},
        .color = {0.75, 0.75, 0.75}
    };
    sceneAddLight(scene, light);
    Float32 frameStartTime = tigrTime();
    Float32 lastFrameTime = frameStartTime;
    while(!tigrClosed(screen))
    {
        Float32 inputDeltaTime = frameStartTime - lastFrameTime;
        if(inputDeltaTime < 0.0f || inputDeltaTime > 1.0f)
        {
            inputDeltaTime = 1.0f / 60.0f;
        }
        else if(inputDeltaTime > 0.1f)
        {
            inputDeltaTime = 0.1f;
        }
        handleInput(scene, screen, inputDeltaTime);
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
        tigrPrint(screen, tfont, 10, 10, tigrRGB(0xff, 0, 0), "FPS: %.1f (%.2fms)", fps, fps > 0.0f ? (1000.0f / fps) : 0.0f);
        tigrUpdate(screen);
        Float32 nextFrameTime = tigrTime();
        Float32 actualDeltaTime = nextFrameTime - frameStartTime;
        frameStartTime = nextFrameTime;
        timeAccumulator += actualDeltaTime;
        frameCount++;
        if(timeAccumulator >= 1.0f)
        {
            fps = (Float32)frameCount / timeAccumulator;
            frameCount = 0;
            timeAccumulator = 0.0f;
        }
    }
    destroyScene(scene);
    tigrFree(renderBuffer);
    tigrFree(screen);
    return 0;
}