#pragma once
#include <SDL2/SDL.h>
#include <glm/glm.hpp>
#include "../core/BoardState.h"





class InputHandler {
public:
    InputHandler();

    
    void setCamera(const glm::mat4& view, const glm::mat4& proj,
                   int screenW, int screenH);

    
    
    Square processEvent(const SDL_Event& e, bool& isClick, bool& isRightClick);

    
    Square hoveredSquare() const { return hoveredSq_; }

    
    bool isOverUI() const { return overUI_; }

private:
    glm::mat4 view_{1}, proj_{1};
    int screenW_ = 800, screenH_ = 600;
    Square hoveredSq_ = SQ_NONE;
    bool overUI_ = false;

    Square screenToSquare(int mouseX, int mouseY) const;
    glm::vec3 unproject(float ndcX, float ndcY) const;
};
