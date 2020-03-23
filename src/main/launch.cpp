#include <iostream>

#include "terminal.h"

int main(int argc, char** argv) {
    Terminal::Terminal term{"Hello, Terminal!", 640, 480};
    // term.setCursorXY(10, 20);
    // std::cout << "Cursor: " << term.getCursorX() << ", " << term.getCursorY() << std::endl;
    term.begin();
}