cmake_minimum_required(VERSION 3.10)
project(gravity_sim LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_VERBOSE_MAKEFILE ON)

# # Пути к каталогам
# set(SOURCE_DIR ${CMAKE_SOURCE_DIR}/src)
# set(INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)
# set(SHADER_DIR ${CMAKE_SOURCE_DIR}/shaders)
# set(GLAD_DIR ${CMAKE_SOURCE_DIR}/third_party/glad)

# # Поиск файлов исходного кода и заголовков
# file(GLOB_RECURSE SRC_FILES ${SOURCE_DIR}/*.cpp)
# file(GLOB_RECURSE HEADER_FILES ${INCLUDE_DIR}/*.hpp)

# # Добавляем GLAD
# add_library(glad STATIC ${GLAD_DIR}/src/glad.c)
# set_target_properties(glad PROPERTIES LINKER_LANGUAGE C)
# target_include_directories(glad PUBLIC ${GLAD_DIR}/include)

# Поиск и подключение GLFW
find_package(PkgConfig REQUIRED)
pkg_search_module(GLFW REQUIRED glfw3)

# Поиск GLM (должен быть установлен через пакетный менеджер или вручную)
find_path(GLM_INCLUDE_DIR glm/glm.hpp)
if(NOT GLM_INCLUDE_DIR)
    message(FATAL_ERROR "GLM not found. Please install glm.")
endif()

# Основной исполняемый файл
add_executable(gravity_sim
    gravity_sim_3Dgrid.cpp
)

# # Подключаем директории
# target_include_directories(gravity_sim PRIVATE
#     ${INCLUDE_DIR}
#     ${GLFW_INCLUDE_DIRS}
#     ${GLM_INCLUDE_DIR}
#     ${GLAD_DIR}/include
# )

# Линковка библиотек
target_link_libraries(gravity_sim PRIVATE
    ${GLFW_LIBRARIES}
    GLEW
    GL
    X11
    pthread
    dl
    m
)

# # Передаём путь до шейдеров в виде препроцессорного определения
# target_compile_definitions(gravity_sim PRIVATE SHADER_DIR="${SHADER_DIR}")

# # Копирование каталога 'shaders' в папку с бинарником после сборки
# add_custom_command(TARGET gravity_sim POST_BUILD
#     COMMAND ${CMAKE_COMMAND} -E copy_directory
#     ${SHADER_DIR} $<TARGET_FILE_DIR:gravity_sim>/shaders
# )
