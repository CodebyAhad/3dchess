#include "MoveHistory.h"
#include <sstream>

void MoveHistory::push(const Move& m, const std::string& san, const std::string& fen,
                       bool check, bool checkmate) {
    entries_.push_back({m, san, fen, check, checkmate});
}

void MoveHistory::pop() {
    if (!entries_.empty()) entries_.pop_back();
}

void MoveHistory::clear() { entries_.clear(); }

std::string MoveHistory::toPGN(const std::string& event,
                                const std::string& site,
                                const std::string& date) const {
    std::ostringstream pgn;
    pgn << "[Event \"" << event << "\"]\n";
    pgn << "[Site \""  << site  << "\"]\n";
    pgn << "[Date \""  << date  << "\"]\n";
    pgn << "[White \"Human\"]\n";
    pgn << "[Black \"Leela Chess Zero\"]\n";
    pgn << "\n";

    for (size_t i = 0; i < entries_.size(); ++i) {
        if (i % 2 == 0) pgn << (i / 2 + 1) << ". ";
        pgn << entries_[i].san;
        if (entries_[i].isCheckmate) pgn << "#";
        else if (entries_[i].isCheck) pgn << "+";
        pgn << " ";
    }
    pgn << "*\n";
    return pgn.str();
}
