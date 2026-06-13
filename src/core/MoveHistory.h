#pragma once
#include "BoardState.h"
#include "MoveValidator.h"
#include <vector>
#include <string>


struct HistoryEntry {
    Move        move;
    std::string san;          
    std::string fenAfter;     
    bool        isCheck      = false;
    bool        isCheckmate  = false;
};

class MoveHistory {
public:
    void push(const Move& m, const std::string& san, const std::string& fen,
              bool check, bool checkmate);
    void pop();
    void clear();

    const std::vector<HistoryEntry>& entries() const { return entries_; }
    size_t size() const { return entries_.size(); }
    bool   empty() const { return entries_.empty(); }

    
    std::string toPGN(const std::string& event = "?",
                      const std::string& site  = "?",
                      const std::string& date  = "????.??.??") const;

private:
    std::vector<HistoryEntry> entries_;
};
