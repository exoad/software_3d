#include "obj_loader.h"
#include "scene.h"
#include "geometry.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "logger.h"

#define MAX_LINE 512

Void loadObj(CharSeq path, Scene* scene)
{
    LOG_I("Loading OBJ model: %s", path);
    CFile* f = fopen(path, "r");
    if(!f)
    {
        LOG_E("Failed to open OBJ file: %s", path);
        return;
    }
    Vec3* verts = null;
    Vec2* uvs = null;
    Size count = 0, cap = 0;
    Size uvCount = 0, uvCap = 0;
    Int8 line[MAX_LINE];
    ObjModel model = {
        .min = {INFINITY, INFINITY, INFINITY},
        .max = {-INFINITY, -INFINITY, -INFINITY}
    };
    while(fgets(line, sizeOf(line), f))
    {
        if(line[0] == 'v' && line[1] == ' ')
        {
            Float32 x, y, z;
            if(sscanf(line + 2, "%f %f %f", &x, &y, &z) == 3)
            {
                if(count == cap)
                {
                    cap = cap ? cap * 2 : 64;
                    verts = (Vec3*)realloc(verts, sizeOf(Vec3) * cap);
                }
                Vec3 v = {x, y, z};
                verts[count++] = v;
                if(v.x < model.min.x) model.min.x = v.x;
                if(v.y < model.min.y) model.min.y = v.y;
                if(v.z < model.min.z) model.min.z = v.z;
                if(v.x > model.max.x) model.max.x = v.x;
                if(v.y > model.max.y) model.max.y = v.y;
                if(v.z > model.max.z) model.max.z = v.z;
            }
        }
        else if(line[0] == 'v' && line[1] == 't')
        {
            Float32 u, v;
            if(sscanf(line + 3, "%f %f", &u, &v) >= 2)
            {
                if(uvCount == uvCap)
                {
                    uvCap = uvCap ? uvCap * 2 : 64;
                    uvs = (Vec2*)realloc(uvs, sizeOf(Vec2) * uvCap);
                }
                uvs[uvCount++] = (Vec2){u, v};
            }
        }
        else if(line[0] == 'f' && line[1] == ' ')
        {
            Int32 v0, t0, n0, v1, t1, n1, v2, t2, n2;
            Int32 res = sscanf(line + 2, "%d/%d/%d %d/%d/%d %d/%d/%d", &v0, &t0, &n0, &v1, &t1, &n1, &v2, &t2, &n2);
            if(res == 9)
            {
                v0--;
                v1--;
                v2--;
                t0--;
                t1--;
                t2--;
                GeomTriangle tri = {verts[v0], verts[v1], verts[v2], uvs[t0], uvs[t1], uvs[t2], {{0.8f, 0.8f, 0.8f}, 32.0f, 0.8f, 0.0f, 0.0f, 1.0f, TEXTURE_NONE, null}};
                sceneAddTriangle(scene, tri);
            }
            else
            {
                res = sscanf(line + 2, "%d/%d %d/%d %d/%d", &v0, &t0, &v1, &t1, &v2, &t2);
                if(res == 6)
                {
                    v0--;
                    v1--;
                    v2--;
                    t0--;
                    t1--;
                    t2--;
                    GeomTriangle tri = {.p0 = verts[v0], .p1 = verts[v1], .p2 = verts[v2], .uv0 = uvs[t0], .uv1 = uvs[t1], .uv2 = uvs[t2], .material = {{0.8f, 0.8f, 0.8f}, 32.0f, 0.8f, 0.0f, 0.0f, 1.0f, TEXTURE_NONE, null}};
                    sceneAddTriangle(scene, tri);
                }
                else if(sscanf(line + 2, "%d %d %d", &v0, &v1, &v2) == 3)
                {
                    v0--;
                    v1--;
                    v2--;
                    GeomTriangle tri = {.p0 = verts[v0], .p1 = verts[v1], .p2 = verts[v2], .uv0 = {0, 0}, .uv1 = {0, 0}, .uv2 = {0, 0}, .material = {{0.8f, 0.8f, 0.8f}, 32.0f, 0.8f, 0.0f, 0.0f, 1.0f, TEXTURE_NONE, null}};
                    sceneAddTriangle(scene, tri);
                }
            }
        }
    }
    LOG_I("Finished loading OBJ. Total Verts: %zu", count);
    if(count > 0)
    {
        sceneAddModel(scene, model);
    }
    if(verts)
    {
        free(verts);
    }
    if(uvs)
    {
        free(uvs);
    }
    fclose(f);
}
