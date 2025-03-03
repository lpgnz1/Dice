// slider.hpp
#ifndef SLIDER_HPP
#define SLIDER_HPP

#include <SFML/Window.hpp>

// Function to draw the slider
void drawSlider(float sliderX, float sliderY, float sliderLength, float sliderHeight, float sliderHandleHeight, float sliderValue, sf::Window& window);

// Function to handle slider interaction (mouse events)
void handleSliderInteraction(float sliderX, float sliderY, float sliderLength, float sliderHeight, float sliderHandleHeight, float& sliderValue, float& rotationSpeed, float minRotationSpeed, float maxRotationSpeed, bool& isDragging, bool& isSliderDragging, sf::Event event, sf::Window& window);

#endif // SLIDER_HPP