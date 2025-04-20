#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <iostream>
#include <cmath>
#include <vector>
#include "dice.hpp"
//#include "slider.hpp" // Removed slider header include
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/component_wise.hpp>

#ifdef _WIN32 // Include this block only for Windows builds
#include <Windows.h>

void MakeWindowTransparent(sf::Window& window) {
    HWND hwnd = window.getSystemHandle(); // Get the Windows handle of the SFML window
    SetWindowLongPtr(hwnd, GWL_EXSTYLE, GetWindowLongPtr(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED); // Set layered window attribute
    SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 0, LWA_COLORKEY); // Set transparency color key to RGB(0,0,0) - black
}
#endif

sf::Window* InitialiseWindow() {
    int screenWidth = GetSystemMetrics(SM_CXSCREEN); // Get screen width
    int screenHeight = GetSystemMetrics(SM_CYSCREEN); // Get full screen height
    int taskbarHeight = 0;

    // Get taskbar height (more robust method)
    APPBARDATA appBarData;
    appBarData.cbSize = sizeof(APPBARDATA);
    if (SHAppBarMessage(ABM_GETTASKBARPOS, &appBarData)) {
        taskbarHeight = appBarData.rc.bottom - appBarData.rc.top;
    } else {
        // Fallback if SHAppBarMessage fails (less accurate, may not work in all taskbar configurations)
        taskbarHeight = GetSystemMetrics(SM_CYEDGE) + GetSystemMetrics(SM_CYCAPTION);
    }


    int windowHeight = screenHeight - taskbarHeight; // Calculate window height excluding taskbar

    sf::ContextSettings settings;
    settings.depthBits = 24; // Set depth bits for OpenGL context
    settings.stencilBits = 8; // Set stencil bits for OpenGL context
    settings.antialiasingLevel = 4; // Set antialiasing level for OpenGL context
    settings.majorVersion = 3; // Set OpenGL major version
    settings.minorVersion = 3; // Set OpenGL minor version

    // Create SFML VideoMode and Window
    sf::VideoMode videoMode(screenWidth, windowHeight);
    sf::Window* window = new sf::Window(videoMode, "SFML Maximized Below Taskbar", sf::Style::None, settings);

    return window;
}

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
    in vec3 fragColor; // Input from vertex shader
    out vec4 FragColor;
    void main() {
        FragColor = vec4(fragColor, 1.0f); // Output vertex color
    }
    )";

// Shader source code - SIMPLIFIED FRAGMENT SHADER (SOLID BLUE)
const char* fragmentShaderSimple = R"(
    #version 330 core
    out vec4 FragColor;
    void main() {
        FragColor = vec4(0.0f, 0.0f, 1.0f, 1.0f); // Solid blue
    }
    )";

// Function to compile shaders
GLuint compileShader(const char* shaderSource, GLenum shaderType) {
    GLuint shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, &shaderSource, NULL);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cerr << "Shader Compilation Error (" << (shaderType == GL_VERTEX_SHADER ? "Vertex" : "Fragment") << "):\n" << infoLog << std::endl;
        return 0;
    }
    return shader;
}

// Function to create a shader program
GLuint createShaderProgram(GLuint vertexShader, GLuint fragmentShader) {
    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        std::cerr << "Shader Program Linking Error:\n" << infoLog << std::endl;
        return 0;
    }
    return program;
}

// Function to unproject screen coordinates to world coordinates
glm::vec3 unProject(const sf::Vector2i& screenCoords, const glm::mat4& projection, const glm::mat4& view, int windowWidth, int windowHeight) {
    // Normalized device coordinates
    float x = (2.0f * screenCoords.x) / windowWidth - 1.0f;
    float y = 1.0f - (2.0f * screenCoords.y) / windowHeight;
    float z = 1.0f; // We want to unproject to the far plane initially

    glm::vec3 rayNDC = glm::vec3(x, y, z);
    glm::vec4 rayClip = glm::vec4(rayNDC.x, rayNDC.y, rayNDC.z, 1.0);

    // Eye coordinates
    glm::mat4 inverseProjection = glm::inverse(projection);
    glm::vec4 rayEye = inverseProjection * rayClip;
    rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0, 0.0); // Direction of the ray, not a point

    // World coordinates
    glm::mat4 inverseView = glm::inverse(view);
    glm::vec4 rayWorld4D = inverseView * rayEye;
    glm::vec3 rayWorldDir = glm::normalize(glm::vec3(rayWorld4D));

    return rayWorldDir; // Return ray direction
}

// Function to check ray-bounding box intersection (AABB for simplicity)
bool rayIntersectsAABB(const glm::vec3& rayOrigin, const glm::vec3& rayDir, const glm::vec3& aabbMin, const glm::vec3& aabbMax) {
    float tmin = -INFINITY, tmax = INFINITY;

    glm::vec3 bounds[] = { aabbMin, aabbMax };

    for (int i = 0; i < 3; ++i) {
        float invD = 1.0f / rayDir[i];
        float t0 = (bounds[0][i] - rayOrigin[i]) * invD;
        float t1 = (bounds[1][i] - rayOrigin[i]) * invD;

        if (invD < 0.0f)
            std::swap(t0, t1);

        tmin = std::max(tmin, t0);
        tmax = std::min(tmax, t1);

        if (tmax <= tmin)
            return false;
    }

    return true;
}

void checkGLError(const char* operation) {
    GLenum error;
    while ((error = glGetError()) != GL_NO_ERROR) {
        std::cerr << "OpenGL Error after " << operation << ": " << operation << ": " << error << std::endl;
        switch (error) {
            case GL_INVALID_ENUM:     std::cerr << "GL_INVALID_ENUM" << std::endl; break;
            case GL_INVALID_VALUE:    std::cerr << "GL_INVALID_VALUE" << std::endl; break;
            case GL_INVALID_OPERATION: std::cerr << "GL_INVALID_OPERATION" << std::endl; break;
            case GL_STACK_OVERFLOW:    std::cerr << "GL_STACK_OVERFLOW" << std::endl; break;
            case GL_STACK_UNDERFLOW: std::cerr << "GL_STACK_UNDERFLOW" << std::endl; break;
            case GL_OUT_OF_MEMORY:     std::cerr << "GL_OUT_OF_MEMORY" << std::endl; break;
            case GL_INVALID_FRAMEBUFFER_OPERATION: std::cerr << "GL_INVALID_FRAMEBUFFER_OPERATION" << std::endl; break;
        }
    }
}


int main() {
    std::cout << "Program starting..." << std::endl;
    sf::Window* windowPtr = InitialiseWindow(); // Receive a pointer
    sf::Window& window = *windowPtr; // Get a reference to the window to use it like before
    std::cout << "SFML Window Created!" << std::endl;
    #ifdef _WIN32
    MakeWindowTransparent(window);
    std::cout << "Window transparet!" << std::endl;
    #endif
    bool isDragging = false;
    sf::Vector2i lastMousePos;
    float rotationSpeed = 0.1f; // Set a fixed rotation speed
    glm::quat combinedRotation;
    glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 5.0f);
    glm::vec3 cameraTarget = glm::vec3(0.0f);
    glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::quat rotationQuat = glm::quat(1.0f, 0.0f, 0.0f, 0.0f); // Identity quaternion (w, x, y, z) - w=1, others=0
    // Removed slider variables
    bool isSliderDragging = false; // Removed - no longer needed
    // Get d6 geometry data from dice.cpp
    //std::pair<std::vector<float>, std::vector<unsigned int>> geometryData = createCubeGeometry();
    //std::vector<float> d6vertices = geometryData.first;  // Extract vertices
    //std::vector<unsigned int> d6indices = geometryData.second; // Extract indices
    glm::vec3 cubeMin = glm::vec3(-0.5f, -0.5f, -0.5f);
    glm::vec3 cubeMax = glm::vec3(0.5f, 0.5f, 0.5f);
    bool isCubeClicked = false; // FLag to track if the cube is clicked
    sf::Vector2i flickStartPosition;
    sf::Vector2i flickEndPosition;
    bool isFlicking = false;
    glm::vec3 flickDirection = glm::vec3(0.0f);
    float flickForce = 0.0f;
    glm::vec3 angularVelocity = glm::vec3(0.0f);
    float angularDecayRate = 0.98f;
    float minAngularVelocity = 0.0001f; // Threshold to stop rolling
    std::cout << "Variables initilaised" << std::endl;
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return -1;
    }
    checkGLError("glewInit"); // Error check after GLEW init
    std::cout << "GLEW initialized" << std::endl;
    glEnable(GL_DEPTH_TEST);
    checkGLError("glEnable(GL_DEPTH_TEST)"); // Error check
    glDepthFunc(GL_LESS); // Explicitly set depth function to GL_LESS
    //glEnable(GL_BLEND);
    //checkGLError("glEnable(GL_BLEND)"); // Error check
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE); // Disable backface culling - ADD THIS LINE

    checkGLError("glBlendFunc"); // Error check
    std::cout << "OpenGL state set" << std::endl;
    GLuint vertexShader = compileShader(vertexShaderSource, GL_VERTEX_SHADER);
    checkGLError("compileShader - vertex shader"); // Error check
    GLuint fragmentShader = compileShader(fragmentShaderSource, GL_FRAGMENT_SHADER);
    checkGLError("compileShader - fragment shader"); // Error check
    GLuint shaderProgram = createShaderProgram(vertexShader, fragmentShader);
    checkGLError("createShaderProgram"); // Error check

    // **ADD THESE LINES: Check program linking status again and print log**
    GLint programLinkSuccess;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &programLinkSuccess);
    if (!programLinkSuccess) {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cerr << "Shader Program Linking Error ( повторная проверка ):\n" << infoLog << std::endl;
        // Even if linking fails, continue to check uniform locations to debug further
    }


    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    std::cout << "Shaders compiled and program linked" << std::endl;
    checkGLError("Shader cleanup"); // Error check after shader deletion
    GLuint VAO, VBO, EBO;
    unsigned int indexCount; // Declare indexCount here
    createCubeGeometry(VAO, VBO, EBO, 6, indexCount); // Call createCubeGeometry and get indexCount
    std::cout << "DEBUG: Index count passed to glDrawElements: " << indexCount << std::endl;
    checkGLError("createCubeGeometry"); // Error check
    std::cout << "Cube geometry created (VAO, VBO, EBO)" << std::endl;
    sf::Event event; // Declare event outside the loop
    std::cout << "Event made" << std::endl; // Debug: Other event types


    bool isWindowClosed = false; // **ADD THIS FLAG**

    // Main loop
    while (window.isOpen()) {

        checkGLError("Before pollEvent");

        while (window.pollEvent(event)) { // **BACK TO WHILE LOOP**
            checkGLError("After pollEvent");

            if (event.type == sf::Event::Closed) {
                std::cout << "Closing Window" << std::endl;
                isWindowClosed = true; // **SET FLAG BEFORE CLOSING**
                window.close();
                std::cout << "window.close() called" << std::endl;
                break;
            }
            else if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                    // Get matrices
                    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
                    glm::mat4 view = glm::lookAt(cameraPos, cameraTarget, cameraUp);
                    glm::mat4 model = glm::mat4_cast(rotationQuat); // Current cube model matrix
                    // Calculate ray origin and direction
                    glm::vec3 rayWorldDir = unProject(mousePos, projection, view, window.getSize().x, window.getSize().y);
                    glm::vec3 rayWorldOrigin = cameraPos; // Ray starts at camera position
                    // Apply inverse model matrix to AABB to test in model space (cube space)
                    glm::mat4 inverseModel = glm::inverse(model);
                    glm::vec3 rayOriginModelSpace = glm::vec3(inverseModel * glm::vec4(rayWorldOrigin, 1.0f));
                    glm::vec3 rayDirModelSpace = glm::normalize(glm::mat3(inverseModel) * rayWorldDir); // Normalize direction after transformation
                    // Perform ray-AABB intersection test in model space
                    if (rayIntersectsAABB(rayOriginModelSpace, rayDirModelSpace, cubeMin, cubeMax)) {
                        isCubeClicked = true;
                        isDragging = false; // Disable camera rotation when cube is clicked
                        isFlicking = true; // Start flicking when cube is clicked
                        flickStartPosition = mousePos; // Record flick start position
                    }
                    else {
                        isDragging = true;
                        isCubeClicked = false;
                        isFlicking = false; // Not flicking if background is clicked
                        lastMousePos = mousePos;
                    }
                }
            }
            else if (event.type == sf::Event::MouseButtonReleased) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    isDragging = false;
                    isCubeClicked = false; // Reset cube clicked state
                    if (isFlicking) { // If it was a flick gesture
                        isFlicking = false;
                        flickEndPosition = sf::Mouse::getPosition(window);
                        sf::Vector2i flickVector2D = flickEndPosition - flickStartPosition;
                        // Convert sf::Vector2i to glm::vec2 for force calculation
                        glm::vec2 flickVector2D_glm = glm::vec2(static_cast<float>(flickVector2D.x), static_cast<float>(flickVector2D.y)); // Explicit conversion
                        flickForce = glm::length(flickVector2D_glm) * 0.001f; // Use the glm::vec2 version
                        // Convert sf::Vector2i components to float for glm::vec3 construction
                        flickDirection = glm::normalize(glm::vec3(static_cast<float>(flickVector2D.x), -static_cast<float>(flickVector2D.y), 0.5f)); // Explicit conversion to float
                        angularVelocity = glm::cross(glm::vec3(0.0f, 0.0f, 1.0f), flickDirection) * flickForce;
                        std::cout << "Flicked! Force: " << flickForce << ", Direction: " << flickDirection.x << ", " << flickDirection.y << ", " << flickDirection.z << std::endl;
                    }
                }
            }
            else if (event.type == sf::Event::MouseMoved) {
                    if (isDragging && !isSliderDragging && !isCubeClicked) {
                    sf::Vector2i currentMousePos = sf::Mouse::getPosition(window);
                    sf::Vector2i delta = currentMousePos - lastMousePos;
                    // Create quaternions for rotation around Y and X axes based on mouse movement
                    glm::quat yRotationQuat = glm::angleAxis(glm::radians(-delta.x * rotationSpeed), glm::vec3(0.0f, 1.0f, 0.0f)); // Yaw (horizontal rotation)
                    glm::quat xRotationQuat = glm::angleAxis(glm::radians(-delta.y * rotationSpeed), glm::vec3(1.0f, 0.0f, 0.0f)); // Pitch (vertical rotation)
                    // Combine rotations - Apply pitch rotation AFTER yaw rotation (order matters!)
                    glm::quat incrementalRotation = xRotationQuat * yRotationQuat;
                    // Update the total rotation quaternion by multiplying with the incremental rotation
                    rotationQuat = incrementalRotation * rotationQuat; // Pre-multiply to apply rotation in world space effectively
                    lastMousePos = currentMousePos;
                }
            }
        } // End of WHILE event poll (changed back)


        // --- Apply Rolling Motion ---
        if (glm::length(angularVelocity) > minAngularVelocity) {
            glm::quat angularRotationQuat = glm::angleAxis(glm::length(angularVelocity) * rotationSpeed, glm::normalize(angularVelocity)); // Rotation from angular velocity
            rotationQuat = angularRotationQuat * rotationQuat; // Apply rotation
            angularVelocity *= angularDecayRate; // Apply decay
        }
        else {
            angularVelocity = glm::vec3(0.0f); // Stop rolling if velocity is very small
        }
        // --- End Apply Rolling Motion ---
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f); // Dark cyan, fully opaque
        //glClearColor(0.0f, 0.0f, 0.0f, 0.0f); // Transparent background (RGBA: Black, Alpha 0)
        // Clear the screen and depth buffer
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        
        // **Check uniform locations again right before setting them**
        GLint projectionLoc = glGetUniformLocation(shaderProgram, "projection"); // Declare projectionLoc here
        GLint viewLoc = glGetUniformLocation(shaderProgram, "view"); // Declare viewLoc here
        GLint modelLoc = glGetUniformLocation(shaderProgram, "model"); // Declare modelLoc here

        // **SKIP RENDERING IF WINDOW CLOSED**
        if (!isWindowClosed) { // **RENDERING CONDITION**
            // **ADD THIS LINE: Activate shader program BEFORE setting uniforms and drawing**
            glUseProgram(shaderProgram);
            checkGLError("glUseProgram"); // Error check after use program

            // Create transformation matrices
            glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
            glm::mat4 view = glm::lookAt(cameraPos, cameraTarget, cameraUp);
            glm::mat4 model = glm::mat4_cast(rotationQuat); // Apply rotation quaternion


            // Inside the render loop, before setting uniforms...
            GLint currentProgram;
            glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgram); // Get currently bound program ID
            if (currentProgram != shaderProgram) {
                std::cerr << "ERROR: Wrong shader program bound!" << std::endl;
            } else {
                // std::cout << "DEBUG: Shader OK. Uniforms: Proj=" << projectionLoc << ", View=" << viewLoc << ", Model=" << modelLoc << std::endl; // Temporarily uncomment
                if (projectionLoc == -1 || viewLoc == -1 || modelLoc == -1) {
                    //std::cerr << "ERROR: Invalid uniform location detected! Proj=" << projectionLoc << ", View=" << viewLoc << ", Model=" << modelLoc << std::endl;
                }
            }

            // Inside render loop, after calculating matrices, before glUniform...
            // Check Projection Matrix for NaN/Inf column by column
            bool projHasNanOrInf = false;
            for (int i = 0; i < 4; ++i) {
                if (glm::any(glm::isnan(projection[i])) || glm::any(glm::isinf(projection[i]))) {
                    projHasNanOrInf = true;
                    break;
                }
            }
            if (projHasNanOrInf) {
                std::cerr << "ERROR: Projection matrix contains NaN/Inf!" << std::endl;
            }

            // Check View Matrix for NaN/Inf column by column
            bool viewHasNanOrInf = false;
            for (int i = 0; i < 4; ++i) {
                if (glm::any(glm::isnan(view[i])) || glm::any(glm::isinf(view[i]))) {
                    viewHasNanOrInf = true;
                    break;
                }
            }
            if (viewHasNanOrInf) {
                std::cerr << "ERROR: View matrix contains NaN/Inf!" << std::endl;
            }

            // Check Model Matrix for NaN/Inf column by column
            bool modelHasNanOrInf = false;
            for (int i = 0; i < 4; ++i) {
                if (glm::any(glm::isnan(model[i])) || glm::any(glm::isinf(model[i]))) {
                    modelHasNanOrInf = true;
                    break;
                }
            }
            if (modelHasNanOrInf) {
                std::cerr << "ERROR: Model matrix contains NaN/Inf!" << std::endl;
            }


            // Pass matrices to the shader
            glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, &projection[0][0]);
            checkGLError("glUniformMatrix4fv - projection");
            glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view[0][0]);
            checkGLError("glUniformMatrix4fv - view");
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);
            checkGLError("glUniformMatrix4fv - model");

            checkGLError("Before Bind glBindVertexArray"); // Error check
            glBindVertexArray(VAO); // **Explicitly bind VAO before drawing**

            // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // Enable wireframe mode, needs to be before glDrawlements

            checkGLError("Before glDrawElements");
            glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
            checkGLError("After glDrawElements");
            glBindVertexArray(0); // **Unbind VAO after drawing**
            checkGLError("After Unbind glBindVertexArray"); // Error check


            // Update the window
            window.display();
        } else {
            std::cout << "Window rendering skipped because isWindowClosed is true" << std::endl; // Debug output
        }
    }

    std::cout << "Exited main loop!" << std::endl; // ADD THIS LINE - After main loop

    // Clean up - moved out of loop
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderProgram);
    delete windowPtr; // Delete window after loop
    std::cout << "Cleanup complete!" << std::endl; // ADD THIS LINE - After cleanup

    return 0;
}