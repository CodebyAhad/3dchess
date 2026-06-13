#pragma once
#include "Shader.h"
#include "../core/BoardState.h"
#include <vector>
#include <glm/glm.hpp>

class Highlighter {
public:
    Highlighter();
    ~Highlighter();

    bool init();

    
    
    
    
    void draw(const Shader& shader,
              Square selectedSq,
              const std::vector<Square>& legalSqs,
              Square lastMoveFrom,
              Square lastMoveTo,
              Square checkedKingSq) const;

private:
    GLuint vao_ = 0, vbo_ = 0;

    
    void drawSquare(const Shader& shader, Square sq, const glm::vec4& color,
                    float scale, float yOffset) const;
    static glm::vec3 squareCenter(Square sq);
};
