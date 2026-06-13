#pragma once
#include "BoardState.h"
#include <vector>
#include <array>


class CaptureTracker {
public:
    void addCapture(const Piece& p);
    void removeCapture(const Piece& p);   
    void clear();

    
    const std::vector<Piece>& capturedBy(Color c) const;

    
    int materialAdvantage() const;

private:
    static int pieceValue(PieceType t);

    
    std::vector<Piece> capturedByWhite_;
    std::vector<Piece> capturedByBlack_;
};
