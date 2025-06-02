
#pragma once

// #define KEY_ENTER        '\n'  // 0x0A
// #define KEY_CR           '\r'
// #define KEY_BACKSPACE    127
// #define KEY_ESC          '\033'
// #define KEY_SPACE        ' '
// #define KEY_TAB          '\t'
// #define KEY_UP           0x41
// #define KEY_DOWN         0x42
// #define KEY_RIGHT        0x43
// #define KEY_LEFT         0x44
// #define KEY_HOME         'H'
// #define KEY_END          'F'
// #define KEY_CLEAR_SCREEN 0x0C
// #define KEY_CLEAR_AFTER  0x0B

typedef enum {
    KEY_ENTER = '\n',
    KEY_CR = '\r',
    KEY_BACKSPACE = 127,
    KEY_ESC = '\033',
    KEY_SPACE = ' ',
    KEY_TAB = '\t',
    KEY_UP = 0x41,
    KEY_DOWN = 0x42,
    KEY_RIGHT = 0x43,
    KEY_LEFT = 0x44,
    KEY_HOME = 'H',
    KEY_END = 'F',
    KEY_CLEAR_SCREEN = 0x0C,
    KEY_CLEAR_AFTER = 0x0B
} Key_t;

typedef enum {
    K_CTL_SERIAL,
    K_UP,
    K_DOWN,
    K_LEFT,
    K_RIGHT,
    K_TAB,
    K_ENTER,
    K_HOME,                // home
    K_END,                 // end
    K_ERASE,               // backspace
    K_EOF,                 // ctrl + d
    K_KILL,                // ctrl + u
    K_CLEAR_SCREEN,        // ctrl + l
    K_CLEAR_AFTER_CURSOR,  // ctrl + k
    N_KEYBIND
} KeyBind;

struct keybind_t {
    Key_t key;
    Key_t (*func)(struct keybind_t *keybind, void *data);
};

char *keytostr(int key);