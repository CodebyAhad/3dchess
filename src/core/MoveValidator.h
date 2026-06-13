#pragma once
#include "BoardState.h"
#include <vector>

struct Move {
    Square    from, to;
    PieceType promotion = NONE;   

    std::string toUCI() const;     
    static Move fromUCI(const std::string& uci);
};

class MoveValidator {
public:
    explicit MoveValidator(BoardState& board);

    
    std::vector<Move> pseudoLegalMoves() const;

    
    std::vector<Move> legalMoves();

    
    bool isInCheck(Color c) const;

    
    bool isCheckmate();
    bool isStalemate();

    
    bool isLegal(Square from, Square to, PieceType promotion = NONE);

    
    std::vector<Square> legalDestinations(Square from);

private:
    BoardState& board_;

    void genPawnMoves  (Square sq, Color c, std::vector<Move>& out) const;
    void genKnightMoves(Square sq, Color c, std::vector<Move>& out) const;
    void genSlidingMoves(Square sq, Color c, const int dirs[][2], int numDirs, std::vector<Move>& out) const;
    void genKingMoves  (Square sq, Color c, std::vector<Move>& out) const;

    bool isSquareAttacked(Square sq, Color byColor) const;
};
