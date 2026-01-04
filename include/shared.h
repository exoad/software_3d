#ifndef SHARED_H
#define SHARED_H

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>

typedef int32_t Int32;
typedef int64_t Int64;
typedef uint32_t UInt32;
typedef uint64_t UInt64;
typedef char Int8;
typedef uint8_t UInt8;
typedef uintptr_t UPtr;
typedef size_t Size;
typedef float Float32;
typedef double Float64;
typedef int16_t Int16;
typedef uint16_t UInt16;
typedef void Void;
typedef const Int8* CharSeq;
typedef FILE CFile;
typedef bool Bool;
typedef Void* Any;

#ifdef __GNUC__
#define pure       __attribute__((pure))
#define hot        __attribute__((hot))
#define flatten    __attribute__((flatten))
#define constfx    __attribute__((const))
#define never      __attribute__((noreturn))
#define deprecated __attribute__((deprecated))
#define unused     __attribute__((unused))
#define packed     __attribute__((packed))
#else
#define pure
#define hot
#define flatten
#define constfx
#define never
#define deprecated
#define unused
#define packed
#endif

#define null NULL
#define simple static inline
#define use(x) (Void) (x)
#define PI 3.14159265358979323846f

typedef struct {
    Float32 x, y, z;
} Vec3;

#define VEC3_ZERO (Vec3) {0.0f, 0.0f, 0.0f}

simple Vec3 vec3Add(Vec3 a, Vec3 b)
{
    return (Vec3) {a.x + b.x, a.y + b.y, a.z + b.z};
}

simple Vec3 vec3Sub(Vec3 a, Vec3 b)
{
    return (Vec3) {a.x - b.x, a.y - b.y, a.z - b.z};
}

simple Vec3 vec3Mul(Vec3 a, Float32 s)
{
    return (Vec3) {a.x * s, a.y * s, a.z * s};
}

simple Float32 vec3Dot(Vec3 a, Vec3 b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

simple Float32 vec3Len(Vec3 a)
{
    return sqrtf(vec3Dot(a, a));
}

simple Vec3 vec3Norm(Vec3 a)
{
    Float32 len = vec3Len(a);
    return len > 0 ? vec3Mul(a, 1.0f / len) : (Vec3){0, 0, 0};
}

simple Vec3 vec3Reflect(Vec3 v, Vec3 n)
{
    return vec3Sub(v, vec3Mul(n, 2 * vec3Dot(v, n)));
}

simple Vec3 vec3Cross(Vec3 a, Vec3 b)
{
    return (Vec3) {
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
}

simple Float32 vec3Dist(Vec3 a, Vec3 b)
{
    return vec3Len(vec3Sub(a, b));
}

typedef struct {
    Float32 x, y;
} Vec2;

typedef struct {
    Vec3 normal;
    Float32 distance;
} Plane;


simple Bool geomRayPlaneIntersect(Vec3 origin, Vec3 dir, Plane plane, Float32* t)
{
    Float32 denom = vec3Dot(plane.normal, dir);
    if(fabsf(denom) > 1e-6f)
    {
        *t = (plane.distance - vec3Dot(plane.normal, origin)) / denom;
        return *t >= 0;
    }
    return false;
}

simple Vec3 vec3RotateY(Vec3 v, Float32 angle)
{
    Float32 c = cosf(angle);
    Float32 s = sinf(angle);
    return (Vec3){ v.x * c - v.z * s, v.y, v.x * s + v.z * c };
}

simple Vec3 vec3RotateX(Vec3 v, Float32 angle)
{
    Float32 c = cosf(angle);
    Float32 s = sinf(angle);
    return (Vec3){ v.x, v.y * c - v.z * s, v.y * s + v.z * c };
}

#endif