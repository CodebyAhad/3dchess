#include "PieceMesh.h"
#include "../renderer/Board3D.h"


#define TINYOBJLOADER_IMPLEMENTATION
#include <tinyobj/tiny_obj_loader.h>

#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <iostream>

PieceMesh::PieceMesh()  = default;
PieceMesh::~PieceMesh() {
    for (auto& [k, m] : meshes_) {
        if (m.vao) glDeleteVertexArrays(1, &m.vao);
        if (m.vbo) glDeleteBuffers(1, &m.vbo);
        if (m.ebo) glDeleteBuffers(1, &m.ebo);
    }
}


bool PieceMesh::loadAll(const std::string& dir) {
    struct Entry { PieceType type; std::string file; };
    const Entry entries[] = {
        {PAWN,   "pawn.obj"},
        {KNIGHT, "knight.obj"},
        {BISHOP, "bishop.obj"},
        {ROOK,   "rook.obj"},
        {QUEEN,  "queen.obj"},
        {KING,   "king.obj"},
    };
    bool ok = true;
    for (auto& e : entries) {
        MeshData mesh;
        std::string path = dir + "/" + e.file;
        if (!loadOBJ(path, mesh)) {
            std::cerr << "[PieceMesh] Failed to load: " << path << "\n";
            
            
            ok = false;
        }
        meshes_[e.type] = mesh;
    }
    return ok;
}


bool PieceMesh::loadOBJ(const std::string& path, MeshData& out) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    bool loaded = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str());
    if (!loaded) { std::cerr << err << "\n"; return false; }

    if (attrib.vertices.empty()) return false;

    
    
    std::vector<glm::vec3> transformedVerts(attrib.vertices.size() / 3);
    glm::vec3 minV(1e9f, 1e9f, 1e9f);
    glm::vec3 maxV(-1e9f, -1e9f, -1e9f);
    for (size_t i = 0; i < transformedVerts.size(); ++i) {
        const float x = attrib.vertices[3 * i + 0];
        const float y = attrib.vertices[3 * i + 1];
        const float z = attrib.vertices[3 * i + 2];
        glm::vec3 p(x, z, -y);
        transformedVerts[i] = p;
        minV.x = std::min(minV.x, p.x);
        minV.y = std::min(minV.y, p.y);
        minV.z = std::min(minV.z, p.z);
        maxV.x = std::max(maxV.x, p.x);
        maxV.y = std::max(maxV.y, p.y);
        maxV.z = std::max(maxV.z, p.z);
    }
    const glm::vec3 center((minV.x + maxV.x) * 0.5f, minV.y, (minV.z + maxV.z) * 0.5f);

    
    std::vector<float> verts;
    std::vector<GLuint> inds;

    for (auto& shape : shapes) {
        for (auto& idx : shape.mesh.indices) {
            
            glm::vec3 p = transformedVerts[idx.vertex_index] - center;
            verts.push_back(p.x);
            verts.push_back(p.y);
            verts.push_back(p.z);
            
            if (idx.normal_index >= 0) {
                const float nx = attrib.normals[3 * idx.normal_index + 0];
                const float ny = attrib.normals[3 * idx.normal_index + 1];
                const float nz = attrib.normals[3 * idx.normal_index + 2];
                glm::vec3 n(nx, nz, -ny);
                verts.push_back(n.x);
                verts.push_back(n.y);
                verts.push_back(n.z);
            } else {
                verts.push_back(0); verts.push_back(1); verts.push_back(0);
            }
            inds.push_back((GLuint)inds.size());
        }
    }

    out.indexCount = (int)inds.size();

    glGenVertexArrays(1, &out.vao);
    glGenBuffers(1, &out.vbo);
    glGenBuffers(1, &out.ebo);

    glBindVertexArray(out.vao);
    glBindBuffer(GL_ARRAY_BUFFER, out.vbo);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), verts.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, out.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, inds.size() * sizeof(GLuint), inds.data(), GL_STATIC_DRAW);

    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
    return true;
}


glm::vec3 PieceMesh::pieceColor(Color c) {
    return (c == WHITE)
        ? glm::vec3(0.92f, 0.88f, 0.82f)   
        : glm::vec3(0.18f, 0.14f, 0.12f);   
}

float PieceMesh::pieceScale(PieceType t) {
    switch (t) {
        case PAWN:   return 0.015f;
        case KNIGHT: return 0.015f;
        case BISHOP: return 0.015f;
        case ROOK:   return 0.015f;
        case QUEEN:  return 0.015f;
        case KING:   return 0.015f;
        default:     return 0.015f;
    }
}

void PieceMesh::drawMesh(const MeshData& mesh) const {
    if (!mesh.vao) return;
    glBindVertexArray(mesh.vao);
    glDrawElements(GL_TRIANGLES, mesh.indexCount, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}

void PieceMesh::drawPiece(PieceType type, Color color,
                           const glm::vec3& position,
                           const Shader& shader) const {
    auto it = meshes_.find(type);
    if (it == meshes_.end()) return;

    float s = pieceScale(type);
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    model = glm::scale(model, glm::vec3(s));

    shader.setMat4("model", model);
    shader.setVec3("pieceColor", pieceColor(color));
    drawMesh(it->second);
}

void PieceMesh::drawAll(const BoardState& board, const Shader& shader) const {
    for (Square sq = 0; sq < 64; ++sq) {
        const Piece& p = board.at(sq);
        if (p.empty()) continue;
        glm::vec3 pos = Board3D::squareCenter(fileOf(sq), rankOf(sq));
        pos.y = Board3D::BOARD_Y + 0.01f;
        drawPiece(p.type, p.color, pos, shader);
    }
}
