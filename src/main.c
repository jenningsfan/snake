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

#define C_PURPLE C_RGB(31, 0, 31)
#define C_ORANGE C_RGB(255 / 31, 127 / 31, 0)
#define C_YELLOW C_RGB(31, 31, 0)
#define C_LIGHT_BLUE C_RGB(173 / 31, 216 / 31, 230 / 31)

int C_APPLE = C_RED;
int C_SNAKE = C_GREEN;
int C_BG = C_WHITE;

int colours[] = { C_RED, C_ORANGE, C_YELLOW, C_GREEN, C_BLUE, C_LIGHT_BLUE, C_WHITE, C_BLACK };
char* colour_names[] = { "Red", "Orange", "Yellow", "Green", "Blue", "Light Blue", "White", "Black" };

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

typedef enum {
    STATE_PLAY,
    STATE_SETUP,
    STATE_DEAD,
} state_t;

typedef enum {
    OPT_SPEED,
    OPT_APPLE,
    OPT_SNAKE,
    OPT_BG,
} options_t;

options_t curr_option = OPT_SPEED;
char *speeds[] = { "Dead", "Light Speed", "Very fast", "Fast", "Normal", "Slow", "Very slow", "Very very slow" };

const coord_t UP = { 0, -SIZE };
const coord_t DOWN = { 0, SIZE };
const coord_t LEFT = { -SIZE, 0 };
const coord_t RIGHT = { SIZE, 0 };

snake_t snake;
coord_t direction;
coord_t apple;
int score;
state_t state;
int speed = RTC_4Hz;

extern bopti_image_t img_eyes;
extern bopti_image_t party_hat_up;
extern bopti_image_t party_hat_down;
extern bopti_image_t party_hat_left;
extern bopti_image_t party_hat_right;

bool coord_eq(coord_t a, coord_t b) {
    return (a.x == b.x && a.y == b.y);
}

void dsquare(coord_t pos, int c) {
    drect(pos.x, pos.y, pos.x + SIZE - 1, pos.y + SIZE - 1, c);
}

void dclear_image(coord_t pos, bopti_image_t* image, int c) {
    drect(pos.x, pos.y, pos.x + image->width, pos.y + image->height, c);
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
    snake_t* start_snake = snake;

    snake = snake->next;
    while (snake != NULL) {
        dsquare(snake->pos, c);
        snake = snake->next;
    }
    
    if (c == C_SNAKE) {
        if (coord_eq(direction, UP)) {
            dimage(start_snake->pos.x, start_snake->pos.y, &party_hat_up);
        }
        else if (coord_eq(direction, DOWN)) {
            dimage(start_snake->pos.x, start_snake->pos.y, &party_hat_down);
        }
        else if (coord_eq(direction, LEFT)) {
            dimage(start_snake->pos.x, start_snake->pos.y, &party_hat_left);
        }
        else if (coord_eq(direction, RIGHT)) {
            dimage(start_snake->pos.x, start_snake->pos.y, &party_hat_right);
        }
        else {
            dprint(100, 100, C_SNAKE, "DIRECTION IS (%i, %i) WHICH IS INVALID", direction.x, direction.y);
            state = STATE_DEAD;
            return;
        }

        dimage(start_snake->next->pos.x, start_snake->next->pos.y, &img_eyes);
    }
    else {
        coord_t clear = { start_snake->pos.x, start_snake->pos.y };
        dclear_image(clear, &party_hat_up, c);
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
    for (snake_t* i = snake; i != NULL; i = i->next) {
        for (snake_t* j = snake; j != NULL; j = j->next) {
            if (coord_eq(i->pos, j->pos) && i != j) {
                return true;
            }
        }
    }
    
    return false;
}

bool apple_eaten(snake_t* snake, coord_t apple) {
    for (snake_t* i = snake; i != NULL; i = i->next) {
        if (coord_eq(i->pos, apple)) {
            return true;
        }
    }

    return false;
}

int update_play() {
    if (state != STATE_PLAY) {
        return state == STATE_DEAD ? TIMER_STOP : TIMER_CONTINUE;
    }

    dsnake(&snake, C_BG);
    
    snake.old_pos = snake.pos;
    snake.pos.x += direction.x;
    snake.pos.y += direction.y;
    snake.pos.x = clamp(snake.pos.x, 0, DWIDTH);
    snake.pos.y = clamp(snake.pos.y, 0, DHEIGHT);
    
    update_snake(snake.next);
    
    if (internal_collision(&snake)) {
        dclear(C_RED);
        dprint(0, DHEIGHT / 2.0, C_WHITE, "Game Over   Score: %i", score);
        dupdate();
        state = STATE_DEAD;
        return TIMER_STOP;
    }
    
    if (apple_eaten(&snake, apple)) {
        score += 1;
        
        dsquare(apple, C_BG);
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
    }
    
    
    drect(0, 0, DWIDTH, 10, C_BG);
    dprint(0, 0, C_BLACK, "Score: %i", score);
    
    dsnake(&snake, C_SNAKE);
    dsquare(apple, C_APPLE);
    
    dupdate();
    return TIMER_CONTINUE;
}

size_t pos_in_array(int array[], int item, size_t len) {
    for (size_t i = 0; i < len; i++) {
        if (array[i] == item) {
            return i;
        }
    }

    return -1;
}

int update_wrap(int val, int direction, int min, int max) {
    val += direction;
    if (val > max) {
        val = min;
    }
    else if (val < min) {
        val = max;
    }

    return val;
}

void update_speed(int direction) {
    speed = update_wrap(speed, direction, 0, 7);

    rtc_periodic_disable();
    rtc_periodic_enable(speed, (gint_call_t) { .function = &update_play });
}

void update_setup(key_event_t key) {
    size_t apple_pos = pos_in_array(colours, C_APPLE, 8);
    size_t snake_pos = pos_in_array(colours, C_SNAKE, 8);
    size_t bg_pos = pos_in_array(colours, C_BG, 8);

    switch (key.key) {
        case KEY_OPTN:
            state = STATE_PLAY;
            dclear(C_BG);
            return;
        case KEY_UP:
        case KEY_DOWN:
            int update_dir_opt = key.key == KEY_DOWN ? 1 : -1;
            curr_option = update_wrap(curr_option, update_dir_opt, 0, 3);
            break;
        case KEY_LEFT:
        case KEY_RIGHT:
            int update_dir = key.key == KEY_LEFT ? 1 : -1;
            switch (curr_option) {
                case OPT_SPEED:
                    update_speed(update_dir);
                    break;
                case OPT_APPLE:
                    apple_pos = update_wrap(apple_pos, update_dir, 0, 7);
                    C_APPLE = colours[apple_pos];
                    break;
                case OPT_SNAKE:
                    snake_pos = update_wrap(snake_pos, update_dir, 0, 7);
                    C_SNAKE = colours[snake_pos];
                    break;
                case OPT_BG:
                    bg_pos = update_wrap(bg_pos, update_dir, 0, 7);
                    C_BG = colours[bg_pos];
                    break;
            }
        break;
    }

    dclear(C_BG);
    dprint(15, 0, C_BLACK, "Speed: %s", speeds[speed]);
    dprint(15, 22, C_APPLE, "Apple: %s", colour_names[apple_pos]);
    dprint(15, 44, C_SNAKE, "Snake: %s", colour_names[snake_pos]);
    dprint(15, 66, C_BLACK, "Bg: %s", colour_names[bg_pos]);

    drect(0, curr_option * 22, 10, (curr_option + 1) * 22, C_BLUE);
    dupdate();
}

void init() {
    for (snake_t* i = snake.next; i != NULL; i = i->next) {
        free(i);
    }

    snake = (snake_t){ rand_pos(), { 0, 0 }, NULL, NULL };
    snake.next = (snake_t*) calloc(1, sizeof(snake_t));
    snake.next->prev = &snake;

    direction = DOWN;
    apple = rand_pos();
    score = 0;
    state = STATE_PLAY;

    dclear(C_BG);
    dsquare(apple, C_APPLE);

    rtc_periodic_disable();
    rtc_periodic_enable(speed, (gint_call_t) { .function = &update_play });
}

int main(void)
{
    srand(time(NULL));

    while (true) {
        init();
    
        while (true) {
            key_event_t key = getkey();
            if (key.type == KEYEV_DOWN || key.type == KEYEV_HOLD) {
                switch (state) {
                    case STATE_PLAY:
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
                            case KEY_OPTN:
                                state = STATE_SETUP;
                                key.key = -1; // hack to stop it from exiting immediatley
                                update_setup(key);
                                break;
                            case KEY_LEFTPAR:
                                speed = RTC_NONE;
                                rtc_periodic_disable();
                                rtc_periodic_enable(speed, (gint_call_t) { .function = &update_play });
                                break;
                            case KEY_RIGHTPAR:
                                speed = RTC_256Hz;
                                rtc_periodic_disable();
                                rtc_periodic_enable(speed, (gint_call_t) { .function = &update_play });
                                break;
                            case KEY_LOG:
                                speed = RTC_4Hz;
                                rtc_periodic_disable();
                                rtc_periodic_enable(speed, (gint_call_t) { .function = &update_play });
                                break;
                        }
                        break;
                    case STATE_SETUP:
                        update_setup(key);
                        break;
                }

                if (update_play() == TIMER_STOP) {
                    break;
                }
            }
        }
    
        getkey();
    }

    return 1;
}
