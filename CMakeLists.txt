cmake_minimum_required(VERSION 3.10)
project(GravitySimulation)


set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)


include_directories(include)


find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_search_module(GLFW REQUIRED glfw3)

find_path(GLM_INCLUDE_DIR glm/glm.hpp)


set(SOURCES
    main.cpp
    src/utils.cpp
    src/Shader.cpp
    src/Camera.cpp
    src/Object.cpp
    src/Grid.cpp
    src/GravitySimulation.cpp
)


add_executable(gravity_sim ${SOURCES})

# Линкуем библиотеки
target_link_libraries(gravity_sim PRIVATE
    ${GLFW_LIBRARIES}
    GLEW
    GL
    X11
    pthread
    dl
    m
)



# add_executable(gravity_sim
#     gravity_sim.cpp
# )

# target_include_directories(gravity_sim PRIVATE
#     ${INCLUDE_DIR}
#     ${GLFW_INCLUDE_DIRS}
#     ${GLM_INCLUDE_DIR}
#     ${GLAD_DIR}/include
# )




# target_compile_definitions(gravity_sim PRIVATE SHADER_DIR="${SHADER_DIR}")

# add_custom_command(TARGET gravity_sim POST_BUILD
#     COMMAND ${CMAKE_COMMAND} -E copy_directory
#     ${SHADER_DIR} $<TARGET_FILE_DIR:gravity_sim>/shaders
# )