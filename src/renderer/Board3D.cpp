#include "Board3D.h"
#include <vector>





glm::vec3 Board3D::squareCenter(int file, int rank) {
    return glm::vec3(
        (3.5f - file) * SQUARE_SIZE,
        BOARD_Y,
        (rank - 3.5f) * SQUARE_SIZE
    );
}

Board3D::Board3D()  = default;
Board3D::~Board3D() {
    if (vao_) glDeleteVertexArrays(1, &vao_);
    if (vbo_) glDeleteBuffers(1, &vbo_);
    if (ebo_) glDeleteBuffers(1, &ebo_);
}

bool Board3D::init() {
    buildMesh();
    return vao_ != 0;
}




struct BoardVertex {
    float x, y, z;
    float nx, ny, nz;
    float u, v;
    float colorId;   
};

void Board3D::buildMesh() {
    std::vector<BoardVertex> verts;
    std::vector<GLuint>      inds;

    
    for (int rank = 0; rank < 8; ++rank) {
        for (int file = 0; file < 8; ++file) {
            float x0 = (3.0f - file) * SQUARE_SIZE;
            float x1 = x0 + SQUARE_SIZE;
            float z0 = (rank - 4.0f) * SQUARE_SIZE;
            float z1 = z0 + SQUARE_SIZE;
            float cid = ((file + rank) % 2 == 0) ? 0.0f : 1.0f;

            GLuint base = (GLuint)verts.size();

            
            verts.push_back({x0, BOARD_Y, z0,  0,1,0,  0,0, cid});
            verts.push_back({x1, BOARD_Y, z0,  0,1,0,  1,0, cid});
            verts.push_back({x1, BOARD_Y, z1,  0,1,0,  1,1, cid});
            verts.push_back({x0, BOARD_Y, z1,  0,1,0,  0,1, cid});

            
            inds.insert(inds.end(), {base, base+2, base+1, base, base+3, base+2});
        }
    }

    
    const float border = 0.5f;
    const float thickness = 0.2f;
    float bmin = -4.0f - border, bmax = 4.0f + border;
    
    GLuint base = (GLuint)verts.size();
    float by = BOARD_Y - thickness;
    verts.push_back({bmin, by, bmin, 0,1,0, 0,0, 0.5f});
    verts.push_back({bmax, by, bmin, 0,1,0, 1,0, 0.5f});
    verts.push_back({bmax, by, bmax, 0,1,0, 1,1, 0.5f});
    verts.push_back({bmin, by, bmax, 0,1,0, 0,1, 0.5f});
    inds.insert(inds.end(), {base, base+2, base+1, base, base+3, base+2});

    indexCount_ = (int)inds.size();

    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);
    glGenBuffers(1, &ebo_);

    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(BoardVertex), verts.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, inds.size() * sizeof(GLuint), inds.data(), GL_STATIC_DRAW);

    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(BoardVertex), (void*)0);
    glEnableVertexAttribArray(0);
    
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(BoardVertex), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);
    
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(BoardVertex), (void*)(6*sizeof(float)));
    glEnableVertexAttribArray(2);
    
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(BoardVertex), (void*)(8*sizeof(float)));
    glEnableVertexAttribArray(3);

    glBindVertexArray(0);
}

void Board3D::draw(const Shader& shader) const {
    shader.use();
    glBindVertexArray(vao_);
    glDrawElements(GL_TRIANGLES, indexCount_, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}
