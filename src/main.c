#include <gint/display.h>
#include <gint/keyboard.h>
#include <gint/rtc.h>
#include <time.h>
#include <stdlib.h>

#define SIZE 10
#define ROUND(x) ((x) / SIZE) * SIZE

#define C_APPLE C_RED
#define C_SNAKE C_GREEN
#define C_BG C_WHITE

typedef struct {
    int x;
    int y;
} coord_t;

coord_t apple = { 0, 0 };
coord_t snake = { 0, 0 };
coord_t direction = { 0, -10 };
int score = 0;

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
    int y = clamp(ROUND(rand() % (DHEIGHT - SIZE)), 20, DHEIGHT);
    coord_t pos = { x, y };

    return pos;
}

int update() {
    dsquare(snake, C_BG);
    snake.x += direction.x;
    snake.y += direction.y;

    snake.x = clamp(snake.x, 0, DWIDTH);
    snake.y = clamp(snake.y, 20, DHEIGHT);
    
    dsquare(snake, C_SNAKE);

    if (snake.x == apple.x && snake.y == apple.y) {
        score += 1;

        apple = rand_pos();
        dsquare(apple, C_APPLE);
    }
    
    drect(0, 0, DWIDTH, 20, C_WHITE);
    dprint(0, 0, C_BLACK, "Score: %i", score);
    dprint(0, 10, C_BLACK, "AX: %i AY: %i SX: %i SY: %i", apple.x, apple.y, snake.x, snake.y);

    dupdate();
    return TIMER_CONTINUE;
}

int main(void)
{
    srand(time(NULL));

    apple = rand_pos();
    snake = rand_pos();
    
    dclear(C_BG);
    dsquare(apple, C_APPLE);

    int (*functionPtr)();
	functionPtr = &update;
	gint_call_t callback = { .function = functionPtr};
	rtc_periodic_enable(RTC_4Hz, callback);

    while (true) {
        key_event_t key = getkey();
        if (key.type == KEYEV_DOWN || key.type == KEYEV_HOLD) {
            switch (key.key) {
                case KEY_UP:
                    direction.x = 0;
                    direction.y = -SIZE;
                    break;
                case KEY_DOWN:
                    direction.x = 0;
                    direction.y = SIZE;
                    break;
                case KEY_LEFT:
                    direction.x = -SIZE;
                    direction.y = 0;
                    break;
                case KEY_RIGHT:
                    direction.x = SIZE;
                    direction.y = 0;
                    break;
            }
            update();
        }
    }
    return 1;
}
