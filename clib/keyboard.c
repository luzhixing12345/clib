
#include "keyboard.h"



char *keytostr(int key) {
    switch (key) {
        case KEY_ENTER:
            return "ENTER";
        case KEY_BACKSPACE:
            return "BACKSPACE";
        case KEY_ESC:
            return "ESC";
        case KEY_SPACE:
            return "SPACE";
        case KEY_TAB:
            return "TAB";
        default:
            return "UNKNOWN";
    }
}
