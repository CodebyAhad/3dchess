#include "BoardState.h"
#include <sstream>
#include <stdexcept>
#include <cctype>
#include <algorithm>

const std::string BoardState::START_FEN =
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";


BoardState::BoardState() { reset(); }

void BoardState::reset() { loadFEN(START_FEN); }


static PieceType charToPieceType(char c) {
    switch (std::tolower(c)) {
        case 'p': return PAWN;
        case 'n': return KNIGHT;
        case 'b': return BISHOP;
        case 'r': return ROOK;
        case 'q': return QUEEN;
        case 'k': return KING;
        default:  return NONE;
    }
}

static char pieceTypeToChar(PieceType t, Color c) {
    char ch = ' ';
    switch (t) {
        case PAWN:   ch = 'p'; break;
        case KNIGHT: ch = 'n'; break;
        case BISHOP: ch = 'b'; break;
        case ROOK:   ch = 'r'; break;
        case QUEEN:  ch = 'q'; break;
        case KING:   ch = 'k'; break;
        default:     ch = '?'; break;
    }
    return (c == WHITE) ? std::toupper(ch) : ch;
}


void BoardState::loadFEN(const std::string& fen) {
    squares.fill(Piece{});
    std::istringstream ss(fen);
    std::string piecePlacement, activeColor, castling, enPassant;
    ss >> piecePlacement >> activeColor >> castling >> enPassant
       >> halfMoveClock >> fullMoveNumber;

    
    int file = 0, rank = 7;
    for (char c : piecePlacement) {
        if (c == '/') { file = 0; --rank; }
        else if (std::isdigit(c)) { file += (c - '0'); }
        else {
            Color col = std::isupper(c) ? WHITE : BLACK;
            PieceType pt = charToPieceType(c);
            squares[makeSquare(file, rank)] = { pt, col };
            ++file;
        }
    }

    sideToMove = (activeColor == "w") ? WHITE : BLACK;

    castlingRights = CR_NONE;
    for (char c : castling) {
        switch (c) {
            case 'K': castlingRights |= CR_WK; break;
            case 'Q': castlingRights |= CR_WQ; break;
            case 'k': castlingRights |= CR_BK; break;
            case 'q': castlingRights |= CR_BQ; break;
            default: break;
        }
    }

    enPassantSq = (enPassant != "-") ? parseSquare(enPassant) : SQ_NONE;
}


std::string BoardState::toFEN() const {
    std::string fen;
    for (int r = 7; r >= 0; --r) {
        int empty = 0;
        for (int f = 0; f < 8; ++f) {
            const Piece& p = squares[makeSquare(f, r)];
            if (p.empty()) { ++empty; }
            else {
                if (empty) { fen += ('0' + empty); empty = 0; }
                fen += pieceTypeToChar(p.type, p.color);
            }
        }
        if (empty) fen += ('0' + empty);
        if (r > 0) fen += '/';
    }
    fen += (sideToMove == WHITE) ? " w " : " b ";

    std::string cr;
    if (castlingRights & CR_WK) cr += 'K';
    if (castlingRights & CR_WQ) cr += 'Q';
    if (castlingRights & CR_BK) cr += 'k';
    if (castlingRights & CR_BQ) cr += 'q';
    fen += cr.empty() ? "-" : cr;

    fen += ' ';
    fen += (enPassantSq != SQ_NONE) ? squareName(enPassantSq) : "-";
    fen += ' ';
    fen += std::to_string(halfMoveClock);
    fen += ' ';
    fen += std::to_string(fullMoveNumber);
    return fen;
}


Square BoardState::kingSquare(Color c) const {
    for (Square sq = 0; sq < 64; ++sq)
        if (squares[sq].type == KING && squares[sq].color == c) return sq;
    return SQ_NONE;
}

std::vector<Square> BoardState::piecesOf(Color c, PieceType t) const {
    std::vector<Square> out;
    for (Square sq = 0; sq < 64; ++sq)
        if (squares[sq].type == t && squares[sq].color == c) out.push_back(sq);
    return out;
}

std::vector<Square> BoardState::allPiecesOf(Color c) const {
    std::vector<Square> out;
    for (Square sq = 0; sq < 64; ++sq)
        if (!squares[sq].empty() && squares[sq].color == c) out.push_back(sq);
    return out;
}


BoardState::MoveInfo BoardState::applyMove(Square from, Square to, PieceType promotion) {
    MoveInfo info;
    info.from = from; info.to = to;
    info.movedPiece    = squares[from];
    info.capturedPiece = squares[to];
    info.promotion     = promotion;
    info.prevEnPassantSq    = enPassantSq;
    info.prevCastlingRights = castlingRights;
    info.prevHalfMoveClock  = halfMoveClock;

    
    const Piece mover = squares[from];

    
    if (mover.type == PAWN && to == enPassantSq) {
        info.isEnPassant = true;
        int epFile = fileOf(to);
        int epRank = rankOf(from);  
        info.capturedPiece = squares[makeSquare(epFile, epRank)];
        squares[makeSquare(epFile, epRank)] = {};
    }

    
    if (mover.type == KING) {
        int fileDiff = fileOf(to) - fileOf(from);
        if (std::abs(fileDiff) == 2) {
            info.isCastling = true;
            bool kingSide = (fileDiff > 0);
            int rank = rankOf(from);
            Square rookFrom = makeSquare(kingSide ? 7 : 0, rank);
            Square rookTo   = makeSquare(kingSide ? 5 : 3, rank);
            squares[rookTo]   = squares[rookFrom];
            squares[rookFrom] = {};
        }
    }

    
    squares[to]   = mover;
    squares[from] = {};

    
    if (mover.type == PAWN && promotion != NONE) {
        squares[to].type = promotion;
    }

    
    enPassantSq = SQ_NONE;
    if (mover.type == PAWN && std::abs(rankOf(to) - rankOf(from)) == 2) {
        enPassantSq = makeSquare(fileOf(from), (rankOf(from) + rankOf(to)) / 2);
    }

    
    if (mover.type == KING) {
        castlingRights &= (mover.color == WHITE) ? ~(CR_WK | CR_WQ) : ~(CR_BK | CR_BQ);
    }
    if (from == 0  || to == 0)  castlingRights &= ~CR_WQ;
    if (from == 7  || to == 7)  castlingRights &= ~CR_WK;
    if (from == 56 || to == 56) castlingRights &= ~CR_BQ;
    if (from == 63 || to == 63) castlingRights &= ~CR_BK;

    
    if (mover.type == PAWN || !info.capturedPiece.empty()) halfMoveClock = 0;
    else ++halfMoveClock;

    if (sideToMove == BLACK) ++fullMoveNumber;
    sideToMove = (sideToMove == WHITE) ? BLACK : WHITE;

    return info;
}


void BoardState::undoMove(const MoveInfo& info) {
    squares[info.from] = info.movedPiece;
    squares[info.to]   = info.capturedPiece;

    if (info.isEnPassant) {
        int epFile = fileOf(info.to);
        int epRank = rankOf(info.from);
        squares[makeSquare(epFile, epRank)] = info.capturedPiece;
        squares[info.to] = {};
    }

    if (info.isCastling) {
        int rank = rankOf(info.from);
        bool kingSide = (fileOf(info.to) > fileOf(info.from));
        squares[makeSquare(kingSide ? 7 : 0, rank)] = squares[makeSquare(kingSide ? 5 : 3, rank)];
        squares[makeSquare(kingSide ? 5 : 3, rank)] = {};
    }

    if (info.promotion != NONE) squares[info.from].type = PAWN;

    enPassantSq     = info.prevEnPassantSq;
    castlingRights  = info.prevCastlingRights;
    halfMoveClock   = info.prevHalfMoveClock;
    sideToMove      = (sideToMove == WHITE) ? BLACK : WHITE;
    if (sideToMove == BLACK) --fullMoveNumber;
}


std::string BoardState::squareName(Square sq) {
    if (sq == SQ_NONE) return "-";
    std::string s;
    s += ('a' + fileOf(sq));
    s += ('1' + rankOf(sq));
    return s;
}

Square BoardState::parseSquare(const std::string& name) {
    if (name.size() < 2) return SQ_NONE;
    int f = name[0] - 'a';
    int r = name[1] - '1';
    if (f < 0 || f > 7 || r < 0 || r > 7) return SQ_NONE;
    return makeSquare(f, r);
}
