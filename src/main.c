#include <gint/display.h>
#include <gint/keyboard.h>
#include <time.h>
#include <stdlib.h>

#define SIZE 10
#define ROUND(x) ((x) / SIZE) * SIZE

#define C_APPLE C_GREEN
#define C_SNAKE C_RED
#define C_BG C_WHITE

typedef struct {
    int x;
    int y;
} coord_t;

coord_t apple = { 0, 0 };
coord_t snake = { 0, 0 };

void dsquare(coord_t pos, int c) {
    drect(pos.x, pos.y, pos.x + SIZE - 1, pos.y + SIZE - 1, c);
}

int clamp(int val, int min, int max) {
    if (val < min) {
        val = min;
    }
    else if (val > max) {
        val = ROUND(max);
    }

    return val;
}

coord_t rand_pos() {
    int x = ROUND(rand() % (DWIDTH - SIZE));
    int y = ROUND(rand() % (DHEIGHT - SIZE));
    coord_t pos = { x, y };

    return pos;
}

int main(void)
{
    srand(time(NULL));

    apple = rand_pos();
    snake = rand_pos();

    dclear(C_BG);
    dsquare(apple, C_APPLE);
    dsquare(snake, C_SNAKE);
    dprint(0, 0, C_BLACK, "AX: %i AY: %i SX: %i SY: %i", apple.x, apple.y, snake.x, snake.y);
    dupdate();


    while (true) {
        key_event_t key = getkey();
        if (key.type == KEYEV_DOWN || key.type == KEYEV_HOLD) {
            drect(0, 0, DWIDTH, 20, C_BG);
            dsquare(snake, C_BG);

            switch (key.key) {
                case KEY_UP:
                    snake.y -= SIZE;
                    break;
                case KEY_DOWN:
                    snake.y += SIZE;
                    break;
                case KEY_LEFT:
                    snake.x -= SIZE;
                    break;
                case KEY_RIGHT:
                    snake.x += SIZE;
                    break;
            }

            snake.x = clamp(snake.x, 0, DWIDTH);
            snake.y = clamp(snake.y, 0, DHEIGHT);

            if (snake.x == apple.x && snake.y == apple.y) {
                apple = rand_pos();
                dsquare(apple, C_APPLE);
            }

            dprint(0, 0, C_BLACK, "AX: %i AY: %i SX: %i SY: %i", apple.x, apple.y, snake.x, snake.y);
            dsquare(snake, C_SNAKE);
            dupdate();
        }
    }
    return 1;
}
