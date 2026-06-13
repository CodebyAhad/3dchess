#pragma once
#include "DifficultyManager.h"
#include <string>
#include <functional>
#include <thread>
#include <atomic>
#include <queue>
#include <mutex>

#ifdef _WIN32
  #define WIN32_LEAN_AND_MEAN
  #include <windows.h>
#endif







class UCIBridge {
public:
    UCIBridge();
    ~UCIBridge();

    
    bool launch(const std::string& lc0Path);

    
    
    bool initialize(const std::string& weightsPath = {});

    
    
    void requestMove(const std::string& fen,
                     const DifficultyConfig& cfg,
                     std::function<void(const std::string&)> callback);

    
    void stopSearch();

    bool isRunning() const { return running_; }

private:
    void sendLine(const std::string& line);
    std::string readLine();

    void startReader();
    void readerThread();

#ifdef _WIN32
    HANDLE hChildStdinWr_  = INVALID_HANDLE_VALUE;
    HANDLE hChildStdoutRd_ = INVALID_HANDLE_VALUE;
    PROCESS_INFORMATION procInfo_{};
#else
    
    FILE* toEngine_   = nullptr;
    FILE* fromEngine_ = nullptr;
    int   enginePid_  = -1;
#endif

    std::thread              reader_;
    std::atomic<bool>        running_  {false};
    std::atomic<bool>        searching_{false};

    std::function<void(const std::string&)> moveCallback_;
    std::mutex callbackMutex_;
};
