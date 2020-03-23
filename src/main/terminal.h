#ifndef TERMINAL_H
#define TERMINAL_H

#include <chrono>
#include <string>

#define SDL_MAIN_HANDLED
#include <SDL.h>
#undef SDL_MAIN_HANDLED

#include "shell.h"

namespace Terminal {

class Terminal {
public:
    Terminal(const std::string& title, int width, int height);
    ~Terminal();
    
    void begin();
    void next(char c);
    void write(char c);
    
    // void setCursorXY(int x, int y);
    
    // int getCursorX() const;
    // int getCursorY() const;
    
private:
    // State
    Shell mShell;
    int mCurState;
    std::string mCurSeq;
    void mRunSeq();
    // Window
    int mWidth, mHeight;
    // Font
    int mCharWidth, mCharHeight;
    // Buffer
    int mColumnCount, mRowCount;
    char* mBuffer;
    // Cursor
    int mCursorPos;
    int mCursorHeight;
    // Update
    bool mUpdate();
    // Render
    bool mDirty;
    void mRender();
    
    // SDL
    SDL_Rect mClipFrom;
    SDL_Rect mDrawTo;
    SDL_Rect mCursor;
    SDL_Window* mWindow;
    SDL_Renderer* mRenderer;    SDL_Texture* mCharset;
};

} // namespace Terminal

#endif // TERMINAL_H