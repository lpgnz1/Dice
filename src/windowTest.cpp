#include <iostream>
#include <GL/glew.h> // GLEW must be included first!
#include <SFML/Window.hpp>
#include <SFML/OpenGL.hpp>

int main() {
    sf::ContextSettings settings;
    settings.profile = sf::ContextSettings::Compatibility; // **If SFML 3.0 has this option - CHECK DOCS**
    // OR - if no profile setting, try requesting older OpenGL version:
    // settings.majorVersion = 3; // Request OpenGL 3.1 or 3.0 if Compatibility Profile not direct option
    // settings.minorVersion = 1;

    sf::Window window(sf::VideoMode(800, 600), "Minimal Test", sf::Style::Default, settings);

    // GLEW Initialization - MUST be after window creation
    glewExperimental = GL_TRUE;
    GLenum glewError = glewInit();
    if (glewError != GLEW_OK) {
        std::cerr << "GLEW initialization error: " << glewGetErrorString(glewError) << std::endl;
        return -1;
    }
    std::cerr << "GLEW initialized successfully" << std::endl;


    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();
        }

        glClearColor(0.0f, 0.0f, 0.0f, 0.0f); // Transparent black
        GLenum error_clear_color = glGetError();
        if (error_clear_color != GL_NO_ERROR) std::cerr << "glClearColor error: " << error_clear_color << std::endl;
        glClear(GL_COLOR_BUFFER_BIT); // Only clear color buffer
        GLenum error_clear = glGetError();
        if (error_clear != GL_NO_ERROR) std::cerr << "glClear error: " << error_clear << std::endl;


        glDisable(GL_DEPTH_TEST); // Ensure no depth testing interference
        GLenum error_disable_depth = glGetError();
        if (error_disable_depth != GL_NO_ERROR) std::cerr << "glDisable(GL_DEPTH_TEST) error: " << error_disable_depth << std::endl;
        glUseProgram(0);         // Fixed function pipeline
        GLenum error_use_program = glGetError();
        if (error_use_program != GL_NO_ERROR) std::cerr << "glUseProgram(0) error: " << error_use_program << std::endl;
        glBindVertexArray(0);     // No VAOs
        GLenum error_bind_vao = glGetError();
        if (error_bind_vao != GL_NO_ERROR) std::cerr << "glBindVertexArray(0) error: " << error_bind_vao << std::endl;


        glColor3f(1.0f, 0.0f, 0.0f); // Red color, OPAQUE
        GLenum error_color = glGetError();
        if (error_color != GL_NO_ERROR) std::cerr << "glColor3f error: " << error_color << std::endl;
        glRectf(-0.5f, -0.5f, 0.5f, 0.5f); // Draw a red rectangle (OpenGL 1.x rect function) - in NDC
        GLenum error_rect = glGetError();
        if (error_rect != GL_NO_ERROR) std::cerr << "glRectf error: " << error_rect << std::endl;


        window.display();
        GLenum error_display = glGetError();
        if (error_display != GL_NO_ERROR) std::cerr << "window.display() error: " << error_display << std::endl;
    }
    return 0;
}