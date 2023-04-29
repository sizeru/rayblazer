#ifndef RAYBLAZER_PRIMITIVES
#define RAYBLAZER_PRIMITIVES

#include <math.h>
#include <ostream>
#include <vector>
#define GLM_FORCE_INTRINSICS
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t   i8;
typedef int16_t  i16;
typedef int32_t  i32;
typedef int64_t  i64;
typedef float    f32;
typedef double   f64;

#define UNIT 2e-4f /* Smallest unit of length in this project */
#define CAM_DISTANCE 0.5f /* distance of camera from render plane. AKA FOV */

typedef glm::vec3 Vec3, Coord, Vertex;
typedef glm::vec4 Vec4;

std::ostream &operator<<(std::ostream &os, Vec3 const& v);

struct Ray {
    Coord origin;
    Vec3 direction;
};

struct alignas(16) Triangle { // 16-byte align fits better on cache lines"
    // The bare minimum data we're going to need right now is:
    // - 3 object vertices in each triangle
    // - 1 normal per triangle
    // > Therefore 6 floats per triangle
    
    // By the Havel-Herout, algo a triangle is defined by 3 planes
    u32 index[3];
    Vec3 normal; // Used for Havel-Herout algo

};

struct Object {
    Coord origin;
    std::vector<glm::vec3> vertices;
    std::vector<Triangle> triangles;

    Object(Coord origin) : origin(origin) {}
    Object() : origin() {}

    // inline Vec3 vertexAt(u32 index) {
    //     return vertices[index];
    // }
};

std::ostream &operator<<(std::ostream &os, Triangle const& t);

struct Plane {
    // planes, to sync up with the screen coordinate system, Start have +X
    // facing right and +Y facing down.
    Coord start;
    Vec3 down, right;
};

struct Camera {
    Coord origin;
    Vec3 direction;
    Plane clipPlane;

    /* Camera constructors. The default position and direction of the camera is
    a pythagorean quadruple, allowing for basic unit vectors without using
    square roots*/
    Camera() :  origin(Vec3{3.0, 2.0, 6.0}), 
                direction(Vec3{-3.0/7.0, -2.0/7.0, -6.0/7.0}) {}
    
    // Return a vector that shoots from the camera through a pixel
    Vec3 getPixelRay(u32 x, u32 y);
    void generatePlane(f32 width, f32 height);
};

struct Scene {
    Coord origin;   
    Camera camera; 
    std::vector<Object> objects;
    
    Scene(Coord origin) : origin(origin) {}
    Scene() : origin(Coord{0.0, 0.0, 0.0}) {}
};



#endif /* RAYBLAZER_PRIMITIVES */
