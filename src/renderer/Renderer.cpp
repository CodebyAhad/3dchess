#include "Renderer.h"
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
#include <iostream>


static const char* BOARD_VERT = R"(
#version 330 core
layout(location=0) in vec3 aPos;
layout(location=1) in vec3 aNorm;
layout(location=2) in vec2 aUV;
layout(location=3) in float aColorId;
out vec3 fragPos;
out vec3 normal;
out float colorId;
uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;
void main(){
    vec4 world = model * vec4(aPos, 1.0);
    fragPos = world.xyz;
    normal = mat3(transpose(inverse(model))) * aNorm;
    colorId = aColorId;
    gl_Position = proj * view * world;
}
)";

static const char* BOARD_FRAG = R"(
#version 330 core
in vec3 fragPos;
in vec3 normal;
in float colorId;
out vec4 FragColor;
uniform vec3 lightDir;
uniform vec3 lightColor;
uniform vec3 ambientColor;
void main(){
    vec3 lightSquare = vec3(0.90, 0.82, 0.70);
    vec3 darkSquare  = vec3(0.32, 0.20, 0.12);
    vec3 borderColor = vec3(0.18, 0.12, 0.08);
    vec3 baseColor = (colorId < 0.25) ? lightSquare :
                     (colorId > 0.75) ? darkSquare  : borderColor;
    vec3 norm = normalize(normal);
    float diff = max(dot(norm, normalize(lightDir)), 0.0);
    vec3 color = ambientColor * baseColor + diff * lightColor * baseColor;
    FragColor = vec4(color, 1.0);
}
)";

static const char* PIECE_VERT = R"(
#version 330 core
layout(location=0) in vec3 aPos;
layout(location=1) in vec3 aNorm;
out vec3 fragPos;
out vec3 normal;
uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;
void main(){
    vec4 world = model * vec4(aPos, 1.0);
    fragPos = world.xyz;
    normal = mat3(transpose(inverse(model))) * aNorm;
    gl_Position = proj * view * world;
}
)";

static const char* PIECE_FRAG = R"(
#version 330 core
in vec3 fragPos;
in vec3 normal;
out vec4 FragColor;
uniform vec3 pieceColor;
uniform vec3 lightDir;
uniform vec3 lightColor;
uniform vec3 ambientColor;
uniform vec3 viewPos;
void main(){
    vec3 norm = normalize(normal);
    vec3 ld   = normalize(lightDir);
    float diff = max(dot(norm, ld), 0.0);
    vec3 viewDir = normalize(viewPos - fragPos);
    vec3 halfDir = normalize(ld + viewDir);
    float spec = pow(max(dot(norm, halfDir), 0.0), 64.0);
    vec3 color = ambientColor * pieceColor
               + diff  * lightColor * pieceColor
               + spec  * lightColor * 0.4;
    FragColor = vec4(color, 1.0);
}
)";

static const char* HIGHLIGHT_VERT = R"(
#version 330 core
layout(location=0) in vec3 aPos;
uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;
void main(){
    gl_Position = proj * view * model * vec4(aPos, 1.0);
}
)";

static const char* HIGHLIGHT_FRAG = R"(
#version 330 core
out vec4 FragColor;
uniform vec4 highlightColor;
void main(){ FragColor = highlightColor; }
)";


Renderer::Renderer()  = default;
Renderer::~Renderer() = default;

bool Renderer::init(int screenW, int screenH, const std::string& assetDir) {
    screenW_ = screenW; screenH_ = screenH;

    if (!board3d_.init())    { std::cerr << "[Renderer] Board3D init failed\n";    return false; }
    if (!highlighter_.init()){ std::cerr << "[Renderer] Highlighter init failed\n"; return false; }
    if (!buildShaders())     { std::cerr << "[Renderer] Shader build failed\n";    return false; }

    pieces_.loadAll(assetDir + "/models");   
    ui_.init(screenW, screenH);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    resetCamera();
    return true;
}

void Renderer::resize(int screenW, int screenH) {
    screenW_ = screenW; screenH_ = screenH;
    glViewport(0, 0, screenW, screenH);
    ui_.resize(screenW, screenH);
    updateCamera();
}

bool Renderer::buildShaders() {
    return boardShader_.loadFromSource(BOARD_VERT, BOARD_FRAG)
        && pieceShader_.loadFromSource(PIECE_VERT, PIECE_FRAG)
        && highlightShader_.loadFromSource(HIGHLIGHT_VERT, HIGHLIGHT_FRAG);
}


void Renderer::updateCamera() {
    float yawRad   = glm::radians(camYaw_);
    float pitchRad = glm::radians(camPitch_);
    glm::vec3 eye = glm::vec3(
        camDist_ * std::cos(pitchRad) * std::sin(yawRad),
        camDist_ * std::sin(pitchRad),
        camDist_ * std::cos(pitchRad) * std::cos(yawRad)
    );
    view_ = glm::lookAt(eye, glm::vec3(0), glm::vec3(0,1,0));
    proj_ = glm::perspective(glm::radians(45.0f),
                         (float)screenW_ / (float)screenH_,
                         0.1f, 200.0f);
}

void Renderer::orbitCamera(float dy, float dp) {
    camYaw_   += dy;
    camPitch_ = glm::clamp(camPitch_ + dp, 15.0f, 85.0f);
    updateCamera();
}

void Renderer::zoomCamera(float delta) {
    camDist_ = glm::clamp(camDist_ - delta, 5.0f, 30.0f);
    updateCamera();
}

void Renderer::resetCamera() {
    
    
    
    camYaw_ = 180.0f;
    camPitch_ = 78.0f;
    camDist_ = 17.0f;
    updateCamera();
}

void Renderer::facePlayerColor(Color playerColor) {
    camYaw_ = (playerColor == WHITE) ? 180.0f : 0.0f;
    camPitch_ = 78.0f;
    camDist_ = 17.0f;
    aiOrientationLocked_ = true;
    lastAutoRotateVsAI_ = true;
    autoRotateInitialized_ = false;
    updateCamera();
}

void Renderer::orientForCurrentTurn(const GameController& ctrl) {
    if (ctrl.config().vsAI) {
        if (!lastAutoRotateVsAI_ && !aiOrientationLocked_) {
            facePlayerColor(ctrl.config().playerColor);
        }
        lastAutoRotateVsAI_ = true;
        autoRotateInitialized_ = false;
        return;
    }

    aiOrientationLocked_ = false;
    const Color side = ctrl.board().sideToMove;
    if (!autoRotateInitialized_ || lastAutoRotateVsAI_ || side != lastAutoRotateSide_) {
        camYaw_ = (side == WHITE) ? 180.0f : 0.0f;
        lastAutoRotateSide_ = side;
        autoRotateInitialized_ = true;
        updateCamera();
    }
    lastAutoRotateVsAI_ = false;
}


void Renderer::draw3DScene(const GameController& ctrl) {
    glm::mat4 identity(1.0f);
    glm::vec3 lightDir   = glm::normalize(glm::vec3(2.0f, 5.0f, 3.0f));
    glm::vec3 lightColor = glm::vec3(1.0f, 0.97f, 0.90f);
    glm::vec3 ambient    = glm::vec3(0.35f, 0.32f, 0.30f);
    glm::vec3 viewPos    = glm::vec3(glm::inverse(view_) * glm::vec4(0,0,0,1));

    
    boardShader_.use();
    boardShader_.setMat4("model",        identity);
    boardShader_.setMat4("view",         view_);
    boardShader_.setMat4("proj",         proj_);
    boardShader_.setVec3("lightDir",     lightDir);
    boardShader_.setVec3("lightColor",   lightColor);
    boardShader_.setVec3("ambientColor", ambient);
    board3d_.draw(boardShader_);

    
    highlightShader_.use();
    highlightShader_.setMat4("view", view_);
    highlightShader_.setMat4("proj", proj_);

    
    const auto& hist = ctrl.history().entries();
    if (!hist.empty()) {
        lastMoveFrom_ = hist.back().move.from;
        lastMoveTo_   = hist.back().move.to;
    }

    highlighter_.draw(highlightShader_,
                      ctrl.selectedSquare(),
                      ctrl.highlightedSquares(),
                      lastMoveFrom_,
                      lastMoveTo_,
                      ctrl.checkedKingSquare());

    
    pieceShader_.use();
    pieceShader_.setMat4("view",         view_);
    pieceShader_.setMat4("proj",         proj_);
    pieceShader_.setVec3("lightDir",     lightDir);
    pieceShader_.setVec3("lightColor",   lightColor);
    pieceShader_.setVec3("ambientColor", ambient);
    pieceShader_.setVec3("viewPos",      viewPos);
    pieces_.drawAll(ctrl.board(), pieceShader_);
}


void Renderer::drawFrame(const GameController& ctrl) {
    glClearColor(0.06f, 0.06f, 0.09f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    GameState st = ctrl.state();

    if (st == GameState::MENU) {
        draw3DScene(ctrl);
        ui_.drawMenu();
        return;
    }

    orientForCurrentTurn(ctrl);
    draw3DScene(ctrl);

    ui_.draw(ctrl.history(), ctrl.captures(),
             st, ctrl.config().difficulty,
             ctrl.config().playerColor == WHITE,
             ctrl.config().vsAI,
             ctrl.board().sideToMove);

    if (st == GameState::PROMOTION)
        ui_.drawPromotionPicker(ctrl.board().sideToMove);
}
