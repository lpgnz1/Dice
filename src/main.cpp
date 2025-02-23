#include <SFML/Window.hpp>
#include <iostream>
#include <cmath>
#include <vector>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Shader source code
const char* vertexShaderSource = R"(
#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aColor;
uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
out vec3 fragColor;
void main() {
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    fragColor = aColor;
}
)";

const char* fragmentShaderSource = R"(
#version 330 core
in vec3 fragColor;
out vec4 FragColor;
void main() {
    FragColor = vec4(fragColor, 1.0);
}
)";

// Function to compile shaders
GLuint compileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    // Check for errors
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "Shader compilation error:\n" << infoLog << std::endl;
    }

    return shader;
}

// Function to create a shader program
GLuint createShaderProgram(const char* vertexSource, const char* fragmentSource) {
    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexSource);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentSource);

    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    // Check for errors
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        std::cerr << "Shader program linking error:\n" << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return program;
}

int main() {
    // Create an SFML window with OpenGL support
    sf::Window window(sf::VideoMode(800, 600), "SFML with Modern OpenGL", sf::Style::Default, sf::ContextSettings(24, 8, 4, 3, 3));

    // Store window center for rotation calculations
    glm::vec2 windowCenter(400.0f, 300.0f);
     
    // Mouse tracking variables
    bool isDragging = false;
    sf::Vector2i lastMousePos;
    float rotationX = 0.0f;
    float rotationY = 0.0f;
    float rotationSpeed = 0.005f;

    // Initialize GLEW
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return -1;
    }

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);

    // Compile shaders and create shader program
    GLuint shaderProgram = createShaderProgram(vertexShaderSource, fragmentShaderSource);

    // Define cube vertices
    std::vector<float> vertices = {
        // Front face (red)
        -0.5f, -0.5f,  0.5f,  0.8f, 0.2f, 0.2f,
         0.5f, -0.5f,  0.5f,  0.8f, 0.2f, 0.2f,
         0.5f,  0.5f,  0.5f,  0.8f, 0.2f, 0.2f,
        -0.5f,  0.5f,  0.5f,  0.8f, 0.2f, 0.2f,

        // Back face (blue)
        -0.5f, -0.5f, -0.5f,  0.2f, 0.2f, 0.8f,
         0.5f, -0.5f, -0.5f,  0.2f, 0.2f, 0.8f,
         0.5f,  0.5f, -0.5f,  0.2f, 0.2f, 0.8f,
        -0.5f,  0.5f, -0.5f,  0.2f, 0.2f, 0.8f,
        
        // Top face (green)
        -0.5f,  0.5f, -0.5f,  0.2f, 0.8f, 0.2f,
         0.5f,  0.5f, -0.5f,  0.2f, 0.8f, 0.2f,
         0.5f,  0.5f,  0.5f,  0.2f, 0.8f, 0.2f,
        -0.5f,  0.5f,  0.5f,  0.2f, 0.8f, 0.2f,
        
        // Bottom face (yellow)
        -0.5f, -0.5f, -0.5f,  0.8f, 0.8f, 0.2f,
         0.5f, -0.5f, -0.5f,  0.8f, 0.8f, 0.2f,
         0.5f, -0.5f,  0.5f,  0.8f, 0.8f, 0.2f,
        -0.5f, -0.5f,  0.5f,  0.8f, 0.8f, 0.2f,
        
        // Right face (purple)
         0.5f, -0.5f, -0.5f,  0.8f, 0.2f, 0.8f,
         0.5f,  0.5f, -0.5f,  0.8f, 0.2f, 0.8f,
         0.5f,  0.5f,  0.5f,  0.8f, 0.2f, 0.8f,
         0.5f, -0.5f,  0.5f,  0.8f, 0.2f, 0.8f,
        
        // Left face (cyan)
        -0.5f, -0.5f, -0.5f,  0.2f, 0.8f, 0.8f,
        -0.5f,  0.5f, -0.5f,  0.2f, 0.8f, 0.8f,
        -0.5f,  0.5f,  0.5f,  0.2f, 0.8f, 0.8f,
        -0.5f, -0.5f,  0.5f,  0.2f, 0.8f, 0.8f
    };

    // Updated indices for all faces
    std::vector<unsigned int> indices = {
        0,  1,  2,  2,  3,  0,  // Front
        4,  5,  6,  6,  7,  4,  // Back
        8,  9,  10, 10, 11, 8,  // Top
        12, 13, 14, 14, 15, 12, // Bottom
        16, 17, 18, 18, 19, 16, // Right
        20, 21, 22, 22, 23, 20  // Left
    };


    // Create VAO, VBO, and EBO
    GLuint VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    // Main loop
    while (window.isOpen()) {
        // Handle events
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
            else if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    isDragging = true;
                    lastMousePos = sf::Mouse::getPosition(window);
                }
            }
            else if (event.type == sf::Event::MouseButtonReleased) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    isDragging = false;
                }
            }
        }

                // Update rotation only when dragging
        if (isDragging) {
            sf::Vector2i currentMousePos = sf::Mouse::getPosition(window);
            sf::Vector2i delta = currentMousePos - lastMousePos;
            
            rotationY += delta.x * rotationSpeed;
            rotationX += delta.y * rotationSpeed;
            
            lastMousePos = currentMousePos;
        }

        // Update rotation only when dragging
        if (isDragging) {
            sf::Vector2i currentMousePos = sf::Mouse::getPosition(window);
            sf::Vector2i delta = currentMousePos - lastMousePos;
            
            rotationY += -delta.x * rotationSpeed;
            rotationX += delta.y * rotationSpeed;
            
            lastMousePos = currentMousePos;
        }

        // Clear the screen and depth buffer
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Use the shader program
        glUseProgram(shaderProgram);

        // Create transformation matrices
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
        glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

        // Create model matrix with mouse-controlled rotation
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::rotate(model, rotationX, glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, rotationY, glm::vec3(0.0f, 1.0f, 0.0f));

        // Pass matrices to the shader
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, &projection[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, &view[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, &model[0][0]);

        // Draw the cube
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

        // Update the window
        window.display();

    }

    // Clean up
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderProgram);

    return 0;
}