#include "terminal.h"

#include <chrono>
#include <iostream>

#define SDL_MAIN_HANDLED
#include <SDL.h>

namespace Terminal {

Terminal::Terminal(const std::string& title, int width, int height) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cout << "SDL_Init failed" << std::endl;
        return;
    }
    
    mWindow = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if (mWindow == nullptr) {
        SDL_Quit();
        std::cout << "SDL_CreateWindow failed" << std::endl;
        return;
    }
    
    mRenderer = SDL_CreateRenderer(mWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (mRenderer == nullptr) {
        SDL_DestroyWindow(mWindow);
        SDL_Quit();
        std::cout << "SDL_CreateRenderer failed" << std::endl;
        return;
    }
    
    SDL_Surface* bmp = SDL_LoadBMP(".\\src\\resources\\charset.bmp");
    mCharset = nullptr;
    if (bmp != nullptr) {
        SDL_SetColorKey(bmp, SDL_TRUE, SDL_MapRGB(bmp->format, 0xFF, 0, 0xFF));
        mCharset = SDL_CreateTextureFromSurface(mRenderer, bmp);
    }
    else {
        std::cout << "Failed to load character set" << std::endl;
        return;
    }
    SDL_FreeSurface(bmp);
    
    // State
    mCurState = 0;
    mCurSeq = "";
    // Window
    mWidth = width;
    mHeight = height;
    // Font
    mCharWidth = 8;
    mCharHeight = 8;
    // Buffer
    mColumnCount = mWidth / mCharWidth;
    mRowCount = mHeight / mCharHeight;
    int bufsize = mColumnCount * mRowCount;
    mBuffer = new char[bufsize];
    for (int i = 0; i < bufsize; i++) {
        mBuffer[i] = '\0';
    }
    // Cursor
    mCursorPos = 0;
    mCursorHeight = mCharHeight / 8;
    // Render
    mDirty = true;
    // SDL
    mClipFrom = {0, 0, 8, 8};
    mDrawTo = {0, 0, mCharWidth, mCharHeight};
    mCursor = {0, mCharHeight - mCursorHeight, mCharWidth, mCursorHeight};
}

Terminal::~Terminal() {
    SDL_DestroyTexture(mCharset);
    SDL_DestroyRenderer(mRenderer);
    SDL_DestroyWindow(mWindow);
    SDL_Quit();
    delete[] mBuffer;
}

void Terminal::begin() {
    while (!mUpdate()) {
        mRender();
    }
}

#define CASE(id, body)  \
case id: {              \
    body                \
    else {              \
        write(c);       \
        mCurState = 0;  \
    }                   \
    break;              \
}

void Terminal::next(char c) {
    switch (mCurState) {
        CASE(0, if (c == '\x1B') { mCurSeq = ""; mCurState = 1; })
        CASE(1, if (c == '[') { mCurSeq += c; mCurState = 2; })
        CASE(2, if (c >= 0x30 && c <= 0x3F) { mCurSeq += c; mCurState = 3; }
                else if (c >= 0x20 && c <= 0x2F) { mCurSeq += c; mCurState = 4; }
                else if (c >= 0x40 && c <= 0x7E) { mCurSeq += c; mCurState = 0; mRunSeq(); })
        CASE(3, if (c >= 0x30 && c <= 0x3F) { mCurSeq += c; mCurState = 3; }
                else if (c >= 0x20 && c <= 0x2F) { mCurSeq += c; mCurState = 4; }
                else if (c >= 0x40 && c < 0x7E) { mCurSeq += c; mCurState = 0; mRunSeq(); })
        CASE(4, if (c >= 0x20 && c <= 0x2F) { mCurSeq+= c; mCurState = 4; }
                else if (c >= 0x40 && c <= 0x7E) { mCurSeq += c; mCurState = 0; mRunSeq(); })
        default:
            std::cout << "Invalid state: " << mCurState << "." << std::endl;
            mCurState = 0;
    }
}

void Terminal::write(char c) {
    if (mCursorPos >= (mColumnCount * mRowCount)) {
        return;
    }
    switch (c) {
        case '\x0C': {
            for (int i = 0; i < mColumnCount * mRowCount; i++) {
                mBuffer[i] = ' ';
            }
            mCursorPos = 0;
            break;
        }
        case '\n': {
            mBuffer[mCursorPos] = c;
            mCursorPos = (mCursorPos / mColumnCount) * mColumnCount;
            mCursorPos += mColumnCount;
            break;
        }
        case '\x08': {
            mBuffer[--mCursorPos] = ' ';
            break;
        }
        default: mBuffer[mCursorPos++] = c;
    }
}

void Terminal::mRunSeq() {
    std::cout << "Running sequence \"" << mCurSeq << "\"" << std::endl;
    // CSI command
    /*
    if (mCurSeq[0] == '[') {
        int p = 0;
        switch (mCurSeq[1]) {
            case '1': p = 1; break;
            case '2': p = 2; break;
            case '3': p = 3; break;
            case '4': p = 4; break;
            case '5': p = 5; break;
            case '6': p = 6; break;
            case '7': p = 7; break;
            case '8': p = 8; break;
            case '9': p = 9; break;
        }
        switch (mCurSeq[2]) {
            // Move cursor down
            case 'B': mCursorPos += p * mColumnCount; break;
            case 'A': mCursorPos -= p * mColumnCount; break;
        }
    }
    */
    mCurSeq = "";
}

bool Terminal::mUpdate() {
    SDL_Event event;
    while (SDL_PollEvent(&event) != 0) {
        switch (event.type) {
            // -- Window Event
            case SDL_WINDOWEVENT: {
                if (event.window.windowID != SDL_GetWindowID(mWindow)) break;
                // TODO: Only mark mDirty = true for specific window events.
                mDirty = true;
                switch (event.window.event) {
                    // -- Window size change
                    case SDL_WINDOWEVENT_SIZE_CHANGED: {
                        mWidth = event.window.data1;
                        mHeight = event.window.data2;
                        mColumnCount = mWidth / mCharWidth;
                        mRowCount = mHeight / mCharHeight;
                        break;
                    }
                }
                break;
            }
            // TODO: My god clean this up
            case SDL_KEYDOWN: {
                int scancode = event.key.keysym.scancode;
                switch (scancode) {
                    case SDL_SCANCODE_BACKSPACE: { mShell.write(std::string{'\x08'}); break; }
                    case SDL_SCANCODE_PERIOD: { mShell.write(std::string{'.'}); break; }
                    case SDL_SCANCODE_MINUS: { mShell.write(std::string{'-'}); break; }
                    case SDL_SCANCODE_EQUALS: { mShell.write(std::string{'='}); break; }
                    case SDL_SCANCODE_SEMICOLON: {mShell.write(std::string{';'}); break; }
                    case SDL_SCANCODE_GRAVE: { mShell.write(std::string{'`'}); break; }
                    case SDL_SCANCODE_LEFTBRACKET: { mShell.write(std::string{'['}); break; }
                    case SDL_SCANCODE_RIGHTBRACKET: { mShell.write(std::string{']'}); break; }
                    case SDL_SCANCODE_APOSTROPHE: { mShell.write(std::string{'\''}); break; }
                    case SDL_SCANCODE_BACKSLASH: { mShell.write(std::string{'\\'}); break; }
                    case SDL_SCANCODE_SLASH: { mShell.write(std::string{'/'}); break; }
                    case SDL_SCANCODE_COMMA: { mShell.write(std::string{','}); break; }
                    case SDL_SCANCODE_SPACE: { mShell.write(std::string{' '}); break; }
                    case SDL_SCANCODE_RETURN: { mShell.write(std::string{'\n'}); break; }
                    case SDL_SCANCODE_0: { mShell.write(std::string{'0'}); break; }
                    case SDL_SCANCODE_1: { mShell.write(std::string{'1'}); break; }
                    case SDL_SCANCODE_2: { mShell.write(std::string{'2'}); break; }
                    case SDL_SCANCODE_3: { mShell.write(std::string{'3'}); break; }
                    case SDL_SCANCODE_4: { mShell.write(std::string{'4'}); break; }
                    case SDL_SCANCODE_5: { mShell.write(std::string{'5'}); break; }
                    case SDL_SCANCODE_6: { mShell.write(std::string{'6'}); break; }
                    case SDL_SCANCODE_7: { mShell.write(std::string{'7'}); break; }
                    case SDL_SCANCODE_8: { mShell.write(std::string{'8'}); break; }
                    case SDL_SCANCODE_9: { mShell.write(std::string{'9'}); break; }
                    default: {
                        if (scancode >= SDL_SCANCODE_A && scancode <= SDL_SCANCODE_Z) {
                            mShell.write(std::string{(char) (scancode - SDL_SCANCODE_A + 'a')});
                        }
                    }
                }
                break;
            }
            case SDL_QUIT: {
                return true;
            }
        }
    }
    
    std::string s;
    if (mShell.read(s)) {
        for (int i = 0; i < s.length(); i++) {
            std::cout << "read: " << ((int) s[i]) << std::endl;
            next(s[i]);
        }
    }
    
    return false;
}

void Terminal::mRender() {
    if (mDirty) {
        // Clear the screen
        SDL_SetRenderDrawColor(mRenderer, 0x00, 0x00, 0x00, 0xFF);
        SDL_RenderClear(mRenderer);
        // Draw the characters
        for (int i = 0; i < mColumnCount; i++) {
            for (int j = 0; j < mRowCount; j++) {
                // Use this to implement colored text
                // SDL_SetTextureColorMod(mCharset, 0xFF, 0xFF, 0xFF);
                int index = i + j * mColumnCount;
                if (index < 0 || index >= mColumnCount * mRowCount) continue;
                char c = mBuffer[index];
                mClipFrom.x = c % 16 * 8;
                mClipFrom.y = c / 16 * 8;
                mDrawTo.x = i * mCharWidth;
                mDrawTo.y = j * mCharHeight;
                SDL_RenderCopy(mRenderer, mCharset, &mClipFrom, &mDrawTo);
            }
        }
        // Draw the cursor
        SDL_SetRenderDrawColor(mRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
        int cx = mCursorPos % mColumnCount;
        int cy = mCursorPos / mColumnCount;
        mCursor.x = cx * mCharWidth;
        mCursor.y = cy * mCharHeight + mCharHeight - mCursorHeight;
        SDL_RenderFillRect(mRenderer, &mCursor);
        // Draw to the screen
        SDL_RenderPresent(mRenderer);
    }
}

// void Terminal::setCursorXY(int x, int y) {
    // int cx = x < 1 ? 1 : (x - 1) % mColumnCount;
    // int cy = y < 1 ? 1 : (y - 1) % mRowCount; 
    // mCursorPos = cx + cy * mColumnCount;
// }

// int Terminal::getCursorX() const {
    // return mCursorPos % mColumnCount + 1;
// }

// int Terminal::getCursorY() const {
    // return mCursorPos / mColumnCount + 1;
// }

} // namespace Terminal
