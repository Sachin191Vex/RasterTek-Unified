# CMakeList.txt : CMake project for RasterTek, include source and define
# project specific logic here.

# CMake minimum version requirement
cmake_minimum_required(VERSION 3.15)

# Project name and version
project(RasterTek VERSION 1.0)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Define the source files for the project
set(SOURCES
    src/main.cpp         # Add your main application file here
    inc/RasterTek.h      # Add additional source files as needed
    inc/systemclass.h
    src/systemclass.cpp
    inc/inputclass.h
    src/inputclass.cpp
    inc/applicationclass.h
    src/applicationclass.cpp
    inc/d3dclass.h
    src/d3dclass.cpp
    inc/shaderclass.h
    src/shaderclass.cpp
    inc/modelclass.h
    src/modelclass.cpp
    inc/cameraclass.h
    src/cameraclass.cpp
    inc/textureclass.h
    src/textureclass.cpp
    inc/lightclass.h
    src/lightclass.cpp
    shaders/color.vs     # Vertex shader (Rendering Color)
    shaders/color.ps     # Pixel shader (RRendering Color)
    shaders/texture.vs   # Vertex shader (Rendering Texture)
    shaders/texture.ps   # Pixel shader (Rendering Texture)
    shaders/light.vs     # Vertex shader (Rendering with Ligthing)
    shaders/light.ps     # Pixel shader (Rendering with Ligthing)
	README.md            # README file
)

# Define the executable target
add_executable(${PROJECT_NAME} ${SOURCES})

# Set the Windows subsystem (optional for GUI apps to suppress the console window)
set_target_properties(${PROJECT_NAME} PROPERTIES
    WIN32_EXECUTABLE TRUE
)

# Add include directories if needed
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/inc  # Add your include directory here
)

# Add required libraries (if any)
target_link_libraries(${PROJECT_NAME} PRIVATE
    user32.lib       # Example: Linking Windows libraries
    gdi32.lib        # Example: Linking GDI library
    d3d11.lib        # Example: DirectX 11 library
)

# Install target (optional)
install(TARGETS ${PROJECT_NAME} DESTINATION bin)

# Generate Visual Studio solution
if (MSVC)
    set(CMAKE_GENERATOR_PLATFORM x64) # Set to x64 or x86 based on your platform
endif ()
