#include "shaderpass.h"
#include <stdlib.h>

ShaderPass* createShaderPass(ShaderFn shader)
{
    ShaderPass* pass = (ShaderPass*) malloc(sizeOf(ShaderPass));
    if(!pass)
    {
        return null;
    }
    pass->shader = shader;
    pass->time = 0.0f;
    pass->camPos = VEC3_ZERO;
    pass->camDir = (Vec3){0, 0, 1};
    pass->camUp = (Vec3){0, 1, 0};
    pass->fov = 60.0f * PI / 180.0f;
    pass->userData = null;
    return pass;
}

Void destroyShaderPass(ShaderPass* pass)
{
    if(pass)
    {
        free(pass);
    }
}

Void shaderPassSetCamera(ShaderPass* pass, Vec3 pos, Vec3 dir, Vec3 up, Float32 fov)
{
    pass->camPos = pos;
    pass->camDir = dir;
    pass->camUp = up;
    pass->fov = fov;
}

Void shaderPassExecute(ShaderPass* pass, Tigr* buffer, Float32 deltaTime)
{
    pass->time += deltaTime;
    Float32 aspect = (Float32)buffer->w / buffer->h;
    #pragma omp parallel for collapse(2) schedule(dynamic)
    for(Int32 y = 0; y < buffer->h; ++y)
    {
        for(Int32 x = 0; x < buffer->w; ++x)
        {
            ShaderContext ctx = {
                .x = x,
                .y = y,
                .w = buffer->w,
                .h = buffer->h,
                .u = (2.0f * (x + 0.5f) / buffer->w - 1.0f) * aspect,
                .v = 1.0f - 2.0f * (y + 0.5f) / buffer->h,
                .time = pass->time,
                .deltaTime = deltaTime,
                .camPos = pass->camPos,
                .camDir = pass->camDir,
                .camUp = pass->camUp,
                .fov = pass->fov,
                .userData = pass->userData
            };
            Vec3 color = pass->shader(&ctx);
            color.x = fminf(fmaxf(color.x, 0.0f), 1.0f);
            color.y = fminf(fmaxf(color.y, 0.0f), 1.0f);
            color.z = fminf(fmaxf(color.z, 0.0f), 1.0f);
            buffer->pix[y * buffer->w + x].r = (UInt8)(color.x * 255);
            buffer->pix[y * buffer->w + x].g = (UInt8)(color.y * 255);
            buffer->pix[y * buffer->w + x].b = (UInt8)(color.z * 255);
            buffer->pix[y * buffer->w + x].a = 255;
        }
    }
}

static SdfHit builtinSceneSdf(Vec3 p, Float32 time, Any userData)
{
    use(userData);
    SdfHit hit = {100.0f, 0};

    Float32 ground = sdfPlane(p, (Vec3){0, 1, 0}, 2.0f);
    if(ground < hit.dist) { hit.dist = ground; hit.matId = 1; }

    Vec3 sp1 = {sinf(time) * 3.0f, 0.0f, 5.0f};
    Float32 sphere1 = sdfSphere(vec3Sub(p, sp1), 1.0f);
    if(sphere1 < hit.dist) { hit.dist = sphere1; hit.matId = 2; }

    Vec3 bp = {-2.5f, -0.5f + sinf(time * 1.5f) * 0.5f, 6.0f};
    Float32 box1 = sdfBox(vec3RotateY(vec3Sub(p, bp), time * 0.8f), (Vec3){0.7f, 0.7f, 0.7f});
    if(box1 < hit.dist) { hit.dist = box1; hit.matId = 3; }

    Vec3 tp = {2.5f, 0.0f, 7.0f};
    Float32 torus1 = sdfTorus(opTwist(vec3Sub(p, tp), sinf(time * 0.3f) * 0.3f), 0.9f, 0.35f);
    if(torus1 < hit.dist) { hit.dist = torus1; hit.matId = 4; }

    Vec3 cp = {0.0f, -0.5f, 4.0f};
    Float32 cyl = sdfCylinder(vec3Sub(p, cp), 1.5f, 0.4f);
    if(cyl < hit.dist) { hit.dist = cyl; hit.matId = 5; }

    Vec3 op = {-5.0f, 0.5f, 8.0f};
    Float32 octa = sdfOctahedron(vec3RotateY(vec3Sub(p, op), time * 0.5f), 1.2f);
    if(octa < hit.dist) { hit.dist = octa; hit.matId = 6; }

    Vec3 cap_a = {4.0f, -1.0f, 5.0f};
    Vec3 cap_b = {5.0f, 1.0f, 6.0f};
    Float32 capsule = sdfCapsule(p, cap_a, cap_b, 0.3f);
    if(capsule < hit.dist) { hit.dist = capsule; hit.matId = 7; }

    return hit;
}

static Vec3 builtinGetMaterial(Int32 matId, Vec3 p, Float32 time)
{
    use(time);
    switch(matId)
    {
        case 1: {
            Float32 checker = fmodf(floorf(p.x * 0.5f) + floorf(p.z * 0.5f), 2.0f);
            return checker > 0.5f ? (Vec3){0.7f, 0.7f, 0.7f} : (Vec3){0.3f, 0.3f, 0.3f};
        }
        case 2: return (Vec3){1.0f, 0.3f, 0.2f};
        case 3: return (Vec3){0.2f, 0.5f, 1.0f};
        case 4: return (Vec3){0.9f, 0.7f, 0.1f};
        case 5: return (Vec3){0.2f, 0.8f, 0.3f};
        case 6: return (Vec3){0.8f, 0.2f, 0.8f};
        case 7: return (Vec3){0.3f, 0.8f, 0.8f};
        default: return (Vec3){0.5f, 0.5f, 0.5f};
    }
}

static Vec3 builtinGetNormal(Vec3 p, Float32 time)
{
    Float32 eps = 0.001f;
    Float32 d = builtinSceneSdf(p, time, null).dist;
    Vec3 n = {
        d - builtinSceneSdf((Vec3){p.x - eps, p.y, p.z}, time, null).dist,
        d - builtinSceneSdf((Vec3){p.x, p.y - eps, p.z}, time, null).dist,
        d - builtinSceneSdf((Vec3){p.x, p.y, p.z - eps}, time, null).dist
    };
    return vec3Norm(n);
}

static SdfHit builtinRaymarch(Vec3 ro, Vec3 rd, Float32 time, Int32 maxSteps, Float32 maxDist, Float32 surfDist)
{
    Float32 dist = 0.0f;
    SdfHit hit = {maxDist, 0};
    for(Int32 i = 0; i < maxSteps; ++i)
    {
        Vec3 p = vec3Add(ro, vec3Mul(rd, dist));
        hit = builtinSceneSdf(p, time, null);
        dist += hit.dist;
        if(hit.dist < surfDist || dist > maxDist) break;
    }
    hit.dist = dist;
    return hit;
}

Vec3 builtinSdfRaymarchShader(ShaderContext* ctx)
{
    Vec3 right = vec3Norm(vec3Cross(ctx->camDir, ctx->camUp));
    Float32 scale = tanf(ctx->fov * 0.5f);
    Vec3 rd = vec3Norm(vec3Add(vec3Add(ctx->camDir, vec3Mul(right, ctx->u * scale)), vec3Mul(ctx->camUp, ctx->v * scale)));

    SdfHit hit = builtinRaymarch(ctx->camPos, rd, ctx->time, 80, 100.0f, 0.001f);

    if(hit.dist < 100.0f)
    {
        Vec3 p = vec3Add(ctx->camPos, vec3Mul(rd, hit.dist));
        Vec3 n = builtinGetNormal(p, ctx->time);
        Vec3 lightPos = {5.0f, 8.0f, -5.0f};
        Vec3 lightDir = vec3Norm(vec3Sub(lightPos, p));

        Float32 diff = fmaxf(vec3Dot(n, lightDir), 0.0f);
        Vec3 viewDir = {-rd.x, -rd.y, -rd.z};
        Vec3 halfDir = vec3Norm(vec3Add(lightDir, viewDir));
        Float32 spec = powf(fmaxf(vec3Dot(n, halfDir), 0.0f), 32.0f);

        SdfHit shadow = builtinRaymarch(vec3Add(p, vec3Mul(n, 0.02f)), lightDir, ctx->time, 40, vec3Len(vec3Sub(lightPos, p)), 0.001f);
        Float32 shadowFactor = shadow.dist < vec3Len(vec3Sub(lightPos, p)) - 0.1f ? 0.3f : 1.0f;

        Vec3 baseColor = builtinGetMaterial(hit.matId, p, ctx->time);
        Vec3 color = vec3Mul(baseColor, 0.15f + diff * 0.7f * shadowFactor);
        color = vec3Add(color, vec3Mul((Vec3){1, 1, 1}, spec * shadowFactor * 0.4f));

        Float32 fog = expf(-hit.dist * 0.025f);
        color = vec3Add(vec3Mul(color, fog), vec3Mul((Vec3){0.05f, 0.08f, 0.12f}, 1.0f - fog));
        return color;
    }

    Float32 t = 0.5f * (rd.y + 1.0f);
    return vec3Add(vec3Mul((Vec3){0.02f, 0.04f, 0.08f}, 1.0f - t), vec3Mul((Vec3){0.1f, 0.15f, 0.25f}, t));
}

Vec3 builtinGridShader(ShaderContext* ctx)
{
    Vec3 right = vec3Norm(vec3Cross(ctx->camDir, ctx->camUp));
    Float32 scale = tanf(ctx->fov * 0.5f);
    Vec3 rd = vec3Norm(vec3Add(vec3Add(ctx->camDir, vec3Mul(right, ctx->u * scale)), vec3Mul(ctx->camUp, ctx->v * scale)));

    Float32 t = (-2.0f - ctx->camPos.y) / rd.y;
    if(t > 0.0f && t < 100.0f)
    {
        Vec3 p = vec3Add(ctx->camPos, vec3Mul(rd, t));
        Float32 gridX = fabsf(fmodf(p.x, 1.0f));
        Float32 gridZ = fabsf(fmodf(p.z, 1.0f));
        Float32 lineWidth = 0.02f;
        if(gridX < lineWidth || gridX > 1.0f - lineWidth || gridZ < lineWidth || gridZ > 1.0f - lineWidth)
        {
            Float32 fade = expf(-t * 0.05f);
            return vec3Mul((Vec3){0.0f, 0.8f, 0.4f}, fade);
        }
        Float32 fade = expf(-t * 0.08f);
        return vec3Mul((Vec3){0.02f, 0.05f, 0.03f}, fade);
    }
    return (Vec3){0.01f, 0.02f, 0.03f};
}

Vec3 builtinNormalVisShader(ShaderContext* ctx)
{
    Vec3 right = vec3Norm(vec3Cross(ctx->camDir, ctx->camUp));
    Float32 scale = tanf(ctx->fov * 0.5f);
    Vec3 rd = vec3Norm(vec3Add(vec3Add(ctx->camDir, vec3Mul(right, ctx->u * scale)), vec3Mul(ctx->camUp, ctx->v * scale)));

    SdfHit hit = builtinRaymarch(ctx->camPos, rd, ctx->time, 64, 50.0f, 0.001f);
    if(hit.dist < 50.0f)
    {
        Vec3 p = vec3Add(ctx->camPos, vec3Mul(rd, hit.dist));
        Vec3 n = builtinGetNormal(p, ctx->time);
        return vec3Mul(vec3Add(n, (Vec3){1, 1, 1}), 0.5f);
    }
    return (Vec3){0.1f, 0.1f, 0.1f};
}

Vec3 builtinDepthShader(ShaderContext* ctx)
{
    Vec3 right = vec3Norm(vec3Cross(ctx->camDir, ctx->camUp));
    Float32 scale = tanf(ctx->fov * 0.5f);
    Vec3 rd = vec3Norm(vec3Add(vec3Add(ctx->camDir, vec3Mul(right, ctx->u * scale)), vec3Mul(ctx->camUp, ctx->v * scale)));

    SdfHit hit = builtinRaymarch(ctx->camPos, rd, ctx->time, 64, 50.0f, 0.001f);
    Float32 depth = 1.0f - hit.dist / 50.0f;
    depth = fmaxf(depth, 0.0f);
    return (Vec3){depth, depth, depth};
}

Vec3 builtinUvShader(ShaderContext* ctx)
{
    Float32 u = (ctx->u + 1.0f) * 0.5f;
    Float32 v = (ctx->v + 1.0f) * 0.5f;
    return (Vec3){u, v, 0.5f};
}

Vec3 builtinAmbientOcclusionShader(ShaderContext* ctx)
{
    Vec3 right = vec3Norm(vec3Cross(ctx->camDir, ctx->camUp));
    Float32 scale = tanf(ctx->fov * 0.5f);
    Vec3 rd = vec3Norm(vec3Add(vec3Add(ctx->camDir, vec3Mul(right, ctx->u * scale)), vec3Mul(ctx->camUp, ctx->v * scale)));

    SdfHit hit = builtinRaymarch(ctx->camPos, rd, ctx->time, 64, 50.0f, 0.001f);
    if(hit.dist < 50.0f)
    {
        Vec3 p = vec3Add(ctx->camPos, vec3Mul(rd, hit.dist));
        Vec3 n = builtinGetNormal(p, ctx->time);

        Float32 ao = 0.0f;
        Float32 sca = 1.0f;
        for(Int32 i = 0; i < 5; ++i)
        {
            Float32 hr = 0.01f + 0.12f * (Float32)i / 4.0f;
            Vec3 aoPos = vec3Add(p, vec3Mul(n, hr));
            Float32 dd = builtinSceneSdf(aoPos, ctx->time, null).dist;
            ao += (hr - dd) * sca;
            sca *= 0.95f;
        }
        Float32 occlusion = 1.0f - clamp(ao * 3.0f, 0.0f, 1.0f);
        Vec3 baseColor = builtinGetMaterial(hit.matId, p, ctx->time);
        return vec3Mul(baseColor, occlusion);
    }
    return (Vec3){0.1f, 0.1f, 0.15f};
}

Vec3 builtinEdgeDetectShader(ShaderContext* ctx)
{
    Vec3 right = vec3Norm(vec3Cross(ctx->camDir, ctx->camUp));
    Float32 scale = tanf(ctx->fov * 0.5f);
    Vec3 rd = vec3Norm(vec3Add(vec3Add(ctx->camDir, vec3Mul(right, ctx->u * scale)), vec3Mul(ctx->camUp, ctx->v * scale)));

    SdfHit hit = builtinRaymarch(ctx->camPos, rd, ctx->time, 64, 50.0f, 0.001f);
    if(hit.dist < 50.0f)
    {
        Vec3 p = vec3Add(ctx->camPos, vec3Mul(rd, hit.dist));
        Vec3 n = builtinGetNormal(p, ctx->time);

        Float32 edge = 1.0f - fmaxf(vec3Dot(n, vec3Mul(rd, -1.0f)), 0.0f);
        edge = powf(edge, 3.0f);

        Vec3 baseColor = builtinGetMaterial(hit.matId, p, ctx->time);
        Vec3 edgeColor = (Vec3){1.0f, 1.0f, 1.0f};
        return vec3Add(vec3Mul(baseColor, 1.0f - edge), vec3Mul(edgeColor, edge));
    }
    return (Vec3){0.0f, 0.0f, 0.0f};
}

Vec3 builtinFogShader(ShaderContext* ctx)
{
    Vec3 right = vec3Norm(vec3Cross(ctx->camDir, ctx->camUp));
    Float32 scale = tanf(ctx->fov * 0.5f);
    Vec3 rd = vec3Norm(vec3Add(vec3Add(ctx->camDir, vec3Mul(right, ctx->u * scale)), vec3Mul(ctx->camUp, ctx->v * scale)));

    SdfHit hit = builtinRaymarch(ctx->camPos, rd, ctx->time, 80, 100.0f, 0.001f);

    Vec3 fogColor = {0.4f + sinf(ctx->time * 0.5f) * 0.1f, 0.5f + cosf(ctx->time * 0.3f) * 0.1f, 0.7f + sinf(ctx->time * 0.7f) * 0.1f};

    if(hit.dist < 100.0f)
    {
        Vec3 p = vec3Add(ctx->camPos, vec3Mul(rd, hit.dist));
        Vec3 n = builtinGetNormal(p, ctx->time);
        Vec3 lightDir = vec3Norm((Vec3){0.5f, 1.0f, -0.3f});
        Float32 diff = fmaxf(vec3Dot(n, lightDir), 0.0f) * 0.7f + 0.3f;

        Vec3 baseColor = vec3Mul(builtinGetMaterial(hit.matId, p, ctx->time), diff);

        Float32 fogAmount = 1.0f - expf(-hit.dist * 0.04f);
        return vec3Add(vec3Mul(baseColor, 1.0f - fogAmount), vec3Mul(fogColor, fogAmount));
    }

    return fogColor;
}

Vec3 builtinCelShadingShader(ShaderContext* ctx)
{
    Vec3 right = vec3Norm(vec3Cross(ctx->camDir, ctx->camUp));
    Float32 scale = tanf(ctx->fov * 0.5f);
    Vec3 rd = vec3Norm(vec3Add(vec3Add(ctx->camDir, vec3Mul(right, ctx->u * scale)), vec3Mul(ctx->camUp, ctx->v * scale)));

    SdfHit hit = builtinRaymarch(ctx->camPos, rd, ctx->time, 80, 100.0f, 0.001f);

    if(hit.dist < 100.0f)
    {
        Vec3 p = vec3Add(ctx->camPos, vec3Mul(rd, hit.dist));
        Vec3 n = builtinGetNormal(p, ctx->time);
        Vec3 lightPos = {5.0f, 8.0f, -5.0f};
        Vec3 lightDir = vec3Norm(vec3Sub(lightPos, p));

        Float32 diff = fmaxf(vec3Dot(n, lightDir), 0.0f);

        Float32 celDiff;
        if(diff > 0.9f) celDiff = 1.0f;
        else if(diff > 0.5f) celDiff = 0.7f;
        else if(diff > 0.2f) celDiff = 0.4f;
        else celDiff = 0.2f;

        Float32 fresnel = 1.0f - fmaxf(vec3Dot(n, vec3Mul(rd, -1.0f)), 0.0f);
        Float32 outline = fresnel > 0.7f ? 0.0f : 1.0f;

        Vec3 baseColor = builtinGetMaterial(hit.matId, p, ctx->time);
        return vec3Mul(baseColor, celDiff * outline);
    }

    return (Vec3){0.7f, 0.8f, 0.9f};
}
