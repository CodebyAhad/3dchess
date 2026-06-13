#include <SDL2/SDL.h>
#include <glad/glad.h>
#include <iostream>
#include <string>
#include <atomic>
#include <thread>
#include <chrono>
#include <random>

#ifdef _WIN32
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#endif

#include "renderer/Renderer.h"
#include "input/InputHandler.h"
#include "core/GameController.h"
#include "ai/UCIBridge.h"
#include "ai/DifficultyManager.h"


static constexpr int   INIT_W     = 1200;
static constexpr int   INIT_H     = 800;
static const char*     WINDOW_TITLE = "3D Chess";
static const std::string LC0_EXE  = "lc0.exe";

static std::string exeDir() {
#ifdef _WIN32
    char buf[MAX_PATH];
    DWORD n = GetModuleFileNameA(nullptr, buf, MAX_PATH);
    if (n == 0 || n >= MAX_PATH) return ".";
    std::string path(buf, buf + n);
    auto slash = path.find_last_of("\\/");
    return (slash == std::string::npos) ? "." : path.substr(0, slash);
#else
    return ".";
#endif
}


int main(int , char* []) {

    
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "[SDL] Init failed: " << SDL_GetError() << "\n";
        return 1;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    SDL_DisplayMode displayMode{};
    int windowW = INIT_W;
    int windowH = INIT_H;
    if (SDL_GetCurrentDisplayMode(0, &displayMode) == 0) {
        windowW = displayMode.w;
        windowH = displayMode.h;
    }

    SDL_Window* window = SDL_CreateWindow(
        WINDOW_TITLE,
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        windowW, windowH,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_FULLSCREEN_DESKTOP
    );
    if (!window) {
        std::cerr << "[SDL] CreateWindow failed: " << SDL_GetError() << "\n";
        return 1;
    }

    SDL_GLContext glCtx = SDL_GL_CreateContext(window);
    if (!glCtx) {
        std::cerr << "[SDL] GL context failed: " << SDL_GetError() << "\n";
        return 1;
    }
    SDL_GL_SetSwapInterval(1);  

    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
        std::cerr << "[GLAD] Failed to load OpenGL\n";
        return 1;
    }

    
    const Uint32 EVT_AI_MOVE   = SDL_RegisterEvents(2);
    const Uint32 EVT_NEW_GAME  = EVT_AI_MOVE + 1;
    if (EVT_AI_MOVE == (Uint32)-1 || EVT_NEW_GAME == (Uint32)-1) {
        std::cerr << "[SDL] SDL_RegisterEvents failed\n";
        return 1;
    }
    std::cout << "[GL] Vendor:   " << glGetString(GL_VENDOR)   << "\n";
    std::cout << "[GL] Renderer: " << glGetString(GL_RENDERER) << "\n";
    std::cout << "[GL] Version:  " << glGetString(GL_VERSION)  << "\n";

    glEnable(GL_MULTISAMPLE);
    int drawableW = windowW;
    int drawableH = windowH;
    SDL_GetWindowSize(window, &drawableW, &drawableH);
    glViewport(0, 0, drawableW, drawableH);

    
    const std::string baseDir = exeDir();
    std::string assetDir = baseDir + "/assets";

    
    Renderer       renderer;
    InputHandler   input;
    GameController ctrl;
    UCIBridge      uci;

    if (!renderer.init(drawableW, drawableH, assetDir)) {
        std::cerr << "[Renderer] init failed\n";
        return 1;
    }

    
    std::atomic<bool> aiAvailable{false};
    std::atomic<bool> aiReady{false};
    std::atomic<bool> aiLaunched{false};

    const std::string lc0Path = baseDir + "/" + LC0_EXE;
    if (uci.launch(lc0Path)) {
        aiLaunched = true;
        std::thread([&](){
            
            std::string weightsPath = baseDir + "/maia-1100.pb.gz";
            const bool initialized = uci.initialize(weightsPath);
            aiAvailable = initialized;
            if (!initialized) aiLaunched = false;
            aiReady = true;
        }).detach();
    } else {
        std::cerr << "[UCI] lc0 not found\n";
        aiReady = true;
    }

    
    Level    pendingLevel  = Level::MEDIUM;
    MenuColorChoice pendingColorChoice = MenuColorChoice::RANDOM;
    bool     pendingVsAI   = true;
    bool     hasResetCurrentGame = false;
    bool     autoMenuScheduled = false;
    Uint32   autoMenuAtMs = 0;

    auto winnerText = [&](Color winner) {
        if (ctrl.config().vsAI) {
            return (winner == ctrl.config().playerColor) ? std::string("YOU WIN") : std::string("AI WINS");
        }
        return (winner == WHITE) ? std::string("WHITE WINS") : std::string("BLACK WINS");
    };

    auto scheduleMenuReturn = [&](Uint32 delayMs) {
        autoMenuScheduled = true;
        autoMenuAtMs = SDL_GetTicks() + delayMs;
    };

    auto clearResultState = [&] {
        autoMenuScheduled = false;
        renderer.ui().setResultMessage("");
    };

    auto postEvent = [](Uint32 type) {
        SDL_Event ev{};
        ev.type = type;
        SDL_PushEvent(&ev);
    };

    auto resolvePlayerColor = [&] {
        if (pendingColorChoice == MenuColorChoice::BLACK) return BLACK;
        if (pendingColorChoice == MenuColorChoice::RANDOM) {
            static std::mt19937 rng{std::random_device{}()};
            return std::uniform_int_distribution<int>(0, 1)(rng) == 0 ? WHITE : BLACK;
        }
        return WHITE;
    };

    
    renderer.ui().setOnDifficultyChanged([&](Level l){
        pendingLevel = l;
        pendingVsAI = true;
        postEvent(EVT_NEW_GAME);
    });
    renderer.ui().setOnColorChanged([&](MenuColorChoice c) { pendingColorChoice = c; });
    renderer.ui().setOnHumanGame([&]{
        pendingVsAI = false;
        postEvent(EVT_NEW_GAME);
    });
    renderer.ui().setOnExit([&]{ postEvent(SDL_QUIT); });
    renderer.ui().setOnUndo([&]{ ctrl.undoLastMove(); });
    renderer.ui().setOnResign([&]{
        uci.stopSearch();
        hasResetCurrentGame = false;
        const Color winner = (ctrl.board().sideToMove == WHITE) ? BLACK : WHITE;
        renderer.ui().setResultMessage(winnerText(winner));
        ctrl.endGame();
        scheduleMenuReturn(1800);
    });
    renderer.ui().setOnNewGame([&]{
        uci.stopSearch();
        hasResetCurrentGame = true;
        clearResultState();
        ctrl.resetGame();
        if (ctrl.config().vsAI) renderer.facePlayerColor(ctrl.config().playerColor);
    });
    renderer.ui().setOnBackToMenu([&]{
        uci.stopSearch();
        hasResetCurrentGame = false;
        clearResultState();
        ctrl.returnToMenu();
    });
    renderer.ui().setOnPromotion([&](PieceType pt){ ctrl.choosePromotion(pt); });

    
    ctrl.onAITurn = [&](const std::string& fen) {
        
        
        auto request = [&](const std::string& fenNow) {
            auto cfg = DifficultyManager::getConfig(ctrl.config().difficulty);
            uci.requestMove(fenNow, cfg, [&](const std::string& uciMove){
                
                SDL_Event ev;
                ev.type       = EVT_AI_MOVE;
                ev.user.code  = 0;
                ev.user.data1 = new std::string(uciMove);
                SDL_PushEvent(&ev);
            });
        };

        if (aiAvailable && aiReady) { request(fen); return; }

        std::thread([&, fen]{
            
            for (int i = 0; i < 100; ++i) {
                if (aiAvailable && aiReady) break;
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
            if (aiAvailable && aiReady) request(fen);
        }).detach();
    };

    
    bool  dragging    = false;
    int   dragLastX   = 0, dragLastY = 0;

    
    bool running = true;
    while (running) {
        
        int wFrame, hFrame;
        SDL_GetWindowSize(window, &wFrame, &hFrame);
        input.setCamera(renderer.view(), renderer.proj(), wFrame, hFrame);
        renderer.ui().setInputState(ctrl.state());

        SDL_Event e;
        while (SDL_PollEvent(&e)) {

            
            if (e.type == EVT_AI_MOVE) {
                auto* moveStr = static_cast<std::string*>(e.user.data1);
                ctrl.applyAIMove(*moveStr);
                delete moveStr;
                continue;
            }
            if (e.type == EVT_NEW_GAME) {
                GameConfig cfg;
                cfg.difficulty  = pendingLevel;
                cfg.playerColor = pendingVsAI ? resolvePlayerColor() : WHITE;
                cfg.vsAI        = pendingVsAI && aiLaunched.load();
                hasResetCurrentGame = false;
                clearResultState();
                ctrl.newGame(cfg);
                if (cfg.vsAI) renderer.facePlayerColor(cfg.playerColor);
                continue;
            }

            
            if (e.type == SDL_QUIT)                          { running = false; continue; }
            if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) { running = false; continue; }
            if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_r)      { renderer.resetCamera(); continue; }
            if (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_RESIZED) {
                int w = e.window.data1, h = e.window.data2;
                renderer.resize(w, h);
                input.setCamera(renderer.view(), renderer.proj(), w, h);
                continue;
            }

            
            if (e.type == SDL_MOUSEWHEEL) {
                renderer.zoomCamera((float)e.wheel.y * 0.8f);
                continue;
            }

            
            if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_RIGHT) {
                dragging = true; dragLastX = e.button.x; dragLastY = e.button.y;
                continue;
            }
            if (e.type == SDL_MOUSEBUTTONUP && e.button.button == SDL_BUTTON_RIGHT) {
                dragging = false;
                continue;
            }
            if (e.type == SDL_MOUSEMOTION && dragging) {
                float dx = (float)(e.motion.x - dragLastX);
                float dy = (float)(e.motion.y - dragLastY);
                dragLastX = e.motion.x; dragLastY = e.motion.y;
                renderer.orbitCamera(dx * 0.4f, dy * 0.3f);
                continue;
            }

            
            if (e.type == SDL_MOUSEMOTION) {
                int w, h; SDL_GetWindowSize(window, &w, &h);
                
                renderer.ui().onMouseMove((float)e.motion.x, (float)(h - e.motion.y));
                continue;
            }

            
            if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
                int w, h; SDL_GetWindowSize(window, &w, &h);
                float ux = (float)e.button.x;
                float uy = (float)(h - e.button.y);

                
                if (renderer.ui().onMouseClick(ux, uy)) continue;

                
                bool isClick = false, isRight = false;
                Square sq = input.processEvent(e, isClick, isRight);
                if (isClick && sq != SQ_NONE) {
                    ctrl.selectSquare(sq);
                }
                continue;
            }
        }

        const GameState frameState = ctrl.state();
        if (!autoMenuScheduled) {
            if (frameState == GameState::CHECKMATE) {
                const Color winner = (ctrl.board().sideToMove == WHITE) ? BLACK : WHITE;
                renderer.ui().setResultMessage(winnerText(winner));
                scheduleMenuReturn(2200);
            } else if (frameState == GameState::STALEMATE || frameState == GameState::DRAW_50MOVE) {
                renderer.ui().setResultMessage(frameState == GameState::STALEMATE ? "DRAW BY STALEMATE" : "DRAW");
                scheduleMenuReturn(2200);
            }
        }

        if (autoMenuScheduled && SDL_GetTicks() >= autoMenuAtMs) {
            uci.stopSearch();
            hasResetCurrentGame = false;
            clearResultState();
            ctrl.returnToMenu();
        }

        
        renderer.ui().setShowBackToMenu(hasResetCurrentGame);
        renderer.drawFrame(ctrl);
        SDL_GL_SwapWindow(window);
    }

    
    SDL_GL_DeleteContext(glCtx);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
