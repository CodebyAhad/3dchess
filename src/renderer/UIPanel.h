#pragma once
#include "Shader.h"
#include "../core/MoveHistory.h"
#include "../core/CaptureTracker.h"
#include "../core/GameController.h"
#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <functional>












struct UIButton {
    float x, y, w, h;
    std::string label;
    bool  hovered = false;
    std::function<void()> onClick;
};

enum class MenuColorChoice {
    WHITE,
    BLACK,
    RANDOM,
};

class UIPanel {
public:
    UIPanel();
    ~UIPanel();

    bool init(int screenW, int screenH);
    void resize(int screenW, int screenH);

    
    void draw(const MoveHistory& history,
              const CaptureTracker& captures,
              GameState state,
              Level difficulty,
              bool playerIsWhite,
              bool vsAI,
              Color sideToMove) const;

    
    void drawMenu() const;

    
    void drawPromotionPicker(Color color) const;

    
    bool onMouseMove(float x, float y);
    bool onMouseClick(float x, float y);
    void setInputState(GameState s) { inputState_ = s; }
    void setShowBackToMenu(bool show) { showBackToMenu_ = show; }
    void setResultMessage(const std::string& message) { resultMessage_ = message; }

    
    void setOnDifficultyChanged(std::function<void(Level)> cb) { onDifficultyChanged_ = cb; }
    void setOnColorChanged     (std::function<void(MenuColorChoice)> cb) { onColorChanged_ = cb; }
    void setOnNewGame          (std::function<void()>      cb) { onNewGame_           = cb; }
    void setOnHumanGame        (std::function<void()>      cb) { onHumanGame_         = cb; }
    void setOnExit             (std::function<void()>      cb) { onExit_              = cb; }
    void setOnUndo             (std::function<void()>      cb) { onUndo_              = cb; }
    void setOnResign           (std::function<void()>      cb) { onResign_            = cb; }
    void setOnBackToMenu       (std::function<void()>      cb) { onBackToMenu_        = cb; }
    void setOnPromotion        (std::function<void(PieceType)> cb) { onPromotion_     = cb; }

private:
    int screenW_ = 800, screenH_ = 600;

    
    GameState inputState_ = GameState::MENU;
    bool showBackToMenu_ = false;
    bool currentVsAI_ = true;
    std::string resultMessage_;

    
    static constexpr float SIDEBAR_W     = 220.0f;
    static constexpr float SIDEBAR_PAD   = 12.0f;

    
    GLuint quadVao_ = 0, quadVbo_ = 0;
    Shader quadShader_;

    std::vector<UIButton> menuButtons_;
    std::vector<UIButton> colorButtons_;
    std::vector<UIButton> gameButtons_;
    std::vector<UIButton> promotionButtons_;
    MenuColorChoice selectedMenuColor_ = MenuColorChoice::RANDOM;

    
    glm::mat4 ortho_{1.0f};

    void buildQuad();
    void buildButtons();

    
    void drawRect(float x, float y, float w, float h, const glm::vec4& color) const;
    void drawText(const std::string& text, float x, float y, float scale, const glm::vec4& color) const;
    void drawButton(const UIButton& btn, const glm::vec4& base, const glm::vec4& hover,
                    const std::string& label, bool centered) const;
    void drawResultBanner(const std::string& text, const glm::vec4& color) const;

    
    void drawSidebar(const MoveHistory& history, const CaptureTracker& captures,
                     GameState state, Level difficulty, bool vsAI) const;

    std::function<void(Level)>      onDifficultyChanged_;
    std::function<void(MenuColorChoice)> onColorChanged_;
    std::function<void()>           onNewGame_;
    std::function<void()>           onHumanGame_;
    std::function<void()>           onExit_;
    std::function<void()>           onUndo_;
    std::function<void()>           onResign_;
    std::function<void()>           onBackToMenu_;
    std::function<void(PieceType)>  onPromotion_;
};
