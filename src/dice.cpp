//dice.cpp
#include "dice.hpp"
#include <vector>
#include <iostream>


float d6vertices[] = {
    // positions          // colors             // Face: Left (Yellow)
    -0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 0.0f, // 12 (was 4)
    -0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 0.0f, // 13 (was 0)
    -0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f, // 14 (was 3)
    -0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.0f, // 15 (was 7)

    // positions          // colors             // Face: Top (Magenta)
    -0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 1.0f, // 16 (was 3)
    0.5f,  0.5f, -0.5f,  1.0f, 0.0f, 1.0f, // 17 (was 2)
    0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 1.0f, // 18 (was 6)
    -0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 1.0f, // 19 (was 7)

    // positions          // colors             // Face: Bottom (Cyan)
    -0.5f, -0.5f,  0.5f,  0.0f, 1.0f, 1.0f, // 20 (was 4)
    0.5f, -0.5f,  0.5f,  0.0f, 1.0f, 1.0f, // 21 (was 5)
    0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 1.0f, // 22 (was 1)
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 1.0f,  // 23 (was 0)

    // positions          // colors             // Face: Front (Red)
    -0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f, // 0
    0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f, // 1
    0.5f,  0.5f, -0.5f,  1.0f, 0.0f, 0.0f, // 2
    -0.5f,  0.5f, -0.5f,  1.0f, 0.0f, 0.0f, // 3

    // positions          // colors             // Face: Right (Green)
    0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f, // 4 (was 1)
    0.5f, -0.5f,  0.5f,  0.0f, 1.0f, 0.0f, // 5
    0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 0.0f, // 6
    0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f, // 7 (was 2)

    // positions          // colors             // Face: Back (Blue)
    0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 1.0f, // 8 (was 5)
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 1.0f, // 9 (was 4)
    -0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f, // 10 (was 7)
    0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f, // 11 (was 6)
};

unsigned int d6indices[] = {
    0,  1,  2,  2,  3,  0,   // Front
    4,  5,  6,  6,  7,  4,   // Back
    8,  9,  10, 10, 11, 8,   // Top
    12, 13, 14, 14, 15, 12,  // Bottom
    16, 17, 18, 18, 19, 16,  // Right
    20, 21, 22, 22, 23, 20  // Left
};


void createCubeGeometry(GLuint& VAO, GLuint& VBO, GLuint& EBO, int dice, unsigned int& indexCount) {
    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    for (float val : d6vertices) {
        vertices.push_back(val);
    }
    for (unsigned int index : d6indices) {
        indices.push_back(index);
    }


    // After creating vertices and indices, add logging:
    // std::cout << "All Vertices: " << std::endl; // Corrected to 18 vertices (6 * 3 components)
    // for (int i = 0; i < vertices.size(); ++i) { // Print first 18 vertex components
    //     std::cout << "Vertex component " << i << ": " << vertices[i] << std::endl;
    // }

    std::cout << "First 6 indices: " << std::endl;
    for (int i = 0; i < 6 && i < indices.size(); ++i) { // Print first 6 indices
        std::cout << "Index " << i << ": " << indices[i] << std::endl;
    }
    indexCount = indices.size();


    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); // Unbind EBO

}