#pragma once
#include "BoardState.h"
#include "MoveValidator.h"
#include "MoveHistory.h"
#include "CaptureTracker.h"
#include <functional>
#include <string>

enum class GameState {
    MENU,
    PLAYING,
    PLAYER_TURN,    
    AI_THINKING,    
    PROMOTION,      
    CHECK,
    CHECKMATE,
    STALEMATE,
    DRAW_50MOVE,
    GAME_OVER,
};

enum class Level { EASY, MEDIUM, HARD };

struct GameConfig {
    Level       difficulty  = Level::MEDIUM;
    Color       playerColor = WHITE;
    bool        vsAI        = true;
};

class GameController {
public:
    GameController();

    void newGame(const GameConfig& cfg);
    void resetGame();
    void returnToMenu();
    void endGame();

    
    
    bool selectSquare(Square sq);

    
    void applyAIMove(const std::string& uci);

    
    void choosePromotion(PieceType pt);

    
    void undoLastMove();

    
    const BoardState&    board()    const { return board_; }
    BoardState&          board()          { return board_; }
    GameState            state()    const { return state_; }
    const GameConfig&    config()   const { return config_; }
    MoveHistory& history() { return history_; }
    const MoveHistory& history()  const { return history_; }
    CaptureTracker& captures() { return captures_; }
    const CaptureTracker& captures() const { return captures_; }

    Square selectedSquare()  const { return selectedSq_; }
    Square promotionFrom()   const { return promotionFrom_; }
    Square promotionTo()     const { return promotionTo_; }

    std::vector<Square>  highlightedSquares() const { return highlighted_; }
    Square checkedKingSquare() const;

    
    std::function<void(const std::string& fen)> onAITurn;
    std::function<void(GameState)>              onStateChange;

private:
    BoardState     board_;
    MoveValidator  validator_;
    MoveHistory    history_;
    CaptureTracker captures_;
    GameConfig     config_;
    GameState      state_   = GameState::MENU;
    Square         selectedSq_   = SQ_NONE;
    Square         promotionFrom_ = SQ_NONE;
    Square         promotionTo_   = SQ_NONE;
    std::vector<Square> highlighted_;

    void setState(GameState s);
    void afterMove();                     
    std::string buildSAN(const Move& m,
                         const BoardState::MoveInfo& info,
                         bool givesCheck,
                         bool givesMate) const;
    bool isPlayerTurn() const;
};
