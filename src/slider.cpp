// slider.cpp
#include "slider.hpp"
#include <GL/glew.h> // For OpenGL functions
#include <iostream> // For debugging (optional)
#include <algorithm> // For std::max, std::min

void drawSlider(float sliderX, float sliderY, float sliderLength, float sliderHeight, float sliderHandleHeight, float sliderValue, sf::Window& window) {
    glDisable(GL_DEPTH_TEST); // Disable depth test for 2D UI elements
    glUseProgram(0); // Use fixed function pipeline
    glBindVertexArray(0); // Unbind VAO for fixed function

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, window.getSize().x, 0, window.getSize().y); // Orthographic projection

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(sliderX, sliderY, 0.0f); // Position the slider

    // Draw slider track (gray rectangle)
    glColor3f(0.5f, 0.5f, 0.5f);
    glBegin(GL_QUADS);
        glVertex2f(0, 0);
        glVertex2f(sliderLength, 0);
        glVertex2f(sliderLength, sliderHeight);
        glVertex2f(0, sliderHeight);
    glEnd();

    // Calculate handle position based on sliderValue
    float handleX = sliderValue * sliderLength;

    // Draw slider handle (darker gray rectangle)
    glColor3f(0.3f, 0.3f, 0.3f);
    glBegin(GL_QUADS);
        glVertex2f(handleX - sliderHandleHeight / 2.0f, -sliderHandleHeight / 2.0f); // Center handle around handleX
        glVertex2f(handleX + sliderHandleHeight / 2.0f, -sliderHandleHeight / 2.0f);
        glVertex2f(handleX + sliderHandleHeight / 2.0f, sliderHeight + sliderHandleHeight / 2.0f);
        glVertex2f(handleX - sliderHandleHeight / 2.0f, sliderHeight + sliderHandleHeight / 2.0f);
    glEnd();

    glEnable(GL_DEPTH_TEST); // Re-enable depth testing
}


void handleSliderInteraction(float sliderX, float sliderY, float sliderLength, float sliderHeight, float sliderHandleHeight, float& sliderValue, float& rotationSpeed, float minRotationSpeed, float maxRotationSpeed, bool& isDragging, bool& isSliderDragging, sf::Event event, sf::Window& window) {
    if (event.type == sf::Event::MouseButtonPressed) {
        if (event.mouseButton.button == sf::Mouse::Left) {
            isDragging = true; // Keep cube dragging as before
            // lastMousePos will be updated in main loop

            // Check if mouse click is inside the slider's horizontal bounds
            float mouseX = static_cast<float>(event.mouseButton.x);
            float mouseY = static_cast<float>(event.mouseButton.y);

            if (mouseX >= sliderX && mouseX <= sliderX + sliderLength && mouseY >= sliderY && mouseY <= sliderY + sliderHandleHeight + sliderHeight) { // Increased height for easier handle click
                isSliderDragging = true;
            }
        }
    } else if (event.type == sf::Event::MouseButtonReleased) {
        if (event.mouseButton.button == sf::Mouse::Left) {
            isDragging = false; // Keep cube dragging as before
            isSliderDragging = false; // Stop slider dragging as well
        }
    } else if (event.type == sf::Event::MouseMoved && isDragging && isSliderDragging) { // Only handle slider drag when mouse is moved and dragging AND slider is being dragged
        sf::Vector2i currentMousePos = sf::Mouse::getPosition(window);
        float mouseX = static_cast<float>(currentMousePos.x);

        // Calculate slider value based on mouse X position within the slider track
        sliderValue = std::max(0.0f, std::min(1.0f, (mouseX - sliderX) / sliderLength)); // Clamp value between 0 and 1
        rotationSpeed = minRotationSpeed + sliderValue * (maxRotationSpeed - minRotationSpeed); // Update rotationSpeed
    }
}