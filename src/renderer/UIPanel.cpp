#include "UIPanel.h"
#include <glm/gtc/matrix_transform.hpp>
#include <array>
#include <cctype>


static const char* QUAD_VERT = R"(
#version 330 core
layout(location=0) in vec2 aPos;
uniform mat4 ortho;
uniform vec4 rect;
void main(){
    vec2 pos = rect.xy + aPos * rect.zw;
    gl_Position = ortho * vec4(pos, 0.0, 1.0);
}
)";

static const char* QUAD_FRAG = R"(
#version 330 core
out vec4 FragColor;
uniform vec4 color;
void main(){ FragColor = color; }
)";

namespace {
using GlyphRows = std::array<unsigned char, 7>;

bool glyphFor(char c, GlyphRows& out) {
    switch (c) {
        case 'A': out = {14,17,17,31,17,17,17}; return true;
        case 'B': out = {30,17,17,30,17,17,30}; return true;
        case 'C': out = {14,17,16,16,16,17,14}; return true;
        case 'D': out = {30,17,17,17,17,17,30}; return true;
        case 'E': out = {31,16,16,30,16,16,31}; return true;
        case 'F': out = {31,16,16,30,16,16,16}; return true;
        case 'G': out = {14,17,16,23,17,17,14}; return true;
        case 'H': out = {17,17,17,31,17,17,17}; return true;
        case 'I': out = {31,4,4,4,4,4,31}; return true;
        case 'J': out = {30,8,8,8,8,17,14}; return true;
        case 'K': out = {17,18,20,24,20,18,17}; return true;
        case 'L': out = {16,16,16,16,16,16,31}; return true;
        case 'M': out = {17,27,21,21,17,17,17}; return true;
        case 'N': out = {17,25,21,19,17,17,17}; return true;
        case 'O': out = {14,17,17,17,17,17,14}; return true;
        case 'P': out = {30,17,17,30,16,16,16}; return true;
        case 'Q': out = {14,17,17,17,21,18,13}; return true;
        case 'R': out = {30,17,17,30,20,18,17}; return true;
        case 'S': out = {15,16,16,14,1,1,30}; return true;
        case 'T': out = {31,4,4,4,4,4,4}; return true;
        case 'U': out = {17,17,17,17,17,17,14}; return true;
        case 'V': out = {17,17,17,17,17,10,4}; return true;
        case 'W': out = {17,17,17,21,21,21,10}; return true;
        case 'Y': out = {17,17,10,4,4,4,4}; return true;
        case 'Z': out = {31,1,2,4,8,16,31}; return true;
        
        case '0': out = {14,17,19,21,25,17,14}; return true;
        case '1': out = {4,12,4,4,4,4,14}; return true;
        case '2': out = {14,17,1,2,4,8,31}; return true;
        case '3': out = {30,1,1,14,1,1,30}; return true;
        case '4': out = {2,6,10,18,31,2,2}; return true;
        case '5': out = {31,16,16,30,1,1,30}; return true;
        case '6': out = {14,16,16,30,17,17,14}; return true;
        case '7': out = {31,1,2,4,8,8,8}; return true;
        case '8': out = {14,17,17,14,17,17,14}; return true;
        case '9': out = {14,17,17,15,1,1,14}; return true;

        
        case 'X': out = {17,10,4,4,4,10,17}; return true; 
        case '-': out = {0,0,0,31,0,0,0}; return true;    
        case '+': out = {0,4,4,31,4,4,0}; return true;    
        case '#': out = {10,10,31,10,31,10,10}; return true; 
        case '=': out = {0,0,31,0,31,0,0}; return true;    
        case '.': out = {0,0,0,0,0,12,12}; return true;
        default: break;
    }
    return false;
}

static float measureTextWidth(const std::string& text) {
    constexpr float glyphW = 5.0f;
    constexpr float advance = (glyphW + 1.0f) * 2.0f;
    float w = 0.0f;
    for (char ch : text) {
        if (ch == ' ') { w += advance; continue; }
        char u = static_cast<char>(std::toupper(static_cast<unsigned char>(ch)));
        GlyphRows rows{};
        if (!glyphFor(u, rows)) { w += advance; continue; }
        w += advance;
    }
    return w;
}

} 


UIPanel::UIPanel()  = default;
UIPanel::~UIPanel() {
    if (quadVao_) glDeleteVertexArrays(1, &quadVao_);
    if (quadVbo_) glDeleteBuffers(1, &quadVbo_);
}

bool UIPanel::init(int screenW, int screenH) {
    screenW_ = screenW; screenH_ = screenH;
    buildQuad();
    buildButtons();
    quadShader_.loadFromSource(QUAD_VERT, QUAD_FRAG);
    resize(screenW, screenH);
    return true;
}

void UIPanel::resize(int screenW, int screenH) {
    screenW_ = screenW; screenH_ = screenH;
    ortho_ = glm::ortho(0.0f, (float)screenW, 0.0f, (float)screenH, -1.0f, 1.0f);
    buildButtons();
}


void UIPanel::buildQuad() {
    
    float verts[] = { 0,0, 1,0, 1,1, 0,0, 1,1, 0,1 };
    glGenVertexArrays(1, &quadVao_);
    glGenBuffers(1, &quadVbo_);
    glBindVertexArray(quadVao_);
    glBindBuffer(GL_ARRAY_BUFFER, quadVbo_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
}

void UIPanel::buildButtons() {
    float sx = (float)screenW_ - SIDEBAR_W;
    float p  = SIDEBAR_PAD;
    float bw = SIDEBAR_W - 2*p;
    float bh = 34.0f;

    gameButtons_.clear();
    gameButtons_.push_back({ sx+p, p, bw, bh, "Undo Move", false, [this]{ if(onUndo_) onUndo_(); }});
    gameButtons_.push_back({ sx+p, p + bh + 6, bw, bh, "Resign", false, [this]{ if(onResign_) onResign_(); }});
    gameButtons_.push_back({ sx+p, p + (bh + 6) * 2, bw, bh, "Back to Menu", false, [this]{ if(onBackToMenu_) onBackToMenu_(); }});

    
    float mx = (float)screenW_/2 - 150.0f;
    float topY = (float)screenH_/2 + 56.0f;
    float step = 58.0f;
    menuButtons_.clear();
    menuButtons_.push_back({ mx, topY,             300, 44, "Easy",   false, [this]{ if(onDifficultyChanged_) onDifficultyChanged_(Level::EASY);  }});
    menuButtons_.push_back({ mx, topY - step,      300, 44, "Medium", false, [this]{ if(onDifficultyChanged_) onDifficultyChanged_(Level::MEDIUM); }});
    menuButtons_.push_back({ mx, topY - step * 2,  300, 44, "Hard",   false, [this]{ if(onDifficultyChanged_) onDifficultyChanged_(Level::HARD);   }});
    menuButtons_.push_back({ mx, topY - step * 4 - 54.0f,  300, 44, "Play Human",  false, [this]{ if(onHumanGame_) onHumanGame_(); }});
    menuButtons_.push_back({ mx, topY - step * 5 - 46.0f,  300, 36, "Exit",  false, [this]{ if(onExit_) onExit_(); }});

    colorButtons_.clear();
    const float colorY = topY - step * 3 - 34.0f;
    colorButtons_.push_back({ mx, colorY, 92, 36, "White", false, [this]{
        selectedMenuColor_ = MenuColorChoice::WHITE;
        if(onColorChanged_) onColorChanged_(MenuColorChoice::WHITE);
    }});
    colorButtons_.push_back({ mx + 104.0f, colorY, 92, 36, "Black", false, [this]{
        selectedMenuColor_ = MenuColorChoice::BLACK;
        if(onColorChanged_) onColorChanged_(MenuColorChoice::BLACK);
    }});
    colorButtons_.push_back({ mx + 208.0f, colorY, 92, 36, "Random", false, [this]{
        selectedMenuColor_ = MenuColorChoice::RANDOM;
        if(onColorChanged_) onColorChanged_(MenuColorChoice::RANDOM);
    }});

    
    float pw = 52.0f, ph = 52.0f;
    float px = (float)screenW_/2 - 2*(pw+6);
    float py = (float)screenH_/2 - ph/2;
    promotionButtons_.clear();
    const PieceType promoTypes[4] = {QUEEN, ROOK, BISHOP, KNIGHT};
    const std::string labels[4] = {"Q", "R", "B", "K"};
    for (int i = 0; i < 4; ++i) {
        PieceType choice = promoTypes[i];
        promotionButtons_.push_back({
            px + i*(pw+8), py, pw, ph, labels[i], false,
            [this, choice]{ if(onPromotion_) onPromotion_(choice); }
        });
    }
}


void UIPanel::drawRect(float x, float y, float w, float h, const glm::vec4& color) const {
    quadShader_.use();
    quadShader_.setMat4("ortho", ortho_);
    quadShader_.setVec4("rect",  glm::vec4(x, y, w, h));
    quadShader_.setVec4("color", color);
    glBindVertexArray(quadVao_);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void UIPanel::drawText(const std::string& text, float x, float y, float , const glm::vec4& ) const {
    constexpr float glyphW = 5.0f;
    constexpr float glyphH = 7.0f;
    constexpr float pixel = 2.0f;
    constexpr float advance = (glyphW + 1.0f) * pixel;
    float penX = x;

    for (char ch : text) {
        if (ch == ' ') {
            penX += advance;
            continue;
        }
        char u = static_cast<char>(std::toupper(static_cast<unsigned char>(ch)));
        GlyphRows rows{};
        if (!glyphFor(u, rows)) {
            penX += advance;
            continue;
        }
        for (int row = 0; row < static_cast<int>(glyphH); ++row) {
            for (int col = 0; col < static_cast<int>(glyphW); ++col) {
                if (rows[row] & (1 << (4 - col))) {
                    drawRect(penX + col * pixel, y - row * pixel, pixel, pixel, {0.92f, 0.94f, 0.98f, 0.95f});
                }
            }
        }
        penX += advance;
    }
}

void UIPanel::drawButton(const UIButton& btn, const glm::vec4& base, const glm::vec4& hover,
                         const std::string& label, bool centered) const {
    const glm::vec4 c = btn.hovered ? hover : base;
    drawRect(btn.x, btn.y, btn.w, btn.h, c);
    drawRect(btn.x, btn.y, btn.w, 1.5f, {0.4f,0.55f,0.8f,0.6f});
    const float textX = centered
        ? btn.x + btn.w * 0.5f - measureTextWidth(label) * 0.5f
        : btn.x + 10.0f;
    drawText(label, textX, btn.y + btn.h - 11.0f, 1.0f, {1,1,1,1});
}

void UIPanel::drawResultBanner(const std::string& text, const glm::vec4& color) const {
    const float w = measureTextWidth(text);
    drawRect((float)screenW_/2 - w/2 - 18.0f, (float)screenH_/2 + 10.0f,
             w + 36.0f, 42.0f, color);
    drawText(text, (float)screenW_/2 - w/2, (float)screenH_/2 + 38.0f, 1.0f, {1,1,1,1});
}


void UIPanel::drawSidebar(const MoveHistory& history,
                           const CaptureTracker& captures,
                           GameState state,
                           Level difficulty,
                           bool vsAI) const {
    float sx = (float)screenW_ - SIDEBAR_W;
    float sh = (float)screenH_;
    float p  = SIDEBAR_PAD;

    
    drawRect(sx, 0, SIDEBAR_W, sh, {0.08f, 0.08f, 0.10f, 0.92f});
    
    drawRect(sx, 0, 1.5f, sh, {0.3f, 0.3f, 0.35f, 1.0f});

    
    const float bottomPad = 110.0f; 
    float cy = sh - bottomPad;
    drawRect(sx+p, cy, SIDEBAR_W-2*p, 2.0f, {0.3f, 0.3f, 0.35f, 1.0f}); 
    
    auto& wCaps = captures.capturedBy(WHITE);
    for (size_t i = 0; i < wCaps.size(); ++i) {
        drawRect(sx + p + i*18.0f, cy + 6, 14.0f, 14.0f, {0.9f, 0.88f, 0.82f, 0.9f});
    }
    
    auto& bCaps = captures.capturedBy(BLACK);
    for (size_t i = 0; i < bCaps.size(); ++i) {
        drawRect(sx + p + i*18.0f, cy + 26, 14.0f, 14.0f, {0.18f, 0.14f, 0.12f, 0.9f});
    }

    
    const float historyGap = 10.0f;
    float hy = cy - historyGap;
    const auto& entries = history.entries();
    int startIdx = (int)entries.size() - 18;
    if (startIdx < 0) startIdx = 0;
    for (int i = startIdx; i < (int)entries.size(); ++i) {
        float rowY = hy - (i - startIdx) * 20.0f;
        glm::vec4 rowColor = (i % 2 == 0)
            ? glm::vec4(0.12f, 0.12f, 0.16f, 1.0f)
            : glm::vec4(0.10f, 0.10f, 0.13f, 1.0f);
        drawRect(sx+p, rowY - 16.0f, SIDEBAR_W-2*p, 18.0f, rowColor);
        
        if (i % 2 == 0) {
            drawRect(sx+p, rowY - 16.0f, 3.0f, 18.0f, {0.4f, 0.6f, 1.0f, 0.8f});
        }
        
        int moveNo = (i / 2) + 1;
        std::string prefix = (i % 2 == 0)
            ? (std::to_string(moveNo) + ". ")
            : (std::to_string(moveNo) + "... ");
        drawText(prefix + entries[i].san, sx + p + 8.0f, rowY - 4.0f, 1.0f, {1,1,1,1});
    }

    
    glm::vec4 statusColor = {0.2f, 0.7f, 0.3f, 0.9f};
    std::string statusText = "Your turn";
    if (state == GameState::AI_THINKING)  { statusColor = {0.7f, 0.5f, 0.1f, 0.9f}; statusText = "AI thinking..."; }
    if (state == GameState::CHECK)        { statusColor = {0.9f, 0.3f, 0.2f, 0.9f}; statusText = "Check!"; }
    if (state == GameState::CHECKMATE)    { statusColor = {0.9f, 0.1f, 0.1f, 0.9f}; statusText = "Checkmate"; }
    if (state == GameState::STALEMATE)    { statusColor = {0.5f, 0.5f, 0.5f, 0.9f}; statusText = "Draw"; }
    if (state == GameState::DRAW_50MOVE)  { statusColor = {0.5f, 0.5f, 0.5f, 0.9f}; statusText = "Draw"; }
    if (state == GameState::PROMOTION)    { statusColor = {0.3f, 0.6f, 0.9f, 0.9f}; statusText = "Choose piece"; }

    drawRect(sx+p, 10.0f, SIDEBAR_W-2*p, 28.0f, statusColor);
    drawText(statusText, sx + p + 8.0f, 29.0f, 1.0f, {1,1,1,1});

    
    for (size_t i = 0; i < gameButtons_.size(); ++i) {
        if (i == 2 && (currentVsAI_ || !showBackToMenu_)) continue;
        const auto& btn = gameButtons_[i];
        drawButton(btn, {0.18f, 0.22f, 0.32f, 1.0f}, {0.3f, 0.45f, 0.7f, 1.0f}, btn.label, false);
    }

    if (vsAI) {
        glm::vec4 diffColor =
            (difficulty == Level::EASY)   ? glm::vec4(0.2f, 0.75f, 0.3f, 0.9f) :
            (difficulty == Level::MEDIUM) ? glm::vec4(0.8f, 0.6f,  0.1f, 0.9f) :
                                            glm::vec4(0.9f, 0.25f, 0.2f, 0.9f);
        drawRect(sx + p, sh - 30.0f, 60.0f, 20.0f, diffColor);
        const std::string diffText =
            (difficulty == Level::EASY)   ? "EASY" :
            (difficulty == Level::MEDIUM) ? "MED"  : "HARD";
        drawText(diffText, sx + p + 6.0f, sh - 16.0f, 1.0f, {1,1,1,1});
    }
}


void UIPanel::draw(const MoveHistory& history,
                    const CaptureTracker& captures,
                    GameState state,
                    Level difficulty,
                    bool playerIsWhite,
                    bool vsAI,
                    Color sideToMove) const {
    
    const_cast<UIPanel*>(this)->inputState_ = state;
    const_cast<UIPanel*>(this)->currentVsAI_ = vsAI;
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    drawSidebar(history, captures, state, difficulty, vsAI);
    if (state == GameState::GAME_OVER && !resultMessage_.empty()) {
        drawResultBanner(resultMessage_, {0.08f, 0.06f, 0.06f, 0.88f});
    } else if (state == GameState::CHECKMATE) {
        const bool winnerIsWhite = (sideToMove == BLACK);
        const bool playerWon = (winnerIsWhite == playerIsWhite);
        const std::string text = vsAI ? (playerWon ? "YOU WIN" : "AI WINS")
                                      : (winnerIsWhite ? "WHITE WINS" : "BLACK WINS");
        drawResultBanner(text, {0.08f, 0.06f, 0.06f, 0.88f});
    } else if (state == GameState::STALEMATE || state == GameState::DRAW_50MOVE) {
        const std::string text = (state == GameState::STALEMATE) ? "DRAW BY STALEMATE" : "DRAW";
        drawResultBanner(text, {0.06f, 0.06f, 0.08f, 0.88f});
    }
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
}

void UIPanel::drawMenu() const {
    const_cast<UIPanel*>(this)->inputState_ = GameState::MENU;
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    
    drawRect(0, 0, (float)screenW_, (float)screenH_, {0.0f, 0.0f, 0.0f, 0.58f});
    
    drawRect((float)screenW_/2 - 196, (float)screenH_/2 - 324, 432, 562, {0.0f,0.0f,0.0f,0.35f});
    drawRect((float)screenW_/2 - 210, (float)screenH_/2 - 310, 420, 550, {0.071f,0.071f,0.071f,0.85f});
    drawRect((float)screenW_/2 - 208, (float)screenH_/2 + 188, 416, 50, {0.102f,0.102f,0.102f,0.95f});
    drawRect((float)screenW_/2 - 210, (float)screenH_/2 - 310, 420, 2, {0.20f,0.20f,0.20f,1.0f});
    drawRect((float)screenW_/2 - 210, (float)screenH_/2 + 238, 420, 2, {0.20f,0.20f,0.20f,1.0f});
    drawRect((float)screenW_/2 - 210, (float)screenH_/2 - 310, 2, 550, {0.20f,0.20f,0.20f,1.0f});
    drawRect((float)screenW_/2 + 208, (float)screenH_/2 - 310, 2, 550, {0.20f,0.20f,0.20f,1.0f});

    const float cx = (float)screenW_ / 2.0f;
    {
        const std::string title = "3D CHESS";
        drawText(title, cx - measureTextWidth(title) / 2.0f, (float)screenH_/2 + 218, 1.0f, {1,1,1,1});
    }
    {
        const std::string playAi = "PLAY AI";
        drawText(playAi, cx - measureTextWidth(playAi) / 2.0f, (float)screenH_/2 + 130, 1.0f, {0.70f,0.70f,0.70f,1});
    }

    for (size_t i = 0; i < menuButtons_.size(); ++i) {
        const auto& btn = menuButtons_[i];
        glm::vec4 c = btn.hovered
            ? glm::vec4(0.22f, 0.22f, 0.22f, 1.0f)
            : glm::vec4(0.14f, 0.14f, 0.14f, 0.96f);
        if (i == 3) {
            c = btn.hovered
                ? glm::vec4(0.30f, 0.69f, 0.31f, 1.0f)
                : glm::vec4(0.18f, 0.49f, 0.20f, 1.0f);
        } else if (i == 4) {
            c = btn.hovered
                ? glm::vec4(0.96f, 0.26f, 0.21f, 1.0f)
                : glm::vec4(0.78f, 0.16f, 0.16f, 1.0f);
        }
        drawButton(btn, c, c, btn.label, true);
    }

    {
        const std::string choose = "CHOOSE COLOR";
        const float chooseY = colorButtons_.empty() ? ((float)screenH_/2 - 122.0f) : (colorButtons_.front().y + 68.0f);
        drawText(choose, cx - measureTextWidth(choose) / 2.0f, chooseY, 1.0f, {0.70f,0.70f,0.70f,1});
    }
    for (const auto& btn : colorButtons_) {
        const bool selected =
            (selectedMenuColor_ == MenuColorChoice::WHITE && btn.label == "White") ||
            (selectedMenuColor_ == MenuColorChoice::BLACK && btn.label == "Black") ||
            (selectedMenuColor_ == MenuColorChoice::RANDOM && btn.label == "Random");
        glm::vec4 c = selected
            ? glm::vec4(0.10f, 0.46f, 0.82f, 1.0f)
            : glm::vec4(0.14f, 0.14f, 0.14f, 0.96f);
        const glm::vec4 hover = selected
            ? glm::vec4(0.13f, 0.59f, 0.95f, 1.0f)
            : glm::vec4(0.22f, 0.22f, 0.22f, 1.0f);
        drawButton(btn, c, hover, btn.label, true);
    }
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
}

void UIPanel::drawPromotionPicker(Color color) const {
    const_cast<UIPanel*>(this)->inputState_ = GameState::PROMOTION;
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    
    float pw = 4*(52+8)+12.0f, ph = 80.0f;
    float px = (float)screenW_/2 - pw/2;
    float py = (float)screenH_/2 - ph/2;
    drawRect(px-8, py-12, pw+16, ph+24, {0.05f,0.05f,0.08f,0.90f});
    glm::vec3 pieceClr = (color == WHITE)
        ? glm::vec3(0.92f, 0.88f, 0.82f)
        : glm::vec3(0.18f, 0.14f, 0.12f);

    for (auto& btn : promotionButtons_) {
        glm::vec4 c = btn.hovered
            ? glm::vec4(0.3f, 0.5f, 0.8f, 1.0f)
            : glm::vec4(pieceClr.r * 0.8f, pieceClr.g * 0.8f, pieceClr.b * 0.8f, 1.0f);
        drawRect(btn.x, btn.y, btn.w, btn.h, c);
        drawText(btn.label, btn.x + 20.0f, btn.y + 34.0f, 1.0f, {1,1,1,1});
    }
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
}


bool UIPanel::onMouseMove(float x, float y) {
    bool hit = false;
    for (auto& btn : gameButtons_)     { btn.hovered = false; }
    for (auto& btn : menuButtons_)     { btn.hovered = false; }
    for (auto& btn : colorButtons_)    { btn.hovered = false; }
    for (auto& btn : promotionButtons_){ btn.hovered = false; }

    if (inputState_ == GameState::MENU) {
        for (auto& btn : menuButtons_) { btn.hovered = (x>=btn.x && x<=btn.x+btn.w && y>=btn.y && y<=btn.y+btn.h); if(btn.hovered) hit=true; }
        for (auto& btn : colorButtons_) { btn.hovered = (x>=btn.x && x<=btn.x+btn.w && y>=btn.y && y<=btn.y+btn.h); if(btn.hovered) hit=true; }
        return hit;
    }
    if (inputState_ == GameState::PROMOTION) {
        for (auto& btn : promotionButtons_){ btn.hovered = (x>=btn.x && x<=btn.x+btn.w && y>=btn.y && y<=btn.y+btn.h); if(btn.hovered) hit=true; }
        return hit;
    }
    
    for (size_t i = 0; i < gameButtons_.size(); ++i) {
        if (i == 2 && (currentVsAI_ || !showBackToMenu_)) continue;
        auto& btn = gameButtons_[i];
        btn.hovered = (x>=btn.x && x<=btn.x+btn.w && y>=btn.y && y<=btn.y+btn.h);
        if(btn.hovered) hit=true;
    }
    return hit;
}

bool UIPanel::onMouseClick(float x, float y) {
    auto check = [&](std::vector<UIButton>& btns, bool includeBack) -> bool {
        for (size_t i = 0; i < btns.size(); ++i) {
            if (&btns == &gameButtons_ && i == 2 && (currentVsAI_ || !includeBack)) continue;
            auto& btn = btns[i];
            if (x>=btn.x && x<=btn.x+btn.w && y>=btn.y && y<=btn.y+btn.h) {
                if (btn.onClick) btn.onClick();
                return true;
            }
        }
        return false;
    };
    if (inputState_ == GameState::MENU) {
        if (check(colorButtons_, true)) return true;
        return check(menuButtons_, true);
    }
    if (inputState_ == GameState::PROMOTION) {
        if (check(promotionButtons_, true)) return true;

        
        
        if (!promotionButtons_.empty()) {
            const UIButton& first = promotionButtons_.front();
            const UIButton& last  = promotionButtons_.back();
            const float stripX0 = first.x;
            const float stripX1 = last.x + last.w;
            const float stripY0 = first.y;
            const float stripY1 = first.y + first.h;
            if (x >= stripX0 && x <= stripX1 && y >= stripY0 && y <= stripY1) {
                int bestIdx = 0;
                float bestDist = 1e9f;
                for (int i = 0; i < (int)promotionButtons_.size(); ++i) {
                    const float cx = promotionButtons_[i].x + promotionButtons_[i].w * 0.5f;
                    float d = std::abs(x - cx);
                    if (d < bestDist) { bestDist = d; bestIdx = i; }
                }
                if (promotionButtons_[bestIdx].onClick) promotionButtons_[bestIdx].onClick();
                return true;
            }
        }
        return false;
    }
    return check(gameButtons_, showBackToMenu_);
}
