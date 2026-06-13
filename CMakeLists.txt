cmake_minimum_required(VERSION 3.16)
project(3DChess VERSION 1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)


find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)


find_path(GLM_INCLUDE_DIR glm/glm.hpp
    HINTS ${CMAKE_SOURCE_DIR}/lib/glm
          $ENV{GLM_DIR}/include
          /usr/include
          /usr/local/include)
if(NOT GLM_INCLUDE_DIR)
    message(FATAL_ERROR "GLM not found. Put it in lib/glm or install system-wide.")
endif()


set(SOURCES
    src/main.cpp

    src/core/BoardState.cpp
    src/core/MoveValidator.cpp
    src/core/MoveHistory.cpp
    src/core/CaptureTracker.cpp
    src/core/GameController.cpp

    src/renderer/Shader.cpp
    src/renderer/Board3D.cpp
    src/renderer/PieceMesh.cpp
    src/renderer/Highlighter.cpp
    src/renderer/UIPanel.cpp
    src/renderer/Renderer.cpp

    src/input/InputHandler.cpp

    src/ai/UCIBridge.cpp
    src/ai/DifficultyManager.cpp
)


list(APPEND SOURCES ${CMAKE_SOURCE_DIR}/lib/glad/glad.c)
set_source_files_properties(${CMAKE_SOURCE_DIR}/lib/glad/glad.c PROPERTIES LANGUAGE C)

add_executable(3DChess ${SOURCES})

target_include_directories(3DChess PRIVATE
    src
    lib
    lib/tinyobj
    ${GLM_INCLUDE_DIR}
    ${SDL2_INCLUDE_DIRS}
    ${OPENGL_INCLUDE_DIR}
)

target_link_libraries(3DChess PRIVATE
    ${SDL2_LIBRARIES}
    ${OPENGL_LIBRARIES}
)


if(WIN32)
    target_link_libraries(3DChess PRIVATE kernel32)

endif()


add_custom_command(TARGET 3DChess POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:3DChess>/assets"
    COMMENT "Copying assets to output directory"
)
