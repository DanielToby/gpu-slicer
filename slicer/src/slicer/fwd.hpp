#pragma once

namespace slicer {

struct Vec2 {
    float x;
    float y;
};

struct Vec3 {
    float x;
    float y;
    float z;
};

[[nodiscard]] inline Vec2 toVec2(const Vec3& vec3) {
    return {vec3.x, vec3.y};
}

struct Triangle2D {
    Vec2 v0;
    Vec2 v1;
    Vec2 v2;
};

struct Triangle3D {
    Vec3 normal;
    Vec3 v0;
    Vec3 v1;
    Vec3 v2;
};

}