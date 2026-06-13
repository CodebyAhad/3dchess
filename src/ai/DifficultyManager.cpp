#include "DifficultyManager.h"

DifficultyConfig DifficultyManager::getConfig(Level lvl) {
    switch (lvl) {
        case Level::EASY:   return { 100,    800,  2.0f, 1 };
        case Level::MEDIUM: return { 3000,   2500, 0.8f, 2 };
        case Level::HARD:   return { 100000, 6000, 0.1f, 3 };
    }
    return { 3000, 2500, 0.8f, 2 };
}

std::string DifficultyManager::levelName(Level lvl) {
    switch (lvl) {
        case Level::EASY:   return "Easy";
        case Level::MEDIUM: return "Medium";
        case Level::HARD:   return "Hard";
    }
    return "Unknown";
}
