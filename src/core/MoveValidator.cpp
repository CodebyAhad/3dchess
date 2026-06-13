#include "MoveValidator.h"
#include <algorithm>
#include <cstdlib>


std::string Move::toUCI() const {
    std::string s = BoardState::squareName(from) + BoardState::squareName(to);
    if (promotion != NONE) {
        const char map[] = " pnbrqk";
        s += map[promotion];
    }
    return s;
}

Move Move::fromUCI(const std::string& uci) {
    if (uci.size() < 4) return {};
    Move m;
    m.from = BoardState::parseSquare(uci.substr(0, 2));
    m.to   = BoardState::parseSquare(uci.substr(2, 2));
    if (uci.size() >= 5) {
        switch (uci[4]) {
            case 'q': m.promotion = QUEEN;  break;
            case 'r': m.promotion = ROOK;   break;
            case 'b': m.promotion = BISHOP; break;
            case 'n': m.promotion = KNIGHT; break;
            default: break;
        }
    }
    return m;
}


MoveValidator::MoveValidator(BoardState& board) : board_(board) {}


static const int KNIGHT_DIRS[8][2] = {{1,2},{2,1},{-1,2},{-2,1},{1,-2},{2,-1},{-1,-2},{-2,-1}};
static const int ROOK_DIRS  [4][2] = {{1,0},{-1,0},{0,1},{0,-1}};
static const int BISHOP_DIRS[4][2] = {{1,1},{-1,1},{1,-1},{-1,-1}};
static const int KING_DIRS  [8][2] = {{1,0},{-1,0},{0,1},{0,-1},{1,1},{-1,1},{1,-1},{-1,-1}};

bool MoveValidator::isSquareAttacked(Square sq, Color byColor) const {
    int f = fileOf(sq), r = rankOf(sq);

    
    int pawnDir = (byColor == WHITE) ? -1 : 1;
    for (int df : {-1, 1}) {
        int pf = f + df, pr = r + pawnDir;
        if (pf >= 0 && pf < 8 && pr >= 0 && pr < 8) {
            const Piece& p = board_.at(makeSquare(pf, pr));
            if (p.type == PAWN && p.color == byColor) return true;
        }
    }

    
    for (auto& d : KNIGHT_DIRS) {
        int nf = f + d[0], nr = r + d[1];
        if (nf >= 0 && nf < 8 && nr >= 0 && nr < 8) {
            const Piece& p = board_.at(makeSquare(nf, nr));
            if (p.type == KNIGHT && p.color == byColor) return true;
        }
    }

    
    for (auto& d : ROOK_DIRS) {
        int nf = f + d[0], nr = r + d[1];
        while (nf >= 0 && nf < 8 && nr >= 0 && nr < 8) {
            const Piece& p = board_.at(makeSquare(nf, nr));
            if (!p.empty()) {
                if (p.color == byColor && (p.type == ROOK || p.type == QUEEN)) return true;
                break;
            }
            nf += d[0]; nr += d[1];
        }
    }

    
    for (auto& d : BISHOP_DIRS) {
        int nf = f + d[0], nr = r + d[1];
        while (nf >= 0 && nf < 8 && nr >= 0 && nr < 8) {
            const Piece& p = board_.at(makeSquare(nf, nr));
            if (!p.empty()) {
                if (p.color == byColor && (p.type == BISHOP || p.type == QUEEN)) return true;
                break;
            }
            nf += d[0]; nr += d[1];
        }
    }

    
    for (auto& d : KING_DIRS) {
        int nf = f + d[0], nr = r + d[1];
        if (nf >= 0 && nf < 8 && nr >= 0 && nr < 8) {
            const Piece& p = board_.at(makeSquare(nf, nr));
            if (p.type == KING && p.color == byColor) return true;
        }
    }

    return false;
}


void MoveValidator::genPawnMoves(Square sq, Color c, std::vector<Move>& out) const {
    int f = fileOf(sq), r = rankOf(sq);
    int dir = (c == WHITE) ? 1 : -1;
    int startRank = (c == WHITE) ? 1 : 6;
    int promRank  = (c == WHITE) ? 6 : 1;

    auto addPawnMove = [&](Square from, Square to) {
        if (rankOf(from) == promRank) {
            for (PieceType pt : {QUEEN, ROOK, BISHOP, KNIGHT})
                out.push_back({from, to, pt});
        } else {
            out.push_back({from, to, NONE});
        }
    };

    
    int nr = r + dir;
    if (nr >= 0 && nr < 8 && board_.isEmpty(makeSquare(f, nr))) {
        addPawnMove(sq, makeSquare(f, nr));
        
        if (r == startRank && board_.isEmpty(makeSquare(f, r + 2*dir)))
            out.push_back({sq, makeSquare(f, r + 2*dir), NONE});
    }

    
    for (int df : {-1, 1}) {
        int nf = f + df;
        if (nf < 0 || nf >= 8) continue;
        Square dest = makeSquare(nf, nr);
        if (nr < 0 || nr >= 8) continue;
        const Piece& target = board_.at(dest);
        if ((!target.empty() && target.color != c) || dest == board_.enPassantSq)
            addPawnMove(sq, dest);
    }
}

void MoveValidator::genKnightMoves(Square sq, Color c, std::vector<Move>& out) const {
    int f = fileOf(sq), r = rankOf(sq);
    for (auto& d : KNIGHT_DIRS) {
        int nf = f + d[0], nr = r + d[1];
        if (nf < 0 || nf >= 8 || nr < 0 || nr >= 8) continue;
        Square dest = makeSquare(nf, nr);
        const Piece& p = board_.at(dest);
        if (p.empty() || p.color != c) out.push_back({sq, dest, NONE});
    }
}

void MoveValidator::genSlidingMoves(Square sq, Color c, const int dirs[][2], int numDirs, std::vector<Move>& out) const {
    int f = fileOf(sq), r = rankOf(sq);
    for (int i = 0; i < numDirs; ++i) {
        int nf = f + dirs[i][0], nr = r + dirs[i][1];
        while (nf >= 0 && nf < 8 && nr >= 0 && nr < 8) {
            Square dest = makeSquare(nf, nr);
            const Piece& p = board_.at(dest);
            if (!p.empty()) {
                if (p.color != c) out.push_back({sq, dest, NONE});
                break;
            }
            out.push_back({sq, dest, NONE});
            nf += dirs[i][0]; nr += dirs[i][1];
        }
    }
}

void MoveValidator::genKingMoves(Square sq, Color c, std::vector<Move>& out) const {
    int f = fileOf(sq), r = rankOf(sq);
    for (auto& d : KING_DIRS) {
        int nf = f + d[0], nr = r + d[1];
        if (nf < 0 || nf >= 8 || nr < 0 || nr >= 8) continue;
        Square dest = makeSquare(nf, nr);
        const Piece& p = board_.at(dest);
        if (p.empty() || p.color != c) out.push_back({sq, dest, NONE});
    }

    
    auto& cr = board_.castlingRights;
    if (c == WHITE && r == 0 && f == 4) {
        if ((cr & CR_WK) && board_.isEmpty(makeSquare(5,0)) && board_.isEmpty(makeSquare(6,0))
            && !isSquareAttacked(makeSquare(4,0), BLACK) && !isSquareAttacked(makeSquare(5,0), BLACK))
            out.push_back({sq, makeSquare(6,0), NONE});
        if ((cr & CR_WQ) && board_.isEmpty(makeSquare(3,0)) && board_.isEmpty(makeSquare(2,0)) && board_.isEmpty(makeSquare(1,0))
            && !isSquareAttacked(makeSquare(4,0), BLACK) && !isSquareAttacked(makeSquare(3,0), BLACK))
            out.push_back({sq, makeSquare(2,0), NONE});
    }
    if (c == BLACK && r == 7 && f == 4) {
        if ((cr & CR_BK) && board_.isEmpty(makeSquare(5,7)) && board_.isEmpty(makeSquare(6,7))
            && !isSquareAttacked(makeSquare(4,7), WHITE) && !isSquareAttacked(makeSquare(5,7), WHITE))
            out.push_back({sq, makeSquare(6,7), NONE});
        if ((cr & CR_BQ) && board_.isEmpty(makeSquare(3,7)) && board_.isEmpty(makeSquare(2,7)) && board_.isEmpty(makeSquare(1,7))
            && !isSquareAttacked(makeSquare(4,7), WHITE) && !isSquareAttacked(makeSquare(3,7), WHITE))
            out.push_back({sq, makeSquare(2,7), NONE});
    }
}


std::vector<Move> MoveValidator::pseudoLegalMoves() const {
    std::vector<Move> moves;
    Color c = board_.sideToMove;
    for (Square sq = 0; sq < 64; ++sq) {
        const Piece& p = board_.at(sq);
        if (p.empty() || p.color != c) continue;
        switch (p.type) {
            case PAWN:   genPawnMoves(sq, c, moves); break;
            case KNIGHT: genKnightMoves(sq, c, moves); break;
            case BISHOP: genSlidingMoves(sq, c, BISHOP_DIRS, 4, moves); break;
            case ROOK:   genSlidingMoves(sq, c, ROOK_DIRS,   4, moves); break;
            case QUEEN:  genSlidingMoves(sq, c, BISHOP_DIRS, 4, moves);
                         genSlidingMoves(sq, c, ROOK_DIRS,   4, moves); break;
            case KING:   genKingMoves(sq, c, moves); break;
            default: break;
        }
    }
    return moves;
}

std::vector<Move> MoveValidator::legalMoves() {
    auto pseudo = pseudoLegalMoves();
    std::vector<Move> legal;
    Color c = board_.sideToMove;
    for (auto& m : pseudo) {
        auto info = board_.applyMove(m.from, m.to, m.promotion);
        if (!isInCheck(c)) legal.push_back(m);
        board_.undoMove(info);
    }
    return legal;
}

bool MoveValidator::isInCheck(Color c) const {
    Square ksq = board_.kingSquare(c);
    if (ksq == SQ_NONE) return false;
    return isSquareAttacked(ksq, (c == WHITE) ? BLACK : WHITE);
}

bool MoveValidator::isCheckmate() {
    return isInCheck(board_.sideToMove) && legalMoves().empty();
}

bool MoveValidator::isStalemate() {
    return !isInCheck(board_.sideToMove) && legalMoves().empty();
}

bool MoveValidator::isLegal(Square from, Square to, PieceType promotion) {
    auto moves = legalMoves();
    for (auto& m : moves) {
        if (m.from != from || m.to != to) continue;
        if (m.promotion == promotion) return true;
        if (promotion == NONE && m.promotion != NONE) return true;
    }
    return false;
}

std::vector<Square> MoveValidator::legalDestinations(Square from) {
    auto moves = legalMoves();
    std::vector<Square> dests;
    for (auto& m : moves)
        if (m.from == from) dests.push_back(m.to);
    return dests;
}
