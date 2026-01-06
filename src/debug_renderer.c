#include "debug_renderer.h"
#include <float.h>

Void debugDrawModelBoxes(Tigr* buffer, Scene* scene)
{
    Camera cam = scene->camera;
    Vec3 right = vec3Norm(vec3Cross(cam.dir, cam.up));
    Vec3 up = vec3Norm(vec3Cross(right, cam.dir));
    Float32 aspect = (Float32)buffer->w / buffer->h;
    Float32 fovScale = tanf(cam.fov / 2.0f);

    for(Int32 i = 0; i < scene->modelCount; ++i)
    {
        ObjModel model = scene->models[i];
        Vec3 corners[8] = {
            {model.min.x, model.min.y, model.min.z},
            {model.max.x, model.min.y, model.min.z},
            {model.min.x, model.max.y, model.min.z},
            {model.max.x, model.max.y, model.min.z},
            {model.min.x, model.min.y, model.max.z},
            {model.max.x, model.min.y, model.max.z},
            {model.min.x, model.max.y, model.max.z},
            {model.max.x, model.max.y, model.max.z}
        };

        Float32 minX = FLT_MAX, minY = FLT_MAX;
        Float32 maxX = -FLT_MAX, maxY = -FLT_MAX;
        Bool anyVisible = false;

        for(Int32 j = 0; j < 8; ++j)
        {
            Vec3 relPos = vec3Sub(corners[j], cam.pos);
            Float32 depth = vec3Dot(relPos, cam.dir);

            if(depth > 0.1f)
            {
                Float32 x = vec3Dot(relPos, right) / (depth * fovScale * aspect);
                Float32 y = vec3Dot(relPos, up) / (depth * fovScale);

                Float32 screenX = buffer->w * 0.5f + x * buffer->w * 0.5f;
                Float32 screenY = buffer->h * 0.5f - y * buffer->h * 0.5f;

                if(screenX < minX) minX = screenX;
                if(screenY < minY) minY = screenY;
                if(screenX > maxX) maxX = screenX;
                if(screenY > maxY) maxY = screenY;
                anyVisible = true;
            }
        }

        if(anyVisible)
        {
            tigrRect(buffer, (Int32)minX, (Int32)minY, (Int32)(maxX - minX), (Int32)(maxY - minY), tigrRGB(255, 0, 255));
        }
    }
}

Void debugDrawLabels(Tigr* buffer, Scene* scene)
{
    Camera cam = scene->camera;
    Vec3 right = vec3Norm(vec3Cross(cam.dir, cam.up));
    Vec3 up = vec3Norm(vec3Cross(right, cam.dir));
    Float32 fovScale = tanf(cam.fov / 2.0f);
    Float32 aspect = (Float32)buffer->w / buffer->h;

    for(Int32 i = 0; i < scene->labelCount; ++i)
    {
        DebugLabel l = scene->labels[i];
        Vec3 relPos = vec3Sub(l.pos, cam.pos);
        Float32 d = vec3Dot(relPos, cam.dir);
        if(d < 0.1f || d > 100.0f) continue;

        Float32 x = vec3Dot(relPos, right) / (d * fovScale * aspect);
        Float32 y = vec3Dot(relPos, up) / (d * fovScale);
        Int32 sx = (Int32)(buffer->w * 0.5f + x * buffer->w * 0.5f);
        Int32 sy = (Int32)(buffer->h * 0.5f - y * buffer->h * 0.5f);

        if(sx > 0 && sx < buffer->w && sy > 0 && sy < buffer->h)
        {
            Int32 tw = tigrTextWidth(tfont, l.text);
            Int32 th = tigrTextHeight(tfont, l.text);
            tigrFillRect(buffer, sx - tw/2 - 4, sy - th/2 - 4, tw + 8, th + 8, tigrRGBA(0, 0, 0, 150));
            tigrPrint(buffer, tfont, sx - tw/2, sy - th/2, l.color, l.text);
        }
    }
}
