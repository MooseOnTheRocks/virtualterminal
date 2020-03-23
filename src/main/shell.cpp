#include "shell.h"

#include <iostream>

Shell::Shell() : mCmd{""}, mCmdDif{""} {
}

bool Shell::write(const std::string& s) {
    if (mProc.alive()) {
        return mProc.write(s);
    }
    else {
        // TODO: Don't do this, check if closed somehow before closing
        mProc.close();
        if (s[0] == '\n') {
            mProc.start(mCmd);
            mCmd = "";
            mCmdDif = "\n";
        }
        else {
            if (mCmdDif[0] == '\n') mCmdDif = "";
            mCmd += s;
            mCmdDif += s;
        }
        return true;
    }
}

bool Shell::read(std::string& buf) {
    bool success = mProc.read(buf);
    if (success && buf.length() > 0) {
        return true;
    }
    else {
        buf = "";
        if (mCmdDif.length() > 0) {
            buf.assign(mCmdDif);
            mCmdDif = "";
            return true;
        }
        else {
            return false;
        }
    }
}