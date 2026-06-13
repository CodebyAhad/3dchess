#include "Highlighter.h"
#include "Board3D.h"
#include <glm/gtc/matrix_transform.hpp>

Highlighter::Highlighter()  = default;
Highlighter::~Highlighter() {
    if (vao_) glDeleteVertexArrays(1, &vao_);
    if (vbo_) glDeleteBuffers(1, &vbo_);
}

bool Highlighter::init() {
    
    static const float quad[] = {
        -0.5f, 0.005f, -0.5f,
         0.5f, 0.005f, -0.5f,
         0.5f, 0.005f,  0.5f,
        -0.5f, 0.005f, -0.5f,
         0.5f, 0.005f,  0.5f,
        -0.5f, 0.005f,  0.5f,
    };
    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);
    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
    return true;
}

glm::vec3 Highlighter::squareCenter(Square sq) {
    return Board3D::squareCenter(fileOf(sq), rankOf(sq));
}

void Highlighter::drawSquare(const Shader& shader, Square sq, const glm::vec4& color,
                             float scale, float yOffset) const {
    if (sq == SQ_NONE) return;
    glm::mat4 model = glm::mat4(1.0f);
    glm::vec3 c = squareCenter(sq);
    c.y += yOffset;
    model = glm::translate(model, c);
    model = glm::scale(model, glm::vec3(Board3D::SQUARE_SIZE * scale));
    shader.setMat4("model", model);
    shader.setVec4("highlightColor", color);
    glBindVertexArray(vao_);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void Highlighter::draw(const Shader& shader,
                        Square selectedSq,
                        const std::vector<Square>& legalSqs,
                        Square lastMoveFrom,
                        Square lastMoveTo,
                        Square checkedKingSq) const {
    shader.use();
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE);
    glDepthMask(GL_FALSE);

    
    drawSquare(shader, lastMoveFrom, {0.9f, 0.7f, 0.1f, 0.35f}, 1.02f, 0.006f);
    drawSquare(shader, lastMoveTo,   {0.9f, 0.7f, 0.1f, 0.35f}, 1.02f, 0.006f);

    
    drawSquare(shader, checkedKingSq, {1.0f, 0.05f, 0.04f, 0.72f}, 1.08f, 0.012f);

    
    for (Square sq : legalSqs)
        drawSquare(shader, sq, {0.20f, 0.95f, 0.35f, 0.80f}, 0.35f, 0.010f);

    
    drawSquare(shader, selectedSq, {1.0f, 0.85f, 0.1f, 0.55f}, 1.04f, 0.008f);

    glDepthMask(GL_TRUE);
    glEnable(GL_CULL_FACE);
    glDisable(GL_BLEND);
}
