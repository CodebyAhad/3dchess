#include "UCIBridge.h"
#include <sstream>

UCIBridge::UCIBridge() = default;

UCIBridge::~UCIBridge() {
    stopSearch();
    if (running_) {
        sendLine("quit");
    }

#ifdef _WIN32
    if (procInfo_.hProcess != nullptr) {
        TerminateProcess(procInfo_.hProcess, 0);
    }
    running_ = false;
    if (hChildStdoutRd_ != INVALID_HANDLE_VALUE) {
        CloseHandle(hChildStdoutRd_);
        hChildStdoutRd_ = INVALID_HANDLE_VALUE;
    }
    if (reader_.joinable()) reader_.join();
    if (hChildStdinWr_  != INVALID_HANDLE_VALUE) CloseHandle(hChildStdinWr_);
    if (procInfo_.hProcess != nullptr) {
        CloseHandle(procInfo_.hProcess);
        CloseHandle(procInfo_.hThread);
    }
#else
    running_ = false;
    if (toEngine_)   { fclose(toEngine_); }
    if (fromEngine_) { fclose(fromEngine_); }
    if (reader_.joinable()) reader_.join();
#endif
}


#ifdef _WIN32
bool UCIBridge::launch(const std::string& lc0Path) {
    HANDLE hStdinRd, hStdoutWr;
    SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), nullptr, TRUE };

    if (!CreatePipe(&hStdinRd,  &hChildStdinWr_,  &sa, 0)) return false;
    if (!CreatePipe(&hChildStdoutRd_, &hStdoutWr, &sa, 0)) return false;

    SetHandleInformation(hChildStdinWr_,  HANDLE_FLAG_INHERIT, 0);
    SetHandleInformation(hChildStdoutRd_, HANDLE_FLAG_INHERIT, 0);

    STARTUPINFOA si = {};
    si.cb          = sizeof(si);
    si.hStdInput   = hStdinRd;
    si.hStdOutput  = hStdoutWr;
    si.hStdError   = hStdoutWr;
    si.dwFlags    |= STARTF_USESTDHANDLES;

    std::string cmd = "\"" + lc0Path + "\"";
    bool ok = CreateProcessA(nullptr, cmd.data(), nullptr, nullptr,
                              TRUE, CREATE_NO_WINDOW, nullptr, nullptr,
                              &si, &procInfo_);

    CloseHandle(hStdinRd);
    CloseHandle(hStdoutWr);
    if (!ok) return false;

    return true;
}

void UCIBridge::sendLine(const std::string& line) {
    std::string msg = line + "\n";
    DWORD written;
    WriteFile(hChildStdinWr_, msg.c_str(), (DWORD)msg.size(), &written, nullptr);
}

std::string UCIBridge::readLine() {
    std::string line;
    char ch;
    DWORD read;
    while (ReadFile(hChildStdoutRd_, &ch, 1, &read, nullptr) && read > 0) {
        if (ch == '\n') break;
        if (ch != '\r') line += ch;
    }
    return line;
}

#else

bool UCIBridge::launch(const std::string& lc0Path) {
    int toChild[2], fromChild[2];
    if (pipe(toChild) || pipe(fromChild)) return false;

    enginePid_ = fork();
    if (enginePid_ == 0) {
        dup2(toChild[0],   STDIN_FILENO);
        dup2(fromChild[1], STDOUT_FILENO);
        dup2(fromChild[1], STDERR_FILENO);
        close(toChild[0]); close(toChild[1]);
        close(fromChild[0]); close(fromChild[1]);
        execl(lc0Path.c_str(), lc0Path.c_str(), nullptr);
        _exit(1);
    }
    close(toChild[0]);
    close(fromChild[1]);
    toEngine_   = fdopen(toChild[1],   "w");
    fromEngine_ = fdopen(fromChild[0], "r");
    if (!toEngine_ || !fromEngine_) return false;

    return true;
}

void UCIBridge::sendLine(const std::string& line) {
    fprintf(toEngine_, "%s\n", line.c_str());
    fflush(toEngine_);
}

std::string UCIBridge::readLine() {
    std::string line;
    char ch;
    while (fread(&ch, 1, 1, fromEngine_) == 1) {
        if (ch == '\n') break;
        if (ch != '\r') line += ch;
    }
    return line;
}
#endif


bool UCIBridge::initialize(const std::string& weightsPath) {
    sendLine("uci");
    for (int i = 0; i < 500; ++i) {
        std::string line = readLine();
        if (line.find("uciok") != std::string::npos) break;
        if (i == 499) return false;
    }
    if (!weightsPath.empty()) {
        sendLine("setoption name WeightsFile value " + weightsPath);
    }
    sendLine("isready");
    for (int i = 0; i < 500; ++i) {
        std::string line = readLine();
        if (line.find("readyok") != std::string::npos) {
            startReader();
            return true;
        }
    }
    return false;
}

void UCIBridge::startReader() {
    if (running_) return;
    running_ = true;
    reader_ = std::thread(&UCIBridge::readerThread, this);
}


void UCIBridge::requestMove(const std::string& fen,
                             const DifficultyConfig& cfg,
                             std::function<void(const std::string&)> callback) {
    if (!running_) return;

    {
        std::lock_guard<std::mutex> lk(callbackMutex_);
        moveCallback_ = callback;
    }
    searching_ = true;

    
    sendLine("setoption name Nodes value " + std::to_string(cfg.nodes));
    sendLine("setoption name Temperature value " + std::to_string(cfg.temperature));
    sendLine("ucinewgame");
    sendLine("position fen " + fen);
    sendLine("go movetime " + std::to_string(cfg.movetimeMs)
             + " nodes " + std::to_string(cfg.nodes));
}

void UCIBridge::stopSearch() {
    if (searching_) {
        sendLine("stop");
        searching_ = false;
    }
}


void UCIBridge::readerThread() {
    while (running_) {
        std::string line = readLine();
        if (line.empty()) continue;

        if (line.rfind("bestmove", 0) == 0 && searching_) {
            searching_ = false;
            
            std::istringstream ss(line);
            std::string token, move;
            ss >> token >> move;  

            std::function<void(const std::string&)> cb;
            {
                std::lock_guard<std::mutex> lk(callbackMutex_);
                cb = moveCallback_;
                moveCallback_ = nullptr;
            }
            if (cb && move != "(none)") cb(move);
        }
    }
}
