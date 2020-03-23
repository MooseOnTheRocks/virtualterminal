#ifndef SHELL_H
#define SHELL_H

#include "process.h"

class Shell {
public:
    Shell();
    ~Shell() = default;
    
    bool write(const std::string& s);
    bool read(std::string& buf);

private:
    std::string mCmd;
    std::string mCmdDif;
    Process mProc;
};

#endif // SHELL_H