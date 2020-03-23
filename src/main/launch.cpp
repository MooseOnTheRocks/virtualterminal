#include "terminal.h"

int main(int argc, char** argv) {
    Terminal::Terminal term{"Hello, Terminal!", 640, 480};
    term.begin();
}