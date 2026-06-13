#include "CaptureTracker.h"
#include <algorithm>

int CaptureTracker::pieceValue(PieceType t) {
    switch (t) {
        case PAWN:   return 100;
        case KNIGHT: return 300;
        case BISHOP: return 300;
        case ROOK:   return 500;
        case QUEEN:  return 900;
        default:     return 0;
    }
}

void CaptureTracker::addCapture(const Piece& p) {
    if (p.empty()) return;
    
    auto& vec = (p.color == BLACK) ? capturedByWhite_ : capturedByBlack_;
    vec.push_back(p);
    std::stable_sort(vec.begin(), vec.end(), [](const Piece& a, const Piece& b){
        return pieceValue(a.type) > pieceValue(b.type);
    });
}

void CaptureTracker::removeCapture(const Piece& p) {
    if (p.empty()) return;
    auto& vec = (p.color == BLACK) ? capturedByWhite_ : capturedByBlack_;
    auto it = std::find_if(vec.begin(), vec.end(), [&](const Piece& x){ return x == p; });
    if (it != vec.end()) vec.erase(it);
}

void CaptureTracker::clear() {
    capturedByWhite_.clear();
    capturedByBlack_.clear();
}

const std::vector<Piece>& CaptureTracker::capturedBy(Color c) const {
    return (c == WHITE) ? capturedByWhite_ : capturedByBlack_;
}

int CaptureTracker::materialAdvantage() const {
    int white = 0, black = 0;
    for (auto& p : capturedByWhite_) white += pieceValue(p.type);
    for (auto& p : capturedByBlack_) black += pieceValue(p.type);
    return white - black;
}
