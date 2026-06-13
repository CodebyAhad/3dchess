#pragma once
#include "Shader.h"
#include <glm/glm.hpp>

class Board3D {
public:
    Board3D();
    ~Board3D();

    bool init();
    void draw(const Shader& shader) const;

    
    static glm::vec3 squareCenter(int file, int rank);

    
    static constexpr float SQUARE_SIZE = 1.0f;
    static constexpr float BOARD_Y     = 0.0f;   

private:
    GLuint vao_ = 0, vbo_ = 0, ebo_ = 0;
    int    indexCount_ = 0;

    void buildMesh();
};
