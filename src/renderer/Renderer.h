#pragma once
#include "Shader.h"
#include "Board3D.h"
#include "PieceMesh.h"
#include "Highlighter.h"
#include "UIPanel.h"
#include "../core/GameController.h"
#include <glm/glm.hpp>
#include <string>

class Renderer {
public:
    Renderer();
    ~Renderer();

    bool init(int screenW, int screenH, const std::string& assetDir);
    void resize(int screenW, int screenH);

    
    void drawFrame(const GameController& ctrl);

    
    void orbitCamera(float deltaYaw, float deltaPitch);
    void zoomCamera(float delta);
    void resetCamera();
    void facePlayerColor(Color playerColor);

    UIPanel& ui() { return ui_; }
    const glm::mat4& view() const { return view_; }
    const glm::mat4& proj() const { return proj_; }
    int screenW() const { return screenW_; }
    int screenH() const { return screenH_; }

private:
    int screenW_ = 800, screenH_ = 600;

    
    Board3D    board3d_;
    PieceMesh  pieces_;
    Highlighter highlighter_;
    UIPanel    ui_;

    
    Shader boardShader_;
    Shader pieceShader_;
    Shader highlightShader_;

    
    float camYaw_ = 45.0f;
    float camPitch_ = 60.0f;
    float camDist_ = 25.0f;
    glm::mat4 view_{1}, proj_{1};
    Color lastAutoRotateSide_ = WHITE;
    bool lastAutoRotateVsAI_ = true;
    bool autoRotateInitialized_ = false;
    bool aiOrientationLocked_ = false;

    void updateCamera();
    void orientForCurrentTurn(const GameController& ctrl);
    void draw3DScene(const GameController& ctrl);

    
    bool buildShaders();

    
    Square lastMoveFrom_ = SQ_NONE;
    Square lastMoveTo_   = SQ_NONE;
};
