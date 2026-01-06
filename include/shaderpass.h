#ifndef SHADERPASS_H
#define SHADERPASS_H

#include "shared.h"
#include "tigr.h"

typedef struct {
    Int32 x, y;
    Int32 w, h;
    Float32 u, v;
    Float32 time;
    Float32 deltaTime;
    Vec3 camPos;
    Vec3 camDir;
    Vec3 camUp;
    Float32 fov;
    Any userData;
} ShaderContext;

typedef Vec3 (*ShaderFn)(ShaderContext* ctx);

typedef struct {
    ShaderFn shader;
    Float32 time;
    Vec3 camPos;
    Vec3 camDir;
    Vec3 camUp;
    Float32 fov;
    Any userData;
} ShaderPass;

ShaderPass* createShaderPass(ShaderFn shader);
Void destroyShaderPass(ShaderPass* pass);
Void shaderPassSetCamera(ShaderPass* pass, Vec3 pos, Vec3 dir, Vec3 up, Float32 fov);
Void shaderPassExecute(ShaderPass* pass, Tigr* buffer, Float32 deltaTime);

simple Float32 sdfSphere(Vec3 p, Float32 r)
{
    return vec3Len(p) - r;
}

simple Float32 sdfBox(Vec3 p, Vec3 b)
{
    Vec3 q = {fabsf(p.x) - b.x, fabsf(p.y) - b.y, fabsf(p.z) - b.z};
    Vec3 maxQ = {fmaxf(q.x, 0.0f), fmaxf(q.y, 0.0f), fmaxf(q.z, 0.0f)};
    return vec3Len(maxQ) + fminf(fmaxf(q.x, fmaxf(q.y, q.z)), 0.0f);
}

simple Float32 sdfTorus(Vec3 p, Float32 R, Float32 r)
{
    Float32 qx = sqrtf(p.x * p.x + p.z * p.z) - R;
    Float32 qy = p.y;
    return sqrtf(qx * qx + qy * qy) - r;
}

simple Float32 sdfPlane(Vec3 p, Vec3 n, Float32 h)
{
    return vec3Dot(p, n) + h;
}

simple Float32 sdfCylinder(Vec3 p, Float32 h, Float32 r)
{
    Float32 dx = sqrtf(p.x * p.x + p.z * p.z) - r;
    Float32 dy = fabsf(p.y) - h;
    Float32 dxClamped = fmaxf(dx, 0.0f);
    Float32 dyClamped = fmaxf(dy, 0.0f);
    return fminf(fmaxf(dx, dy), 0.0f) + sqrtf(dxClamped * dxClamped + dyClamped * dyClamped);
}

simple Float32 opUnion(Float32 d1, Float32 d2)
{
    return fminf(d1, d2);
}

simple Float32 opSubtract(Float32 d1, Float32 d2)
{
    return fmaxf(-d1, d2);
}

simple Float32 opIntersect(Float32 d1, Float32 d2)
{
    return fmaxf(d1, d2);
}

simple Float32 opSmoothUnion(Float32 d1, Float32 d2, Float32 k)
{
    Float32 h = fmaxf(k - fabsf(d1 - d2), 0.0f) / k;
    return fminf(d1, d2) - h * h * k * 0.25f;
}

simple Vec3 opRepeat(Vec3 p, Vec3 c)
{
    return (Vec3){
        fmodf(p.x + c.x * 0.5f, c.x) - c.x * 0.5f,
        fmodf(p.y + c.y * 0.5f, c.y) - c.y * 0.5f,
        fmodf(p.z + c.z * 0.5f, c.z) - c.z * 0.5f
    };
}

simple Vec3 opTwist(Vec3 p, Float32 k)
{
    Float32 c = cosf(k * p.y);
    Float32 s = sinf(k * p.y);
    return (Vec3){p.x * c - p.z * s, p.y, p.x * s + p.z * c};
}

simple Vec3 sdfNormal(Vec3 p, Float32 (*sdf)(Vec3, Any), Any data)
{
    Float32 eps = 0.001f;
    Vec3 n = {
        sdf((Vec3){p.x + eps, p.y, p.z}, data) - sdf((Vec3){p.x - eps, p.y, p.z}, data),
        sdf((Vec3){p.x, p.y + eps, p.z}, data) - sdf((Vec3){p.x, p.y - eps, p.z}, data),
        sdf((Vec3){p.x, p.y, p.z + eps}, data) - sdf((Vec3){p.x, p.y, p.z - eps}, data)
    };
    return vec3Norm(n);
}

simple Float32 sdfCone(Vec3 p, Float32 angle, Float32 h)
{
    Vec2 c = {sinf(angle), cosf(angle)};
    Float32 q = sqrtf(p.x * p.x + p.z * p.z);
    return fmaxf(c.x * q + c.y * p.y, -h - p.y);
}

simple Float32 sdfCapsule(Vec3 p, Vec3 a, Vec3 b, Float32 r)
{
    Vec3 pa = vec3Sub(p, a);
    Vec3 ba = vec3Sub(b, a);
    Float32 h = fminf(fmaxf(vec3Dot(pa, ba) / vec3Dot(ba, ba), 0.0f), 1.0f);
    return vec3Len(vec3Sub(pa, vec3Mul(ba, h))) - r;
}

simple Float32 sdfOctahedron(Vec3 p, Float32 s)
{
    p = (Vec3){fabsf(p.x), fabsf(p.y), fabsf(p.z)};
    return (p.x + p.y + p.z - s) * 0.57735027f;
}

simple Float32 opRound(Float32 d, Float32 r)
{
    return d - r;
}

simple Float32 opOnion(Float32 d, Float32 thickness)
{
    return fabsf(d) - thickness;
}

simple Vec3 opBend(Vec3 p, Float32 k)
{
    Float32 c = cosf(k * p.x);
    Float32 s = sinf(k * p.x);
    return (Vec3){p.x, c * p.y - s * p.z, s * p.y + c * p.z};
}

typedef struct {
    Float32 dist;
    Int32 matId;
} SdfHit;

typedef SdfHit (*SdfSceneFn)(Vec3 p, Float32 time, Any userData);

typedef struct {
    SdfSceneFn sceneFn;
    Vec3 (*getMaterial)(Int32 matId, Vec3 p, Float32 time);
    Int32 maxSteps;
    Float32 maxDist;
    Float32 surfDist;
    Vec3 lightPos;
    Vec3 lightColor;
    Vec3 ambientColor;
    Float32 fogDensity;
    Vec3 fogColor;
    Vec3 skyColorTop;
    Vec3 skyColorBottom;
} SdfSceneDesc;

Vec3 builtinSdfRaymarchShader(ShaderContext* ctx);
Vec3 builtinGridShader(ShaderContext* ctx);
Vec3 builtinNormalVisShader(ShaderContext* ctx);
Vec3 builtinDepthShader(ShaderContext* ctx);
Vec3 builtinUvShader(ShaderContext* ctx);
Vec3 builtinAmbientOcclusionShader(ShaderContext* ctx);
Vec3 builtinEdgeDetectShader(ShaderContext* ctx);
Vec3 builtinFogShader(ShaderContext* ctx);
Vec3 builtinCelShadingShader(ShaderContext* ctx);

#endif
