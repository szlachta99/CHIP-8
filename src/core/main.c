#include "CHIP8.h"
#include <ncurses.h>
#include <string.h>

void draw(Screen s);
void update_kbd(CHIP8 *chip8);

int main(int argc, char *argv[]){
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <ROM file>\n", argv[0]);
        return 1;
    }

    initscr();
    noecho();
    cbreak();
    nodelay(stdscr, TRUE);

    CHIP8 chip8 = {0};
    InitializeCHIP8(&chip8);
    LoadROM(&chip8, argv[1]);
    while(1) {
        Instruction instruction = FetchInstruction(&chip8);
        ExecuteInstruction(&chip8, instruction);
        // Update timers and other logic here
        if(chip8.screen_changed) {
            chip8.screen_changed = 0;
            draw(chip8.screen);
        }
        if(chip8.delay_timer > 0) {
            chip8.delay_timer--;
        }
        if(chip8.sound_timer > 0) {
            beep();
            chip8.sound_timer--;
        }
        update_kbd(&chip8);
    }
    endwin();
    return 0;
}

void draw(Screen s)
{
    clear();
    for (int i = 0; i < 32; i++) {
        char buff[65];
        for (int j = 0; j < 64; j++) {
            buff[j] = (s[i] & (1ULL << (63 - j))) ? '#' : ' ';
        }
        buff[64] = '\0';
        mvaddnstr(i, 0, buff, 64);
    }
    refresh();
}

void update_kbd(CHIP8 *chip8)
{
    memcpy(chip8->prev_keypad, chip8->keypad, sizeof(chip8->keypad)); // save last frame

    memset(chip8->keypad, 0, sizeof(chip8->keypad));

    int ch = getch(); // Only read one key per frame
    if (ch != ERR) { // ERR is returned if no input is present in non-blocking mode
        switch (ch) {
            case '1': chip8->keypad[0x0] = 1; break;
            case '2': chip8->keypad[0x1] = 1; break;
            case '3': chip8->keypad[0x2] = 1; break;
            case '4': chip8->keypad[0x3] = 1; break;
            case 'q': chip8->keypad[0x4] = 1; break;
            case 'w': chip8->keypad[0x5] = 1; break;
            case 'e': chip8->keypad[0x6] = 1; break;
            case 'r': chip8->keypad[0x7] = 1; break;
            case 'a': chip8->keypad[0x8] = 1; break;
            case 's': chip8->keypad[0x9] = 1; break;
            case 'd': chip8->keypad[0xA] = 1; break;
            case 'f': chip8->keypad[0xB] = 1; break;
            case 'z': chip8->keypad[0xC] = 1; break;
            case 'x': chip8->keypad[0xD] = 1; break;
            case 'c': chip8->keypad[0xE] = 1; break;
            case 'v': chip8->keypad[0xF] = 1; break;
        }
    }
}

