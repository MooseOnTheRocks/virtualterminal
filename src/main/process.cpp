#include "process.h"

#include <iostream>
#include <windows.h>

Process::Process() {
    mCmd = "";
    mIsOpen = false;
}

Process::~Process() {
    close();
}

void Process::close() {
    CloseHandle(mProcInfo.hProcess);
    CloseHandle(mProcInfo.hThread);
    CloseHandle(mChildOutWr);
    CloseHandle(mChildInRd);
    mIsOpen = false;
}

// TODO: cmd by reference
bool Process::start(std::string cmd) {
    if (cmd.length() == 0) return false;
    mCmd = cmd;
    SECURITY_ATTRIBUTES saAttr;
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = NULL;
    
    if (!CreatePipe(&mChildOutRd, &mChildOutWr, &saAttr, 0)) {
        std::cout << "CreatePipe for stdout failed" << std::endl;
        close();
        return false;
    }
    if (!SetHandleInformation(mChildOutRd, HANDLE_FLAG_INHERIT, 0)) {
        std::cout << "SetHandleInformation for stdout failed" << std::endl;
        close();
        return false;
    }
    if (!CreatePipe(&mChildInRd, &mChildInWr, &saAttr, 0)) {
        std::cout << "CreatePipe for stdin failed" << std::endl;
        close();
        return false;
    }
    if (!SetHandleInformation(mChildInWr, HANDLE_FLAG_INHERIT, 0)) {
        std::cout << "SetHandleInformation for stdin failed" << std::endl;
        close();
        return false;
    }
    
    ZeroMemory(&mProcInfo, sizeof(PROCESS_INFORMATION));
    ZeroMemory(&mStartupInfo, sizeof(STARTUPINFO));
    mStartupInfo.cb = sizeof(STARTUPINFO);
    mStartupInfo.hStdError = mChildOutWr;
    mStartupInfo.hStdOutput = mChildOutWr;
    mStartupInfo.hStdInput = mChildInRd;
    mStartupInfo.dwFlags |= STARTF_USESTDHANDLES;
    
    char* cmdline = new char[mCmd.length() + 1];
    mCmd.copy(cmdline, mCmd.length());
    cmdline[mCmd.length()] = '\0';
    std::cout << "cmdline: \"" << cmdline << "\"" << std::endl;
    BOOL success = CreateProcess(NULL,
        cmdline,
        NULL,
        NULL,
        TRUE,
        0,
        NULL,
        NULL,
        &mStartupInfo,
        &mProcInfo
    );
    delete[] cmdline;
    
    if (!success) {
        std::cout << "CreateProcess failed" << std::endl;
        close();
        return false;
    }
    
    mIsOpen = true;
    
    return true;
}

// TODO: Polish
bool Process::write(const std::string& s) {
    DWORD dwWritten = 0;
    BOOL bSuccess = FALSE;
    if (!WriteFile(mChildInWr, s.c_str(), s.length(), &dwWritten, NULL)) return false; 
    if (dwWritten == 0) return false;
    std::cout << "wrote: " << dwWritten << std::endl;
    return true;
}

// TODO: Polish
bool Process::read(std::string& buf) {
    DWORD dwRead;
    CHAR chBuf[4096];
    
    DWORD avail;
    
    buf.clear();
    while (PeekNamedPipe(mChildOutRd, NULL, 0, NULL, &avail, NULL) && avail > 0) {
        if (!ReadFile(mChildOutRd, chBuf, 4096, &dwRead, NULL)) {
            return false;
        }
        if (dwRead == 0) { 
            break;
        }
        std::string tmp{chBuf, dwRead};
        buf += tmp;
    }
    return true;
}

// TODO: GetExitCodeProcess is not sufficient for
// determining if the process is actually dead or not.
// (Process can return 259 as exit code).
bool Process::isAlive() const {
    if (mCmd.length() == 0) return false;
    DWORD status;
    if (!GetExitCodeProcess(mProcInfo.hProcess, &status)) return false;
    return status == STILL_ACTIVE;
}

bool Process::isOpen() const {
    return mIsOpen;
}