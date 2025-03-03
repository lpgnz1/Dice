// dice.hpp
#ifndef DICE_HPP
#define DICE_HPP

#include <vector>
#include <utility> // For std::pair
#include <GL/glew.h>

// std::pair<std::vector<float>, std::vector<unsigned int>> createCubeGeometry();

void createCubeGeometry(GLuint& VAO, GLuint& VBO, GLuint& EBO, int dice, unsigned int& indexCount);

#endif // DICE_HPP