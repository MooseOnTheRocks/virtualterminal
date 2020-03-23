#include "shell.h"

#include <iostream>

Shell::Shell() : mCmd{""}, mCmdDif{""} {
}

Shell::~Shell() {
    mProc.close();
}

bool Shell::write(const std::string& s) {
    if (mProc.isAlive()) {
        return mProc.write(s);
    }
    else {
        for (int i = 0; i < s.length(); i++) {
            char c = s[i];
            switch (c) {
                // TODO: Don't just return,
                // process remaining input into buffer.
                case '\n': { 
                    if (!mProc.start("cmd /C " + mCmd)) {
                        mCmdDif = "Could not execute \"" + mCmd + "\"\n";
                        return true;
                    }
                    else {
                        mCmd = "";
                        mCmdDif = "\n";
                        return true;
                    }
                }
                case '\x08': {
                    if (mCmd.length() <= 0) {
                        break;
                    }
                    std::string sub = mCmd.substr(0, mCmd.length() - 1);
                    mCmd = sub;
                    mCmdDif += '\x08';
                    break;
                }
                default: {
                    mCmd += c;
                    mCmdDif += c;
                }
            }
        }
        return true;
    }
}

bool Shell::read(std::string& buf) {
    if (mProc.isOpen()) {
        bool success = mProc.read(buf);
        if (!success) {
            mProc.close();
        }
        else if (!mProc.isAlive()) {
            mProc.close();
        }
        
        buf = mCmdDif + buf;
        mCmdDif = "";
        return success;
    }
    else {
        buf = mCmdDif;
        mCmdDif = "";
        return true;
    }
}