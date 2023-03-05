#include <stdio.h>
#include "MiniFB_cpp.h"
#include <chrono>
#include "parser.hpp"
#include "primitives.hpp"

#define FRAMEWORK_WIDTH 2256
#define FRAMEWORK_HEIGHT 1504

// #define WIDTH FRAMEWORK_WIDTH
// #define HEIGHT FRAMEWORK_HEIGHT
#define ARGB_SIZE 4
#define WIDTH 800
#define HEIGHT 800
// Everything should be in ARGB

#include <math.h>
#include <ostream>
#include <iostream>

// // Subtract vector a - b
// // TODO: Check is using this provides better performance than operator overloads. Probably room for optimization.
// // TODO: Check vs MACROS, ETC
// Vector subtract(Vector a, Vector b) {
//     Vector c;
//     c.x = a.x - b.x;
//     c.y = a.y - b.y;
//     c.z = a.z - b.z;
//     return c;
// }

// The main render loop
int main() {
    // Change the last arg to change window type
    struct mfb_window *window = mfb_open_ex("TEST DISPLAY", WIDTH, HEIGHT, WF_FULLSCREEN); 
    if (!window)
        return 0;

    u32* buffer = (u32*) malloc(WIDTH * HEIGHT * ARGB_SIZE);
    
    Scene scene{};
    scene.objects.push_back(parse_object("../scenes/default_cube.obj")); // NOTE: This is actually an object, not a scene
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
                for (u32 o = 0; o < scene.objects.size(); o++) {
                    Object* obj = &scene.objects[o];
                    f32 minDepth = 10000000.0; // ten million should be enough for all of us
                    Vec3 isectNormal;
                    bool hit = false;
                    for (u32 t = 0; t < obj->triangles.size(); t++) {
                        // CHECK TRIANGLE INTERSECTION
                        // TODO: Switch this to using the Havel-Herout intersection algorithm
                        Vector& normal = obj->triangles[t].normal;
                        f32 cosine = Vec3::dot(lookDir, normal);
                        if (cosine > 0) {
                            continue;
                        }
                        Coord& a = obj->vertices[obj->triangles[t].index[0]];
                        Coord& b = obj->vertices[obj->triangles[t].index[1]];
                        Coord& c = obj->vertices[obj->triangles[t].index[2]];
                        
                        // Check plane intersection
                        f32 numerator = Vec3::dot(normal, a - scene.camera.origin);
                        f32 denom = Vec3::dot(lookDir, normal);
                        f32 depth = numerator / denom;
                        Vector p = scene.camera.origin + (lookDir * depth);

                        bool within_ab = Vec3::dot(Vec3::cross(b - a, p - a), normal) > 0;
                        bool within_bc = Vec3::dot(Vec3::cross(c - b, p - b), normal) > 0;
                        bool within_ca = Vec3::dot(Vec3::cross(a - c, p - c), normal) > 0;
                        if (depth < minDepth && depth > 0 && within_ab && within_bc && within_ca) {
                            f32 dp = Vec3::dot(-lookDir, normal);
                            if (dp > 0) {
                                dp *= 255.0;
                                buffer[i * WIDTH + j] = 0xff << 24 | (u32)dp << 16 | (u32)dp << 8 | (u32)dp;
                            }
                        }
                        // END TRIANGLE INTERSECTION
                    }
                    // if (hit) {

                    // }
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