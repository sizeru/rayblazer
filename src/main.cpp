#include <chrono>
#include <iostream>
#include "MiniFB_cpp.h"
#include "parser.hpp"
#include "primitives.hpp"
#include <thread>
#define GLM_FORCE_INTRINSICS /* This line is not necessary. The -mavx512f compiler flag does all the work for us */
#include "glm/glm.hpp"

#define FRAMEWORK_WIDTH 2256
#define FRAMEWORK_HEIGHT 1504

// #define WIDTH FRAMEWORK_WIDTH
// #define HEIGHT FRAMEWORK_HEIGHT
#define ARGB_SIZE 4
#define WIDTH FRAMEWORK_WIDTH
#define HEIGHT FRAMEWORK_HEIGHT
// Everything should be in ARGB

// Assuming there are 2 billion cycles per second per logical cpu cores, I have
// about 500 cycles per CPU per frame in order to raytrace each pixel

void renderRange(u32 startPixel, u32 endPixel, Scene* scene, u32* frameBuffer);

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

    // Setup threads
    using std::thread;
    const u8 numThreads = thread::hardware_concurrency(); // Get the number of logical cores (hardware threads)
    thread threads[numThreads];
    u32 threadBounds[numThreads + 1];
    for (int i = 0; i < numThreads; i++) {
        threadBounds[i] = i * ((WIDTH*HEIGHT)/numThreads);
    }
    threadBounds[numThreads] = WIDTH*HEIGHT;

    // Frame render loop
    do {
        auto startTime = std::chrono::high_resolution_clock::now();
        for (int t = 0; t < numThreads; t++) {
            threads[t] = thread{renderRange, threadBounds[t], threadBounds[t+1], &scene, buffer};
        }
        for (int t = 0; t < numThreads; t++) {
            threads[t].join();
        }
        auto endTime = std::chrono::high_resolution_clock::now();
        std::cout << "Frametime: " << std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count() << "ms" << std::endl;


        // mfb_update_state - Can remove this check if we can verify ourselves that we are doing everything correct
        if (STATE_OK != mfb_update_ex(window, buffer, WIDTH, HEIGHT)) {
            window = NULL;
            break;
        }
    } while(mfb_wait_sync(window));
}

void renderRange(u32 startPixel, u32 endPixel, Scene* scene, u32* frameBuffer) {
    // RENDERING HAPPENS IN THIS LOOP
    // TODO: Current performance is about 200ms per frame on a single core. This needs to be approximately 13x faster. Try running 13 threads?
    // TODO: Potential savings by allowing for quads as planes as well as triangles. 2 tris = 1 quad. Maybe this is canceled out by BVH.
    // TODO: Potential savings through smart upscaling
    for (u32 i = startPixel; i < endPixel; i++) { // TODO: Test rendering in columns vs rendering in rows
        Vec3 lookDir = scene->camera.getPixelRay(i % WIDTH, i / WIDTH);
        for (u32 o = 0; o < scene->objects.size(); o++) {
            Object* obj = &scene->objects[o];
            f32 minDepth = 10000000.0; // ten million should be enough for all of us
            bool hit = false;
            for (u32 t = 0; t < obj->triangles.size(); t++) {
                // CHECK TRIANGLE INTERSECTION
                // TODO: Switch this to using the Havel-Herout intersection algorithm
                // TODO: Use a BVH w/ refitting or Sweep BVH or Octree or BVH w/ Morton Code
                Vec3& normal = obj->triangles[t].normal;
                f32 cosine = glm::dot(lookDir, normal);
                bool backface = cosine > 0;
                if (backface) {
                    continue;
                }
                Coord& a = obj->vertices[obj->triangles[t].index[0]];
                Coord& b = obj->vertices[obj->triangles[t].index[1]];
                Coord& c = obj->vertices[obj->triangles[t].index[2]];
                
                // Check plane intersection

                f32 numerator = glm::dot(normal, a - scene->camera.origin);
                f32 denom = glm::dot(lookDir, normal);
                f32 depth = numerator / denom;
                Vec3 p = scene->camera.origin + (lookDir * depth);

                bool within_ab = glm::dot(glm::cross(b - a, p - a), normal) > 0; // These lines alone take 100ms
                bool within_bc = glm::dot(glm::cross(c - b, p - b), normal) > 0; // These lines alone take 100ms
                bool within_ca = glm::dot(glm::cross(a - c, p - c), normal) > 0; // These lines alone take 100ms
                bool replace = depth < minDepth && depth > 0 && within_ab && within_bc && within_ca;
                u32 newColor = cosine * -255.0 * replace;
                u32 oldColor = frameBuffer[i];
                u32 px = oldColor * !replace + newColor * replace;
                frameBuffer[i] = 0xff << 24 | px << 16 | px << 8 | px;
                // END TRIANGLE INTERSECTION
            }
        }
    }
    // RENDERING ENDS HERE
}