#include <string.h>
#include <sys/stat.h>
#include <string>
#include <iostream>
#include "parser.hpp"
// Parses scene files exported from other programs and puts them in a format
// which is quicker to calculate with.

// Right now can only parse singular objects.
Object parse_object(const char* fileName) {
    // - read all vertices from file into large array
    // - generate all triangles into massive vector
    // - return vector

    // TODO: Incremental parsing. This reads the entire file into memory right now. Would prefer to parse incrementally.
    struct stat fileStats;
    if (stat(fileName, &fileStats) != 0) {
        printf("Failed to get info about scene\n");
        exit(-1);
    };
    
    char fileContents[fileStats.st_size]; // TODO: Remove this +1 if possible
    FILE* sceneFile = fopen(fileName, "r");
    u32 bytes_read = fread(&fileContents, sizeof(char), fileStats.st_size, sceneFile);
    if(bytes_read != fileStats.st_size) {
        printf("%s:%i: Filesize not equal to bytes read\n", __FILE__, __LINE__);
        exit(-2);
    }

    // We now have the file in memory. Time to parse. We only parse vertices and
    // triangles right now.
    Object object;
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
            object.vertices.push_back(vertex);
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
            // For now we calculate the normal.
            // TODO: Use the per vertex normals
            auto &a = object.vertices[triangle.index[0]];
            auto &b = object.vertices[triangle.index[1]];
            auto &c = object.vertices[triangle.index[2]];
            auto cp = Vec3::cross(a - c, b - c);
            cp.normalize();
            triangle.normal = cp;
            object.triangles.push_back(triangle);
        }
        line = strtok(NULL, "\r\n");
    }

    // #ifdef DEBUG
    std::cout << "Vertices: " << object.vertices.size() << " | Triangles: " << object.triangles.size() << std::endl;
    for (int i = 0; i < object.triangles.size(); i++) {
        std::cout << "Triangle: " << object.triangles[i] << std::endl;
    }
    
    for (int i = 0; i < object.vertices.size(); i++) {
        std::cout << "Vertex: " << object.vertices[i] << std::endl;
    }
    // #endif

    return object;
}


// #include "obj_parser.h"


