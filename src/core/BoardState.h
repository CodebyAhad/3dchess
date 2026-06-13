#pragma once
#include <array>
#include <string>
#include <vector>
#include <cstdint>





enum PieceType : uint8_t {
    NONE   = 0,
    PAWN   = 1,
    KNIGHT = 2,
    BISHOP = 3,
    ROOK   = 4,
    QUEEN  = 5,
    KING   = 6,
};

enum Color : uint8_t { WHITE = 0, BLACK = 1 };

struct Piece {
    PieceType type  = NONE;
    Color     color = WHITE;

    bool empty() const { return type == NONE; }
    bool operator==(const Piece& o) const { return type == o.type && color == o.color; }
    bool operator!=(const Piece& o) const { return !(*this == o); }
};





using Square = int;
constexpr Square SQ_NONE = -1;

inline Square makeSquare(int file, int rank) { return rank * 8 + file; }
inline int    fileOf(Square sq)              { return sq % 8; }
inline int    rankOf(Square sq)              { return sq / 8; }




enum CastlingRight : uint8_t {
    CR_NONE  = 0,
    CR_WK    = 1 << 0,   
    CR_WQ    = 1 << 1,   
    CR_BK    = 1 << 2,   
    CR_BQ    = 1 << 3,   
    CR_ALL   = 0xF,
};




class BoardState {
public:
    
    std::array<Piece, 64> squares{};

    Color    sideToMove   = WHITE;
    uint8_t  castlingRights = CR_ALL;
    Square   enPassantSq  = SQ_NONE;   
    int      halfMoveClock = 0;         
    int      fullMoveNumber = 1;

    
    BoardState();

    
    void loadFEN(const std::string& fen);

    
    std::string toFEN() const;

    
    void reset();

    
    Piece& at(Square sq)             { return squares[sq]; }
    const Piece& at(Square sq) const { return squares[sq]; }
    Piece& at(int file, int rank)    { return squares[makeSquare(file, rank)]; }

    bool isEmpty(Square sq) const    { return squares[sq].empty(); }

    
    Square kingSquare(Color c) const;
    std::vector<Square> piecesOf(Color c, PieceType t) const;
    std::vector<Square> allPiecesOf(Color c) const;

    
    struct MoveInfo {
        Square   from, to;
        Piece    movedPiece, capturedPiece;
        PieceType promotion = NONE;
        bool     isCastling = false;
        bool     isEnPassant = false;
        Square   prevEnPassantSq = SQ_NONE;
        uint8_t  prevCastlingRights = CR_ALL;
        int      prevHalfMoveClock = 0;
    };

    MoveInfo applyMove(Square from, Square to, PieceType promotion = NONE);
    void undoMove(const MoveInfo& info);

    
    static std::string squareName(Square sq);   
    static Square      parseSquare(const std::string& name); 

    static const std::string START_FEN;
};
