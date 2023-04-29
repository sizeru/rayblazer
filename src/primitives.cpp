#include "primitives.hpp"
#define GLM_FORCE_INTRINSICS
#include <glm/glm.hpp>
#include <iostream>

std::ostream &operator<<(std::ostream &os, Triangle const& t) {
    return os << "Indices: (" << t.index[0] << ", " << t.index[1] << ", " << t.index[2] << 
            ") Normal: [" << t.normal.x << ", " << t.normal.y << ", " << t.normal.z << "]";
}

std::ostream &operator<<(std::ostream &os, Vec3 const& v) {
    return os << "[" << v.x << ", " << v.y << ", " << v.z << "]";
}

Vec3 Camera::getPixelRay(u32 x, u32 y) {
    Coord target = clipPlane.start + clipPlane.right * UNIT * float(x) + clipPlane.down * UNIT * float(y);
    Vec3 pixelVec = target - origin;
    return glm::normalize(pixelVec);
}

void Camera::generatePlane(f32 width, f32 height) {
    Plane plane;
    Vec3 left = glm::normalize(glm::cross(direction, Vec3{0.0, -1.0, 0.0}));
    Vec3 up = glm::normalize(glm::cross(direction, left));
    float halfWidth = width / 2.0;
    float halfHeight = height / 2.0;
    
    plane.start = origin + (direction*CAM_DISTANCE) + (left*halfWidth*UNIT) + (up*halfHeight*UNIT);
    plane.right = -left;
    plane.down = -up;
    // std::cout << "Plane calculated: Start: " << plane.start << " | End: " << 
    //         plane.start + (plane.right * width * UNIT) + (plane.down * height * UNIT) << std::endl;  
    clipPlane = plane;
}