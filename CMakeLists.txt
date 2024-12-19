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
    src/main.cpp		 # Add your main application file here
    src/systemclass.cpp  # Add additional source files as needed
    src/inputclass.cpp
    src/applicationclass.cpp
)

# Define the executable target
add_executable(${PROJECT_NAME} ${SOURCES})

# Set the Windows subsystem (optional for GUI apps to suppress the console window)
set_target_properties(${PROJECT_NAME} PROPERTIES
    WIN32_EXECUTABLE TRUE
)

# Add include directories if needed
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/include  # Add your include directory here
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
