cmake_minimum_required(VERSION 3.28)
project(DiceRoller)

# Set output directory for executables
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Specify paths (adjust these to match your actual installation paths)
set(SFML_ROOT "C:/SFML-3.0.0")
set(SFML_DIR "${SFML_ROOT}/lib/cmake/SFML")
set(GLEW_DIR "C:/glew-2.1.0")
set(GLM_DIR "C:/glm-0.9.9.8")

# Find SFML
find_package(SFML 3 REQUIRED COMPONENTS Window System Graphics)

# Add GLEW
include_directories(${GLEW_DIR}/include)
message(STATUS "GLEW Include Directory being added: ${GLEW_DIR}/include") 
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    link_directories(${GLEW_DIR}/lib/Release/x64)
else()
    link_directories(${GLEW_DIR}/lib/Release/Win32)
endif()

# Find OpenGL
find_package(OpenGL REQUIRED)

# Add source directory for header files
include_directories(src) 

# Add GLM
include_directories(${GLM_DIR})

# Add your executable
add_executable(main src/main.cpp
src/dice.cpp
src/slider.cpp)

# Set C++ standard
target_compile_features(main PRIVATE cxx_std_17)

# Link libraries
target_link_libraries(main 
    sfml-window
    sfml-system
    sfml-graphics
    OpenGL::GL
    glew32
)

# Copy DLLs to output directory
if(WIN32)
    # Create a list of DLLs to copy
    set(DLLS_TO_COPY
        "${SFML_ROOT}/bin/sfml-window-3.dll"
        "${SFML_ROOT}/bin/sfml-system-3.dll"
        "${SFML_ROOT}/bin/sfml-graphics-3.dll"
        "${GLEW_DIR}/bin/Release/x64/glew32.dll"
    )

    # Copy each DLL individually
    foreach(DLL IN LISTS DLLS_TO_COPY)
        if(EXISTS "${DLL}")
            add_custom_command(TARGET main POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${DLL}"
                    "$<TARGET_FILE_DIR:main>"
                COMMENT "Copying ${DLL} to output directory"
            )
        else()
            message(WARNING "DLL not found: ${DLL}")
        endif()
    endforeach()
endif()