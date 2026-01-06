#include "renderer.h"
#include <math.h>
#include "logger.h"

Bool intersectSphere(Vec3 origin, Vec3 dir, Sphere sphere, Float32* t)
{
    Vec3 oc = vec3Sub(origin, sphere.center);
    Float32 a = vec3Dot(dir, dir);
    Float32 b = 2 * vec3Dot(oc, dir);
    Float32 c = vec3Dot(oc, oc) - sphere.radius * sphere.radius;
    Float32 disc = b * b - 4 * a * c;
    if(disc < 0)
    {
        return false;
    }
    *t = (-b - sqrtf(disc)) / (2 * a);
    return *t > 0;
}

Vec3 phongShade(Vec3 hit, Vec3 normal, Vec3 viewDir, Vec3 lightPos, Vec3 lightColor, Vec3 objColor, Float32 shininess)
{
    Vec3 lightDir = vec3Norm(vec3Sub(lightPos, hit));
    Float32 diff = fmaxf(vec3Dot(normal, lightDir), 0.0f);
    Vec3 diffuse = vec3Mul(lightColor, diff);
    Vec3 reflectDir = vec3Reflect(
        vec3Mul(lightDir, -1),
        normal
    );
    Float32 spec = powf(
        fmaxf(
            vec3Dot(
                viewDir,
                reflectDir
            ),
            0.0f
        ),
        shininess
    );
    Vec3 specular = vec3Mul(lightColor, spec);
    Vec3 ambient = vec3Mul(objColor, 0.1f);
    Vec3 color = vec3Add(
        vec3Add(
            ambient,
            vec3Mul(diffuse, 0.8f)
        ),
        vec3Mul(specular, 0.5f)
    );
    return (Vec3) {
        fminf(color.x, 1.0f),
        fminf(color.y, 1.0f),
        fminf(color.z, 1.0f)
    };
}

Void renderScene(GfxBuffer* buffer, Camera cam, Sphere sphere, Light light, Vec3 objColor, Float32 shininess)
{
    LOG_D("Rendering frame...");
    Vec3 right = vec3Norm(vec3Cross(cam.dir, cam.up));
    Vec3 up = vec3Norm(vec3Cross(right, cam.dir));

    for(Int32 y = 0; y < buffer->height; ++y)
    {
        for(Int32 x = 0; x < buffer->width; ++x)
        {
            Float32 u = (2.0f * (x + 0.5f) / buffer->width - 1.0f) * tanf(cam.fov / 2.0f) * (buffer->width / (Float32)buffer->height);
            Float32 v = (1.0f - 2.0f * (y + 0.5f) / buffer->height) * tanf(cam.fov / 2.0f);
            Vec3 rayDir = vec3Norm(
                vec3Add(
                    vec3Add(
                        cam.dir,
                        vec3Mul(right, u)
                    ),
                    vec3Mul(up, v)
                )
            );
            Float32 t;
            Color* p = &buffer->pixels[y * buffer->width + x];
            if(intersectSphere(cam.pos, rayDir, sphere, &t))
            {
                Vec3 hit = vec3Add(
                    cam.pos,
                    vec3Mul(rayDir, t)
                );
                Vec3 color = phongShade(
                    hit,
                    vec3Norm(vec3Sub(hit, sphere.center)),
                    vec3Norm(vec3Sub(cam.pos, hit)),
                    light.pos,
                    light.color,
                    objColor,
                    shininess
                );
                p->r = (UInt8) (color.x * 255);
                p->g = (UInt8) (color.y * 255);
                p->b = (UInt8) (color.z * 255);
            }
            else
            {
                p->r = p->g = p->b = 0;
            }
        }
    }
}