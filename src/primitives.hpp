#ifndef RAYBLAZER_PRIMITIVES
#define RAYBLAZER_PRIMITIVES

#include <math.h>
#include <ostream>
#include <vector>

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

#define UNIT 2e-4 /* Smallest unit of length in this project */

typedef struct Vector {
    f32 x, y, z;

    Vector(){};
    Vector(f32 x, f32 y, f32 z) : x(x), y(y), z(z) {};
    
    Vector operator + (Vector other) {  return Vector{x + other.x, y + other.y, z + other.z};   }
    Vector operator - (Vector other) {  return Vector{x - other.x, y - other.y, z - other.z};   }
    Vector operator - ()             {  return Vector{-x, -y, -z}; }
    Vector operator * (f32 scalar)   {  return Vector{x * scalar, y * scalar, z * scalar};      }
    void   operator *= (f32 scalar)  {  x *= scalar;        y *= scalar;        z *= scalar;    }

    void normalize() {
        f32 scaling = 1.0/sqrtf(x*x + y*y + z*z);
        x *= scaling; y *= scaling; z *= scaling; 
    }

    inline static Vector cross(Vector u, Vector v) {
        return Vector{u.y*v.z - u.z*v.y, u.z*v.x - u.x*v.z, u.x*v.y - u.y*v.x};
    }


    inline static f32 dot(Vector u, Vector v) {
        return u.x*v.x + u.y*v.y + u.z*v.z;
    }

} Coord, Vector, Vertex, Vec3;

std::ostream &operator<<(std::ostream &os, Vector const& v);

struct Ray {
    Coord origin;
    Vector direction;
};

struct Triangle {
    // The bare minimum data we're going to need right now is:
    // - 3 object vertices in each triangle
    // - 1 normal per triangle
    // > Therefore 6 floats per triangle
    u32 index[3];
    Vector normal;
};

struct Object {
    Coord origin;
    std::vector<Coord> vertices;
    std::vector<Triangle> triangles;

    inline Vector vertexAt(u32 index) {
        return vertices[index];
    }
};

std::ostream &operator<<(std::ostream &os, Triangle const& t);

struct Plane {
    // planes, to sync up with the screen coordinate system, Start have +X
    // facing right and +Y facing down.
    Coord start;
    Vector down, right;
};

struct Camera {
    Coord origin;
    Vector direction;
    Plane clipPlane;

    /* Camera constructors. The default position and direction of the camera is
    a pythagorean quadruple, allowing for basic unit vectors without using
    square roots*/
    Camera() :  origin(Vec3{3.0, 2.0, 6.0}), 
                direction(Vec3{-3.0/7.0, -2.0/7.0, -6.0/7.0}) {}
    
    // Return a vector that shoots from the camera through a pixel
    Vector getPixelRay(u32 x, u32 y);
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
