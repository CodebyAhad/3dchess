#include "InputHandler.h"
#include "../renderer/Board3D.h"
#include <glm/gtc/matrix_transform.hpp>

static constexpr float SIDEBAR_W_PIX = 220.0f;

InputHandler::InputHandler() = default;

void InputHandler::setCamera(const glm::mat4& view, const glm::mat4& proj,
                              int screenW, int screenH) {
    view_ = view; proj_ = proj;
    screenW_ = screenW; screenH_ = screenH;
}



glm::vec3 InputHandler::unproject(float ndcX, float ndcY) const {
    glm::vec4 rayClip(ndcX, ndcY, -1.0f, 1.0f);
    glm::vec4 rayEye = glm::inverse(proj_) * rayClip;
    rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0f, 0.0f);
    glm::vec3 rayWorld = glm::vec3(glm::inverse(view_) * rayEye);
    return glm::normalize(rayWorld);
}


Square InputHandler::screenToSquare(int mouseX, int mouseY) const {
    
    float ndcX =  (2.0f * mouseX) / screenW_ - 1.0f;
    float ndcY = -(2.0f * mouseY) / screenH_ + 1.0f;

    
    glm::vec3 origin = glm::vec3(glm::inverse(view_) * glm::vec4(0,0,0,1));

    
    glm::vec3 dir = unproject(ndcX, ndcY);

    
    if (std::abs(dir.y) < 1e-6f) return SQ_NONE;
    float t = (Board3D::BOARD_Y - origin.y) / dir.y;
    if (t < 0) return SQ_NONE;

    glm::vec3 hit = origin + t * dir;

    
    int file = (int)std::floor(4.0f - hit.x);
    int rank = (int)std::floor(hit.z + 4.0f);

    if (file < 0 || file > 7 || rank < 0 || rank > 7) return SQ_NONE;
    return makeSquare(file, rank);
}


Square InputHandler::processEvent(const SDL_Event& e, bool& isClick, bool& isRightClick) {
    isClick = false; isRightClick = false;

    if (e.type == SDL_MOUSEMOTION) {
        overUI_ = (e.motion.x > screenW_ - SIDEBAR_W_PIX);
        if (!overUI_) hoveredSq_ = screenToSquare(e.motion.x, e.motion.y);
        else hoveredSq_ = SQ_NONE;
        return hoveredSq_;
    }

    if (e.type == SDL_MOUSEBUTTONDOWN) {
        overUI_ = (e.button.x > screenW_ - SIDEBAR_W_PIX);
        if (e.button.button == SDL_BUTTON_LEFT)  isClick      = !overUI_;
        if (e.button.button == SDL_BUTTON_RIGHT) isRightClick = true;
        if (!overUI_) return screenToSquare(e.button.x, e.button.y);
    }

    return SQ_NONE;
}
