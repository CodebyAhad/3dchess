#pragma once
#include "Shader.h"
#include "../core/BoardState.h"
#include <glm/glm.hpp>
#include <vector>
#include <string>
#include <unordered_map>

struct MeshData {
    GLuint vao = 0, vbo = 0, ebo = 0;
    int    indexCount = 0;
};

class PieceMesh {
public:
    PieceMesh();
    ~PieceMesh();

    
    
    bool loadAll(const std::string& modelDir);

    
    void drawPiece(PieceType type, Color color,
                   const glm::vec3& position,
                   const Shader& shader) const;

    
    void drawAll(const BoardState& board, const Shader& shader) const;

private:
    std::unordered_map<int, MeshData> meshes_;   

    bool loadOBJ(const std::string& path, MeshData& out);
    void drawMesh(const MeshData& mesh) const;

    static glm::vec3 pieceColor(Color c);
    static float     pieceScale(PieceType t);
};
