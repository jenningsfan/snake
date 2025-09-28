#include <gint/display.h>
#include <gint/keyboard.h>
#include <gint/rtc.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 10
#define ROUND(x) ((x) / SIZE) * SIZE

#define WIDTH_SQUARES DWIDTH / SIZE
#define HEIGHT_SQUARES DHEIGHT / SIZE

#define C_APPLE C_RED
#define C_SNAKE C_GREEN
#define C_BG C_WHITE

typedef struct {
    int x;
    int y;
} coord_t;

typedef struct snake_t {
    coord_t pos;
    coord_t old_pos;
    struct snake_t *prev;
    struct snake_t *next;
} snake_t;

const coord_t UP = { 0, -SIZE };
const coord_t DOWN = { 0, SIZE };
const coord_t LEFT = { -SIZE, 0 };
const coord_t RIGHT = { SIZE, 0 };

snake_t snake = { { 0, 0 }, DOWN, NULL, NULL };
coord_t direction = DOWN;
coord_t apple = { 0, 0 };
int score = 0;

bool coord_eq(coord_t a, coord_t b) {
    return (a.x == b.x && a.y == b.y);
}

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

void dsnake(snake_t *snake, int c) {    
    while (snake != NULL) {
        dsquare(snake->pos, c);
        snake = snake->next;
    }
}

void update_snake(snake_t* snake) {
    while (snake != NULL) {
        snake->old_pos = snake->pos;
        snake->pos.x = snake->prev->old_pos.x;
        snake->pos.y = snake->prev->old_pos.y;
        snake = snake->next;
    }
}

snake_t* last_snake(snake_t* snake) {
    return snake->next == NULL ? snake : last_snake(snake->next);
}

bool internal_collision(snake_t* snake) {
    //return false;
    for (snake_t* i = snake; i != NULL; i = i->next) {
        for (snake_t* j = snake; j != NULL; j = j->next) {
            if (coord_eq(i->pos, j->pos) && i != j) {
                return true;
            }
        }
    }
    
    return false;
}

int update() {
    dsnake(&snake, C_BG);

    snake.old_pos = snake.pos;
    snake.pos.x += direction.x;
    snake.pos.y += direction.y;
    snake.pos.x = clamp(snake.pos.x, 0, DWIDTH);
    snake.pos.y = clamp(snake.pos.y, 20, DHEIGHT);

    update_snake(snake.next);
    dsnake(&snake, C_SNAKE);

    if (internal_collision(&snake)) {
        dclear(C_RED);
        dprint(0, DHEIGHT / 2.0, C_WHITE, "Game Over   Score: %i", score);
        dupdate();
        return TIMER_STOP;
    }

    if (snake.pos.x == apple.x && snake.pos.y == apple.y) {
        score += 1;

        apple = rand_pos();
        dsquare(apple, C_APPLE);
        
        snake_t* new_snake = malloc(sizeof(snake_t));
        snake_t* back_snake = last_snake(&snake);
        new_snake->old_pos = back_snake->old_pos;
        new_snake->next = NULL;
        new_snake->prev = back_snake;
        new_snake->pos = back_snake->pos;
        new_snake->pos.x -= back_snake->old_pos.x;
        new_snake->pos.y -= back_snake->old_pos.y;
        back_snake->next = new_snake;

        drect(0, 200, DWIDTH, 225, C_WHITE);
        dprint(0, 200, C_BLACK, "ND: (%i, %i) NP: (%i, %i)", new_snake->old_pos.x, new_snake->old_pos.y, new_snake->pos.x, new_snake->pos.y);
    }
    
    drect(0, 0, DWIDTH, 20, C_WHITE);
    dprint(0, 0, C_BLACK, "Score: %i", score);
    dprint(0, 10, C_BLACK, "AX: %i AY: %i SX: %i SY: %i", apple.x, apple.y, snake.pos.x, snake.pos.y);

    dupdate();
    return TIMER_CONTINUE;
}

int main(void)
{
    srand(time(NULL));

    apple = rand_pos();
    snake.pos = rand_pos();
    
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
                    direction = UP;
                    break;
                case KEY_DOWN:
                    direction = DOWN;
                    break;
                case KEY_LEFT:
                    direction = LEFT;
                    break;
                case KEY_RIGHT:
                    direction = RIGHT;
                    break;
            }
            if (update() == TIMER_STOP) {
                break;
            }
        }
    }

    getkey();

    return 1;
}
