#include "primitives.hpp"
#include <iostream>
#define CAM_DISTANCE 0.125f /* distance of camera from render plane */

std::ostream &operator<<(std::ostream &os, Triangle const& t) {
    return os << "Indices: (" << t.index[0] << ", " << t.index[1] << ", " << t.index[2] << 
            ") Normal: [" << t.normal.x << ", " << t.normal.y << ", " << t.normal.z << "]";
}

std::ostream &operator<<(std::ostream &os, Vector const& v) {
    return os << "[" << v.x << ", " << v.y << ", " << v.z << "]";
}

Vector Camera::getPixelRay(u32 x, u32 y) {
    Coord target = clipPlane.start + clipPlane.right * UNIT*x + clipPlane.down * UNIT*y;
    Vector pixelVec = target - origin;
    pixelVec.normalize();
    return pixelVec;
}

void Camera::generatePlane(f32 width, f32 height) {
    Plane plane;
    Vector left = Vec3::cross(direction, Vec3{0.0, -1.0, 0.0});
    Vector up = Vec3::cross(direction, left);
    left.normalize();
    up.normalize();
    f32 halfWidth = width / 2.0;
    f32 halfHeight = height / 2.0;
    
    plane.start = origin + (direction*CAM_DISTANCE) + (left*halfWidth*UNIT) + (up*halfHeight*UNIT);
    plane.right = -left;
    plane.down = -up;
    // std::cout << "Plane calculated: Start: " << plane.start << " | End: " << 
    //         plane.start + (plane.right * width * UNIT) + (plane.down * height * UNIT) << std::endl;  
    clipPlane = plane;
}