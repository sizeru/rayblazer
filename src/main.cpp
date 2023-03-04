#include <stdio.h>
#include "MiniFB_cpp.h"
#include <chrono>
#include "vector.hpp"
// #include "parser.hpp"

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


#define FRAMEWORK_WIDTH 2256
#define FRAMEWORK_HEIGHT 1504

// #define WIDTH FRAMEWORK_WIDTH
// #define HEIGHT FRAMEWORK_HEIGHT
#define ARGB_SIZE 4
#define WIDTH 800
#define HEIGHT 800
// Everything should be in ARGB

#define UNIT 2e-4
#define CAM_DISTANCE 0.25f /* distance of camera from render plane */

#include <math.h>
#include <ostream>
#include <iostream>

typedef struct Vector {
    f32 x, y, z;

    Vector(){};
    Vector(f32 x, f32 y, f32 z) : x(x), y(y), z(z) {};
    
    Vector operator + (Vector other) {  return Vector{x + other.x, y + other.y, z + other.z};   }
    Vector operator - (Vector other) {  return Vector{x - other.x, y - other.y, z - other.z};   }
    Vector operator - ()             {  return Vector{-x, -y, -z}; }
    Vector operator * (f32 scalar)   {  return Vector{x * scalar, y * scalar, z * scalar};      }
    void   operator *= (f32 scalar)  {  x *= scalar;        y *= scalar;        z *= scalar;    }
    // void   operator = (Vector other) {  x = other.x;        y = other.y;        z = other.z;    }

    void normalize() {
        f32 scaling = 1.0/sqrtf(x*x + y*y + z*z);
        x *= scaling; y *= scaling; z *= scaling; 
    }

} Coord, Vector, Vertex, Vec;

struct Ray {
    Vector origin;
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

std::ostream &operator<<(std::ostream &os, Vector const& v) {
    return os << "[" << v.x << ", " << v.y << ", " << v.z << "]";
}

std::ostream &operator<<(std::ostream &os, Triangle const& t) {
    return os << "Indices: (" << t.index[0] << ", " << t.index[1] << ", " << t.index[2] << 
            ") Normal: [" << t.normal.x << ", " << t.normal.y << ", " << t.normal.z << "]";
}

inline Vector cross(Vector u, Vector v) {
    return Vector{u.y*v.z - u.z*v.y, u.z*v.x - u.x*v.z, u.x*v.y - u.y*v.x};
}


inline f32 dot(Vector u, Vector v) {
    return u.x*v.x + u.y*v.y + u.z*v.z;
}

struct Plane {
    // planes, to sync up with the screen coordinate system, Start have +X
    // facing right and +Y facing down.
    Coord start;
    Vector down, right;
};

struct Camera {
    Coord position;
    Vector direction;
    Plane clipPlane;
    
    // Get the ray that shoots from the camera through the pixel
    inline Vector getPixelRay(u32 x, u32 y) {
        Coord target = clipPlane.start + clipPlane.right * UNIT*x + clipPlane.down * UNIT*y;
        Vector pixelVec = target - position;
        pixelVec.normalize();
        return pixelVec;
    }

    void generatePlane(f32 width, f32 height) {
        Plane plane;
        Vector left = cross(direction, Vector{0.0, -1.0, 0.0});
        Vector up = cross(direction, left);
        left.normalize();
        up.normalize();
        f32 halfWidth = width / 2.0;
        f32 halfHeight = height / 2.0;
        
        plane.start = position + (direction*CAM_DISTANCE) + (left*halfWidth*UNIT) + (up*halfHeight*UNIT);
        plane.right = -left;
        plane.down = -up;
        std::cout << "Plane calculated: Start: " << plane.start << " | End: " << 
                plane.start + (plane.right * width * UNIT) + (plane.down * height * UNIT) << std::endl;  
        clipPlane = plane;
    }
};


struct Scene {
    Camera camera; 
    std::vector<Vertex> vertices;
    std::vector<Triangle> triangles;
    
    inline Vector indexToVector(u32 index) {
        return vertices[index];
    }
};

Scene parse_scene(const char* sceneName);

// Subtract vector a - b
// TODO: Check is using this provides better performance than operator overloads. Probably room for optimization.
// TODO: Check vs MACROS, ETC
Vector subtract(Vector a, Vector b) {
    Vector c;
    c.x = a.x - b.x;
    c.y = a.y - b.y;
    c.z = a.z - b.z;
    return c;
}

// The main render loop
int main() {
    // Change the last arg to change window type
    struct mfb_window *window = mfb_open_ex("TEST DISPLAY", WIDTH, HEIGHT, WF_FULLSCREEN); 
    if (!window)
        return 0;

    u32* buffer = (u32*) malloc(WIDTH * HEIGHT * ARGB_SIZE);
    Scene scene = parse_scene("../scenes/default_cube.obj"); // NOTE: This is actually an object, not a scene
    scene.camera.position = Vector{0.0, 5, 10.0};
    scene.camera.direction = Vector{0.0, -5.0, -10.0};
    scene.camera.direction.normalize();
    scene.camera.generatePlane(WIDTH, HEIGHT);    
    #ifdef DEBUG
    std::cout << "Min lookDir: " << scene.camera.getPixelRay(0, 0) <<
            " | max lookDir: " << scene.camera.getPixelRay(WIDTH, HEIGHT) << std::endl;
    #endif 

    do {
        // RENDERING HAPPENS IN THIS LOOP
        auto startTime = std::chrono::high_resolution_clock::now();
        for (u32 i = 0; i < WIDTH; i++) { // TODO: Test rendering in columns vs rendering in rows
            for (u32 j = 0; j < HEIGHT; j++) {
                Vector lookDir = scene.camera.getPixelRay(i, j);
                // buffer[i * WIDTH + j] = lookDir.x + lookDir.y + lookDir.z;
                // TODO: I stopped here

                // f32 zdepth = 10000000.0; // ten million should be enough for all of us
                for (u32 t = 0; t < scene.triangles.size(); t++) {
                    // CHECK TRIANGLE INTERSECTION
                    Coord a = scene.vertices[scene.triangles[t].index[0]];
                    Coord b = scene.vertices[scene.triangles[t].index[1]];
                    Coord c = scene.vertices[scene.triangles[t].index[2]];
                    Vector normal = scene.triangles[t].normal;
                    
                    // Check plane intersection
                    f32 numerator = dot(normal, a - scene.camera.position);
                    f32 denom = dot(lookDir, normal);
                    f32 D = numerator / denom;
                    Vector p = scene.camera.position + (lookDir * D);

                    bool within_ab = dot(cross(b - a, p - a), normal) > 0;
                    bool within_bc = dot(cross(c - b, p - b), normal) > 0;
                    bool within_ca = dot(cross(a - c, p - c), normal) > 0;
                    if (D > 0 && within_ab && within_bc && within_ca) {
                        // std::cout << "HUH" << std::endl;
                        buffer[i * WIDTH + j] = 0xffffffff;
                        break;
                    }
                    // END TRIANGLE INTERSECTION
                }
                
            }
        }
        // RENDERING ENDS HERE
        // mfb_update_state - Can remove this check if we can verify ourselves that we are doing everything correct
        auto endTime = std::chrono::high_resolution_clock::now();
        std::cout << "Frametime: " << std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "us" << std::endl;
        if (STATE_OK != mfb_update_ex(window, buffer, WIDTH, HEIGHT)) {
            window = NULL;
            break;
        }
    } while(mfb_wait_sync(window));
}

#include <string.h>
#include <sys/stat.h>
#include <string>


// Right now can only parse singular objects.
Scene parse_scene(const char* sceneName) {
    // - read all vertices from file into large array
    // - generate all triangles into massive vector
    // - return vector

    // TODO: Incremental parsing. This reads the entire file into memory right now. Would prefer to parse incrementally.
    struct stat sceneStats;
    if (stat(sceneName, &sceneStats) != 0) {
        printf("Failed to get info about scene\n");
        exit(-1);
    };
    
    char fileContents[sceneStats.st_size]; // TODO: Remove this +1 if possible
    FILE* sceneFile = fopen(sceneName, "r");
    u32 bytes_read = fread(&fileContents, sizeof(char), sceneStats.st_size, sceneFile);
    if(bytes_read != sceneStats.st_size) {
        printf("%s:%i: Filesize not equal to bytes read\n", __FILE__, __LINE__);
        exit(-2);
    }

    // We now have the file in memory. Time to parse. We only parse vertices and
    // triangles right now.
    Scene scene;
    char* saveptr, *saveptr2;
    char* line = strtok(fileContents, "\r\n");
    while (line != NULL) {
        // SAVE VERTICES
        if (line[0] == 'v' && line[1] == ' ') {
            Vertex vertex;
            char* component = strtok_r(line + 2, " \n", &saveptr);
            vertex.x = std::atof(component);
            component = strtok_r(NULL, " \n", &saveptr);
            vertex.y = std::atof(component);
            component = strtok_r(NULL, " \n", &saveptr);
            vertex.z = std::atof(component);
            scene.vertices.push_back(vertex);
        }

        // SAVE TRIANGLES --- f 5/5/1 MEANS f v/vt/vn ??? YES vertex/uv/normal
        // - Only saving vertices right now.
        if (line[0] == 'f' && line[1] == ' ') {
            Triangle triangle;
            char* vertex = strtok_r(line + 2, " \n", &saveptr);
            char* index = strtok_r(vertex, "/\n", &saveptr2);
            triangle.index[0] = atoi(index) - 1;
            vertex = strtok_r(NULL, " \n", &saveptr);
            index = strtok_r(vertex, "/\n", &saveptr2);
            triangle.index[1] = atoi(index) - 1;
            vertex = strtok_r(NULL, " \n", &saveptr);
            index = strtok_r(vertex, "/\n", &saveptr2);
            triangle.index[2] = atoi(index) - 1; 

            auto &a = scene.vertices[triangle.index[0]];
            auto &b = scene.vertices[triangle.index[1]];
            auto &c = scene.vertices[triangle.index[2]];
            auto cp = cross(a - c, b - c);
            cp.normalize();
            triangle.normal = cp;
            scene.triangles.push_back(triangle);
        }
        line = strtok(NULL, "\r\n");
    }

    // #ifdef DEBUG
    std::cout << "Vertices: " << scene.vertices.size() << " | Triangles: " << scene.triangles.size() << std::endl;
    for (int i = 0; i < scene.triangles.size(); i++) {
        std::cout << "Triangle: " << scene.triangles[i] << std::endl;
    }
    
    for (int i = 0; i < scene.vertices.size(); i++) {
        std::cout << "Vertex: " << scene.vertices[i] << std::endl;
    }
    // #endif

    return scene;
}