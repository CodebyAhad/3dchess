#include "GameController.h"

GameController::GameController() : validator_(board_) {}

void GameController::newGame(const GameConfig& cfg) {
    config_ = cfg;
    board_.reset();
    history_.clear();
    captures_.clear();
    selectedSq_ = SQ_NONE;
    highlighted_.clear();
    setState(isPlayerTurn() ? GameState::PLAYER_TURN : GameState::AI_THINKING);
    if (!isPlayerTurn() && onAITurn) onAITurn(board_.toFEN());
}

void GameController::resetGame() { newGame(config_); }

void GameController::returnToMenu() {
    selectedSq_ = SQ_NONE;
    promotionFrom_ = promotionTo_ = SQ_NONE;
    highlighted_.clear();
    setState(GameState::MENU);
}

void GameController::endGame() {
    selectedSq_ = SQ_NONE;
    promotionFrom_ = promotionTo_ = SQ_NONE;
    highlighted_.clear();
    setState(GameState::GAME_OVER);
}


bool GameController::isPlayerTurn() const {
    if (!config_.vsAI) return true;
    return board_.sideToMove == config_.playerColor;
}

void GameController::setState(GameState s) {
    state_ = s;
    if (onStateChange) onStateChange(s);
}

Square GameController::checkedKingSquare() const {
    if (state_ != GameState::CHECK && state_ != GameState::CHECKMATE &&
        state_ != GameState::AI_THINKING) {
        return SQ_NONE;
    }
    if (validator_.isInCheck(board_.sideToMove)) {
        return board_.kingSquare(board_.sideToMove);
    }
    return SQ_NONE;
}


bool GameController::selectSquare(Square sq) {
    if (state_ != GameState::PLAYER_TURN && state_ != GameState::CHECK) return false;

    const Piece& clicked = board_.at(sq);

    
    if (selectedSq_ == SQ_NONE) {
        if (clicked.empty() || clicked.color != board_.sideToMove) return false;
        selectedSq_ = sq;
        highlighted_ = validator_.legalDestinations(sq);
        return false;
    }

    
    if (sq == selectedSq_) {
        selectedSq_ = SQ_NONE;
        highlighted_.clear();
        return false;
    }

    
    if (!clicked.empty() && clicked.color == board_.sideToMove) {
        selectedSq_ = sq;
        highlighted_ = validator_.legalDestinations(sq);
        return false;
    }

    
    if (!validator_.isLegal(selectedSq_, sq)) {
        selectedSq_ = SQ_NONE;
        highlighted_.clear();
        return false;
    }

    
    const Piece& mover = board_.at(selectedSq_);
    if (mover.type == PAWN) {
        int promRank = (mover.color == WHITE) ? 7 : 0;
        if (rankOf(sq) == promRank) {
            promotionFrom_ = selectedSq_;
            promotionTo_   = sq;
            selectedSq_    = SQ_NONE;
            highlighted_.clear();
            setState(GameState::PROMOTION);
            return false;
        }
    }

    
    Move m = {selectedSq_, sq, NONE};
    auto info = board_.applyMove(selectedSq_, sq);
    captures_.addCapture(info.capturedPiece);
    bool givesCheck = validator_.isInCheck(board_.sideToMove);
    bool givesMate  = validator_.isCheckmate();
    history_.push(m, buildSAN(m, info, givesCheck, givesMate), board_.toFEN(),
                  givesCheck, givesMate);
    selectedSq_ = SQ_NONE;
    highlighted_.clear();
    afterMove();
    return true;
}

void GameController::choosePromotion(PieceType pt) {
    if (state_ != GameState::PROMOTION) return;
    Move m = {promotionFrom_, promotionTo_, pt};
    auto info = board_.applyMove(promotionFrom_, promotionTo_, pt);
    captures_.addCapture(info.capturedPiece);
    bool givesCheck = validator_.isInCheck(board_.sideToMove);
    bool givesMate  = validator_.isCheckmate();
    history_.push(m, buildSAN(m, info, givesCheck, givesMate), board_.toFEN(),
                  givesCheck, givesMate);
    promotionFrom_ = promotionTo_ = SQ_NONE;
    afterMove();
}

void GameController::applyAIMove(const std::string& uci) {
    if (state_ != GameState::AI_THINKING) return;
    Move m = Move::fromUCI(uci);
    auto info = board_.applyMove(m.from, m.to, m.promotion);
    captures_.addCapture(info.capturedPiece);
    bool givesCheck = validator_.isInCheck(board_.sideToMove);
    bool givesMate  = validator_.isCheckmate();
    history_.push(m, buildSAN(m, info, givesCheck, givesMate), board_.toFEN(),
                  givesCheck, givesMate);
    afterMove();
}


void GameController::afterMove() {
    if (validator_.isCheckmate()) { setState(GameState::CHECKMATE); return; }
    if (validator_.isStalemate()) { setState(GameState::STALEMATE); return; }
    if (board_.halfMoveClock >= 100) { setState(GameState::DRAW_50MOVE); return; }

    if (validator_.isInCheck(board_.sideToMove)) {
        setState(isPlayerTurn() ? GameState::CHECK : GameState::AI_THINKING);
    } else {
        setState(isPlayerTurn() ? GameState::PLAYER_TURN : GameState::AI_THINKING);
    }

    if (state_ == GameState::AI_THINKING && onAITurn) onAITurn(board_.toFEN());
}

void GameController::undoLastMove() {
    if (history_.empty()) return;
    
    int undoCount = (config_.vsAI && history_.size() >= 2) ? 2 : 1;
    for (int i = 0; i < undoCount && !history_.empty(); ++i) {
        history_.pop();
    }

    
    if (!history_.empty()) board_.loadFEN(history_.entries().back().fenAfter);
    else board_.reset();

    
    captures_.clear();
    const int startCount[7] = {0, 8, 2, 2, 2, 1, 1}; 
    int curW[7] = {0}, curB[7] = {0};
    for (Square sq = 0; sq < 64; ++sq) {
        const Piece& p = board_.at(sq);
        if (p.empty()) continue;
        if (p.color == WHITE) curW[p.type]++; else curB[p.type]++;
    }
    
    for (int t = PAWN; t <= KING; ++t) {
        for (int k = curW[t]; k < startCount[t]; ++k) captures_.addCapture(Piece{(PieceType)t, WHITE});
        for (int k = curB[t]; k < startCount[t]; ++k) captures_.addCapture(Piece{(PieceType)t, BLACK});
    }

    selectedSq_ = SQ_NONE;
    highlighted_.clear();
    setState(isPlayerTurn() ? GameState::PLAYER_TURN : GameState::AI_THINKING);
}


static char pieceLetter(PieceType t) {
    switch (t) {
        case KNIGHT: return 'N';
        case BISHOP: return 'B';
        case ROOK:   return 'R';
        case QUEEN:  return 'Q';
        case KING:   return 'K';
        default:     return '\0';
    }
}


std::string GameController::buildSAN(const Move& m,
                                     const BoardState::MoveInfo& info,
                                     bool givesCheck,
                                     bool givesMate) const {
    const Piece moved = info.movedPiece;
    const bool isCapture = (!info.capturedPiece.empty() || info.isEnPassant);

    
    if (info.isCastling && moved.type == KING) {
        std::string s = (fileOf(info.to) > fileOf(info.from)) ? "O-O" : "O-O-O";
        if (givesMate) s += "#";
        else if (givesCheck) s += "+";
        return s;
    }

    std::string san;
    if (moved.type == PAWN) {
        if (isCapture) {
            san += char('a' + fileOf(info.from));
            san += 'x';
        }
        san += BoardState::squareName(info.to);
    } else {
        san += pieceLetter(moved.type);
        if (isCapture) san += 'x';
        san += BoardState::squareName(info.to);
    }

    if (m.promotion != NONE) {
        san += '=';
        san += pieceLetter(m.promotion);
    }

    if (givesMate) san += "#";
    else if (givesCheck) san += "+";
    return san;
}
