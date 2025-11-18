#include "slicer/geometry.hpp"

#include <ostream>

namespace slicer {

std::ostream& operator<<(std::ostream& os, const Vec2& v) {
    return os << "Vec2{x=" << v.x() << ", y=" << v.y() << "}";
}

std::ostream& operator<<(std::ostream& os, const Vec3& v) {
    return os << "Vec2{x=" << v.x() << ", y=" << v.y() << ", z=" << v.z() << "}";
}

std::ostream& operator<<(std::ostream& os, const Segment2D& v) {
    return os << "Segment2D{v0=" << v.v0 << ", v1=" << v.v1 << "}";
}

std::ostream& operator<<(std::ostream& os, const Segment3D& v) {
    return os << "Segment3D{v0=" << v.v0 << ", v1=" << v.v1 << "}";
}

std::ostream& operator<<(std::ostream& os, const Triangle3D& v) {
    return os << "Triangle3D{v0=" << v.v0 << ", v1=" << v.v1 << ", v2=" << v.v2 << "}";
}

}