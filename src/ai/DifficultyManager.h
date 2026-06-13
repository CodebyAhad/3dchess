#pragma once
#include "../core/GameController.h"
#include <string>


struct DifficultyConfig {
    int   nodes;         
    int   movetimeMs;    
    float temperature;   
    int   cpuct;         
};

class DifficultyManager {
public:
    static DifficultyConfig getConfig(Level lvl);
    static std::string      levelName(Level lvl);
};
