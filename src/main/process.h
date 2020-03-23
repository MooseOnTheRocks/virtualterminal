#ifndef PROCESS_H
#define PROCESS_H

#include <string>
#include <windows.h>

class Process {
public:
    Process();
    ~Process();
    
    void close();
    bool start(std::string cmd);
    bool write(const std::string& s);
    bool read(std::string& buf);
    bool alive();
    
private:
    std::string mCmd;
    bool mEmpty;
    PROCESS_INFORMATION mProcInfo;
    STARTUPINFO mStartupInfo;
    HANDLE mChildInRd;
    HANDLE mChildInWr;
    HANDLE mChildOutRd;
    HANDLE mChildOutWr;
    // TODO: Don't make this a literal
    CHAR mChBuf[4096];
};

#endif // PROCESS_H